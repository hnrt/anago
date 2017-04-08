// Copyright (C) 2012-2017 Hideaki Narita


#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "Net/Console.h"
#include "Thread/ThreadManager.h"
#include "ConsoleViewImpl.h"
#include "FrameBuffer.h"


using namespace hnrt;


ConsoleViewImpl::ConsoleViewImpl()
    : _console(Console::create(*this))
    , _consoleThread(NULL)
    , _frameBuffer(FrameBuffer::create(0, 0))
    , _bpp(0)
    , _hasFocus(false)
    , _scaleEnabled(false)
    , _scalingMultiplier(0)
    , _scalingDivisor(0)
    , _needInitScaling(true)
    , _containerWidth(65535)
    , _containerHeight(65535)
    , _terminate(false)
    , _pScale(&ConsoleViewImpl::scaleByThreads)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::ctor", this));

    set_double_buffered(false);
    set_flags(Gtk::CAN_FOCUS);
    set_events(Gdk::STRUCTURE_MASK |
               Gdk::EXPOSURE_MASK |
               Gdk::ENTER_NOTIFY_MASK |
               Gdk::LEAVE_NOTIFY_MASK |
               Gdk::POINTER_MOTION_MASK |
               Gdk::BUTTON_PRESS_MASK |
               Gdk::BUTTON_RELEASE_MASK |
               Gdk::SCROLL_MASK |
               Gdk::FOCUS_CHANGE_MASK |
               Gdk::KEY_PRESS_MASK |
               Gdk::KEY_RELEASE_MASK);

    for (unsigned int i = 0; i < sizeof(_scaleThreads) / sizeof(_scaleThreads[0]); i++)
    {
        _scaleThreads[i] = ThreadManager::instance().create(sigc::mem_fun(*this, &ConsoleViewImpl::scaleWorker), true, "CV-Scaler");
    }

    memset(keyvals, 0, sizeof(keyvals));

    _connection = _dispatcher.connect(sigc::mem_fun(*this, &ConsoleViewImpl::onDispatched));
}


ConsoleViewImpl::~ConsoleViewImpl()
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::dtor", this));

    close();

    _mutexStart.lock();
    _terminate = true;
    _condStart.broadcast();
    _mutexStart.unlock();
    for (unsigned int i = 0; i < sizeof(_scaleThreads) / sizeof(_scaleThreads[0]); i++)
    {
        if (_scaleThreads[i])
        {
            _scaleThreads[i]->join();
        }
    }

    _connection.disconnect();
}


void ConsoleViewImpl::open(const char* location, const char* authorization)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::open", this));
    close();
    _consoleThread = ThreadManager::instance().create(sigc::bind<Glib::ustring, Glib::ustring>(sigc::mem_fun(*this, &ConsoleViewImpl::run), Glib::ustring(location), Glib::ustring(authorization)), true, "Console");
    _consoleThread->set_priority(Glib::THREAD_PRIORITY_HIGH);
}


void ConsoleViewImpl::close()
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::close", this));
    int width = 0;
    int height = 0;
    Glib::Thread* thread = InterlockedExchangePointer(&_consoleThread, (Glib::Thread*)NULL);
    if (thread)
    {
        _console->terminate();
        thread->join();
        Glib::Mutex::Lock lock(_mutexFb);
        width = _frameBuffer->getWidth();
        height = _frameBuffer->getHeight();
        if (width > 0 && height > 0)
        {
            _frameBuffer->changeColor(0.5);
        }
    }
    if (width > 0 && height > 0)
    {
        dispatchUpdateDesktop(0, 0, width, height);
    }
}


void ConsoleViewImpl::run(Glib::ustring location, Glib::ustring authorization)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::run", this));
    try
    {
        _console->open(location.c_str(), authorization.c_str());
        _console->run();
        _console->close();
    }
    catch (...)
    {
        Logger::instance().warn("ConsoleViewImpl@%zx::run: Unhandled exception caught.", this);
    }
}


int ConsoleViewImpl::getFrameWidth()
{
    if (_frameBuffer)
    {
        return _frameBuffer->getWidth();
    }
    else
    {
        return 0;
    }
}


int ConsoleViewImpl::getFrameHeight()
{
    if (_frameBuffer)
    {
        return _frameBuffer->getHeight();
    }
    else
    {
        return 0;
    }
}


bool ConsoleViewImpl::on_configure_event(GdkEventConfigure* event)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::on_configure_event", this));
    if (_scaleEnabled)
    {
        _needInitScaling = true;
    }
    return false;
}


bool ConsoleViewImpl::on_expose_event(GdkEventExpose* event)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::on_expose_event", this));
    Glib::RefPtr<Gdk::Window> window = get_window();
    RefPtr<FrameBuffer> fb;
    {
        Glib::Mutex::Lock lock(_mutexFb);
        if (_needInitScaling)
        {
            _needInitScaling = false;
            initScaling(window);
        }
        fb = _frameBufferScaled ? _frameBufferScaled : _frameBuffer;
    }
    int width = fb->getWidth();
    int height = fb->getHeight();
    int left = event->area.x;
    int top = event->area.y;
    int right = left + event->area.width;
    int bottom = top + event->area.height;
    int x = left < 0 ? 0 : left < width ? left : width;
    int y = top < 0 ? 0 : top < height ? top : height;
    int cx = (right < width ? right : width) - x;
    int cy = (bottom < height ? bottom : height) - y;
    if (cx && cy)
    {
        Glib::RefPtr<Gtk::Style> style = get_style();
        Gtk::StateType state = get_state();
        Glib::RefPtr<Gdk::GC> gc = style->get_fg_gc(state);
        fb->draw(reinterpret_cast<Glib::RefPtr<Gdk::Drawable>&>(window), gc, x, y, cx, cy);
    }
    x = left < width ? width : left;
    y = top;
    cx = x < right ? right - x : 0;
    cy = y < height ? height - y : 0;
    if (cx && cy)
    {
        window->clear_area(x, y, cx, cy);
    }
    x = left;
    y = top < height ? height : top;
    cx = right - left;
    cy = y < bottom ? bottom - y : 0;
    if (cx && cy)
    { 
        window->clear_area(x, y, cx, cy);
    }
    return true;
}


bool ConsoleViewImpl::on_enter_notify_event(GdkEventCrossing* event)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::on_enter_notify_event", this));
    if (!is_focus())
    {
        grab_focus();
    }
    Glib::RefPtr<Gdk::Window> window = get_window();
    Gdk::Cursor blank(Gdk::BLANK_CURSOR);
    window->set_cursor(blank);
    if (_keyboardInputFilter)
    {
        _keyboardInputFilter->reset();
    }
    return true;
}


bool ConsoleViewImpl::on_leave_notify_event(GdkEventCrossing* event)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::on_leave_notify_event", this));
    Glib::RefPtr<Gdk::Window> window = get_window();
    window->set_cursor();
    return true;
}


bool ConsoleViewImpl::on_motion_notify_event(GdkEventMotion* event)
{
    if (_hasFocus)
    {
        guchar b = 0;
        guint modState = event->state;
        if ((modState & GDK_BUTTON1_MASK)) b |= 1 << 0;
        if ((modState & GDK_BUTTON2_MASK)) b |= 1 << 1;
        if ((modState & GDK_BUTTON3_MASK)) b |= 1 << 2;
        if ((modState & GDK_BUTTON4_MASK)) b |= 1 << 3;
        if ((modState & GDK_BUTTON5_MASK)) b |= 1 << 4;
        sendPointerEvent(b, event->x, event->y);
    }
    return true;
}


bool ConsoleViewImpl::on_button_press_event(GdkEventButton* event)
{
    if (_hasFocus)
    {
        guchar b = 0;
        guint modState = event->state;
        if ((modState & GDK_BUTTON1_MASK)) b |= 1 << 0;
        if ((modState & GDK_BUTTON2_MASK)) b |= 1 << 1;
        if ((modState & GDK_BUTTON3_MASK)) b |= 1 << 2;
        if ((modState & GDK_BUTTON4_MASK)) b |= 1 << 3;
        if ((modState & GDK_BUTTON5_MASK)) b |= 1 << 4;
        b |= 1 << (event->button - 1);
        sendPointerEvent(b, event->x, event->y);
    }
    return true;
}


bool ConsoleViewImpl::on_button_release_event(GdkEventButton* event)
{
    if (_hasFocus)
    {
        guchar b = 0;
        guint modState = event->state;
        if ((modState & GDK_BUTTON1_MASK)) b |= 1 << 0;
        if ((modState & GDK_BUTTON2_MASK)) b |= 1 << 1;
        if ((modState & GDK_BUTTON3_MASK)) b |= 1 << 2;
        if ((modState & GDK_BUTTON4_MASK)) b |= 1 << 3;
        if ((modState & GDK_BUTTON5_MASK)) b |= 1 << 4;
        b &= ~(1 << (event->button - 1));
        sendPointerEvent(b, event->x, event->y);
    }
    return true;
}


bool ConsoleViewImpl::on_scroll_event(GdkEventScroll* event)
{
    if (_hasFocus)
    {
        int shift;
        if (event->direction == GDK_SCROLL_UP ||
            event->direction == GDK_SCROLL_LEFT)
        {
            shift = 3;
        }
        else if (event->direction == GDK_SCROLL_DOWN ||
                 event->direction == GDK_SCROLL_RIGHT)
        {
            shift = 4;
        }
        else
        {
            return true;
        }
        guchar b = 0;
        guint modState = event->state;
        if ((modState & GDK_BUTTON1_MASK)) b |= 1 << 0;
        if ((modState & GDK_BUTTON2_MASK)) b |= 1 << 1;
        if ((modState & GDK_BUTTON3_MASK)) b |= 1 << 2;
        if ((modState & GDK_BUTTON4_MASK)) b |= 1 << 3;
        if ((modState & GDK_BUTTON5_MASK)) b |= 1 << 4;
        b |= 1 << shift;
        sendPointerEvent(b, event->x, event->y);
        b &= ~(1 << shift);
        sendPointerEvent(b, event->x, event->y);
    }
    return true;
}


bool ConsoleViewImpl::on_focus_in_event(GdkEventFocus* event)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::on_focus_in_event", this));
    _hasFocus = true;
    return true;
}


bool ConsoleViewImpl::on_focus_out_event(GdkEventFocus* event)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::on_focus_out_event", this));
    _hasFocus = false;
    for (unsigned int keycode = 0; keycode < 256; keycode++)
    {
        if (keyvals[keycode])
        {
            TRACEPUT("sendKeyEvent(0,%u,%u)", keyvals[keycode], keycode);
            _console->sendKeyEvent(0, keyvals[keycode], keycode);
        }
    }
    return true;
}


bool ConsoleViewImpl::on_key_press_event(GdkEventKey* event)
{
    sendKeyEvent(1, event->keyval, event->state, event->hardware_keycode);
    return true;
}


bool ConsoleViewImpl::on_key_release_event(GdkEventKey* event)
{
    sendKeyEvent(0, event->keyval, event->state, event->hardware_keycode);
    return true;
}


inline void ConsoleViewImpl::dispatchResizeDesktop(int width, int height)
{
    {
        Glib::Mutex::Lock lock(_mutexMsg);
        if (_msgCount < MSG_MAX)
        {
            int index = (_msgIndex + _msgCount++) % MSG_MAX;
            Message& message = _msg[index];
            message.type = Message::RESIZE_DESKTOP;
            message.rect.x = 0;
            message.rect.y = 0;
            message.rect.width = width;
            message.rect.height = height;
        }
        else
        {
            Logger::instance().trace("ConsoleViewImpl: Message queue is FULL!");
            _msgIndex = (_msgIndex + 1) % MSG_MAX;
            Message& message = _msg[_msgIndex];
            message.type = Message::RESIZE_DESKTOP;
            message.rect.x = 0;
            message.rect.y = 0;
            message.rect.width = width;
            message.rect.height = height;
        }
    }
    incRef();
    _dispatcher();
}


inline void ConsoleViewImpl::dispatchUpdateDesktop(int x, int y, int width, int height)
{
    {
        Glib::Mutex::Lock lock(_mutexMsg);
        if (_msgCount < MSG_MAX)
        {
            int index = (_msgIndex + _msgCount++) % MSG_MAX;
            Message& message = _msg[index];
            message.type = Message::UPDATE_DESKTOP;
            message.rect.x = x;
            message.rect.y = y;
            message.rect.width = width;
            message.rect.height = height;
        }
        else
        {
            Logger::instance().trace("ConsoleViewImpl::enableScale: Message queue is FULL!");
            _msgIndex = (_msgIndex + 1) % MSG_MAX;
            Message& message = _msg[_msgIndex];
            message.type = Message::UPDATE_DESKTOP;
            message.rect.x = x;
            message.rect.y = y;
            message.rect.width = width;
            message.rect.height = height;
        }
    }
    incRef();
    _dispatcher();
}


inline void ConsoleViewImpl::dispatchBeep()
{
    {
        Glib::Mutex::Lock lock(_mutexMsg);
        if (_msgCount < MSG_MAX)
        {
            int index = (_msgIndex + _msgCount++) % MSG_MAX;
            Message& message = _msg[index];
            message.type = Message::BEEP;
            message.rect.x = 0;
            message.rect.y = 0;
            message.rect.width = 0;
            message.rect.height = 0;
        }
        else
        {
            Logger::instance().trace("ConsoleViewImpl::enableScale: Message queue is FULL!");
            _msgIndex = (_msgIndex + 1) % MSG_MAX;
            Message& message = _msg[_msgIndex];
            message.type = Message::BEEP;
            message.rect.x = 0;
            message.rect.y = 0;
            message.rect.width = 0;
            message.rect.height = 0;
        }
    }
    incRef();
    _dispatcher();
}


void ConsoleViewImpl::onDispatched()
{
    Message message;
    {
        Glib::Mutex::Lock lock(_mutexMsg);
        if (!_msgCount)
        {
            return;
        }
        message = _msg[_msgIndex];
        _msgIndex = (_msgIndex + 1) % MSG_MAX;
        _msgCount--;
    }
    update(message);
    decRef();
}


void ConsoleViewImpl::sendPointerEvent(unsigned char buttonMask, int x, int y)
{
    {
        Glib::Mutex::Lock lock(_mutexFb);
        if (_scalingMultiplier && _scalingDivisor)
        {
            x = (x * _scalingDivisor) / _scalingMultiplier;
            y = (y * _scalingDivisor) / _scalingMultiplier;
        }
        int width = _frameBuffer->getWidth();
        int height = _frameBuffer->getHeight();
        if (x > width - 1)
        {
            x = width - 1;
        }
        if (y > height - 1)
        {
            y = height - 1;
        }
    }
    _console->sendPointerEvent(buttonMask, static_cast<unsigned short>(x), static_cast<unsigned short>(y));
}


void ConsoleViewImpl::sendKeyEvent(unsigned char downFlag, unsigned int keyval, unsigned int state, unsigned int keycode)
{
    if (_keyboardInputFilter)
    {
        if (_keyboardInputFilter->filter(downFlag, keyval, state, keycode))
        {
            return;
        }
    }
    if (keycode < 256 && keyval < 256)
    {
        if (downFlag)
        {
            keyvals[keycode] = keyval;
        }
        else
        {
            keyvals[keycode] = 0;
        }
    }
    _console->sendKeyEvent(downFlag, keyval, keycode);
}


#define SEND_EXPOSE 1


void ConsoleViewImpl::update(Message& msg)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::update", this));
    Glib::RefPtr<Gdk::Window> window = get_window();
    if (window)
    {
        switch (msg.type)
        {
        case Message::RESIZE_DESKTOP:
            _needInitScaling = true;
            TRACEPUT("invalidate_rect(true)");
            window->invalidate(true);
            break;
        case Message::UPDATE_DESKTOP:
        {
            _mutexFb.lock();
            if (_frameBufferScaled)
            {
                (this->*_pScale)(msg.rect);
            }
            _mutexFb.unlock();
#ifdef SEND_EXPOSE
            GdkEvent event;
            memset(&event, 0, sizeof(event));
            event.expose.type = GDK_EXPOSE;
            event.expose.window = window->gobj();
            event.expose.send_event = TRUE;
            event.expose.area = msg.rect;
            TRACEPUT("send_expose(%d,%d,%d,%d)", event.expose.area.x, event.expose.area.y, event.expose.area.width, event.expose.area.height);
            send_expose(&event);
#else
            TRACEPUT("invalidate_rect(%d,%d,%d,%d)", msg.rect.x, msg.rect.y, msg.rect.width, msg.rect.height);
            window->invalidate_rect(msg.rect, false);
#endif
            break;
        }
        case Message::BEEP:
            gdk_beep();
            break;
        default:
            break;
        }
    }
}


void ConsoleViewImpl::enableScale(bool value)
{
    if ((_scaleEnabled && !value) || (!_scaleEnabled && value))
    {
        int width, height;
        {
            Glib::Mutex::Lock lock(_mutexFb);
            _scaleEnabled = value;
            width = _frameBuffer->getWidth();
            height = _frameBuffer->getHeight();
        }
        dispatchUpdateDesktop(0, 0, width, height);
    }
}


// note: lock _mutexFb before calling this function.
void ConsoleViewImpl::initScaling(Glib::RefPtr<Gdk::Window> window)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::initScaling", this));

    int cxDtp = _frameBuffer->getWidth();
    int cyDtp = _frameBuffer->getHeight();
    TRACEPUT("console=%dx%d", cxDtp, cyDtp);

    _scalingMultiplier = 0;
    _scalingDivisor = 0;

    if (_scaleEnabled)
    {
        int cxWin = _containerWidth;
        int cyWin = _containerHeight;
        TRACEPUT("window=%dx%d", cxWin, cyWin);

        if (cxWin < cxDtp)
        {
            if (cyWin < cyDtp)
            {
                if (cxWin * cyDtp < cyWin * cxDtp)
                {
                    _scalingMultiplier = cxWin;
                    _scalingDivisor = cxDtp;
                }
                else
                {
                    _scalingMultiplier = cyWin;
                    _scalingDivisor = cyDtp;
                }
            }
            else
            {
                _scalingMultiplier = cxWin;
                _scalingDivisor = cxDtp;
            }
        }
        else if (cyWin < cyDtp)
        {
            _scalingMultiplier = cyWin;
            _scalingDivisor = cyDtp;
        }
    }

    if (_scalingMultiplier && _scalingDivisor)
    {
        TRACEPUT("scalingMultiplier=%d scalingDivisor=%d scale=%g", _scalingMultiplier,  _scalingDivisor, 1.0 * _scalingMultiplier / _scalingDivisor);
        GdkRectangle rect;
        rect.x = 0;
        rect.y = 0;
        rect.width = cxDtp;
        rect.height = cyDtp;
        cxDtp = (cxDtp * _scalingMultiplier) / _scalingDivisor;
        cyDtp = (cyDtp * _scalingMultiplier) / _scalingDivisor;
        if (!_frameBufferScaled || _frameBufferScaled->getWidth() != cxDtp || _frameBufferScaled->getHeight() != cyDtp)
        {
            _frameBufferScaled = FrameBuffer::create(cxDtp, cyDtp);
        }
        (this->*_pScale)(rect);
    }
    else
    {
        _frameBufferScaled = RefPtr<FrameBuffer>(NULL);
    }

    set_size_request(cxDtp, cyDtp);
}


void ConsoleViewImpl::scale(GdkRectangle& rect)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::initScaling", this), "x=%d y=%d width=%d height=%d", rect.x, rect.y, rect.width, rect.height);
    if (_needInitScaling)
    {
        return;
    }
    int xStart = (rect.x * _scalingMultiplier) / _scalingDivisor;
    int xEnd = ((rect.x + rect.width) * _scalingMultiplier + _scalingDivisor - 1) / _scalingDivisor;
    int xEnd2 = xEnd;
    if (xEnd >= _frameBufferScaled->getWidth())
    {
        xEnd2 = _frameBufferScaled->getWidth();
        xEnd = xEnd2 - 1;
    }
    int yStart = (rect.y * _scalingMultiplier) / _scalingDivisor;
    int yEnd = ((rect.y + rect.height) * _scalingMultiplier + _scalingDivisor - 1) / _scalingDivisor;
    int yEnd2 = yEnd;
    if (yEnd >= _frameBufferScaled->getHeight())
    {
        yEnd2 = _frameBufferScaled->getHeight();
        yEnd = yEnd2 - 1;
    }
    for (int y = yStart; y < yEnd; y++)
    {
        int yBase = y * _scalingDivisor / _scalingMultiplier;
        double yDist = 1.0 * y * _scalingDivisor / _scalingMultiplier - yBase;
        for (int x = xStart; x < xEnd; x++)
        {
            int xBase = x * _scalingDivisor / _scalingMultiplier;
            double xDist = 1.0 * x * _scalingDivisor / _scalingMultiplier - xBase;
            guchar* p1 = _frameBuffer->getData(xBase + 0, yBase + 0);
            guchar* p2 = _frameBuffer->getData(xBase + 1, yBase + 0);
            guchar* p3 = _frameBuffer->getData(xBase + 0, yBase + 1);
            guchar* p4 = _frameBuffer->getData(xBase + 1, yBase + 1);
            guchar* pDest = _frameBufferScaled->getData(x, y);
            double r =
                p1[0] * (1.0 - xDist) * (1.0 - yDist) +
                p2[0] * xDist * (1.0 - yDist) +
                p3[0] * (1.0 - xDist) * yDist +
                p4[0] * xDist * yDist;
            double g =
                p1[1] * (1.0 - xDist) * (1.0 - yDist) +
                p2[1] * xDist * (1.0 - yDist) +
                p3[1] * (1.0 - xDist) * yDist +
                p4[1] * xDist * yDist;
            double b =
                p1[2] * (1.0 - xDist) * (1.0 - yDist) +
                p2[2] * xDist * (1.0 - yDist) +
                p3[2] * (1.0 - xDist) * yDist +
                p4[2] * xDist * yDist;
            pDest[0] = static_cast<guchar>(r);
            pDest[1] = static_cast<guchar>(g);
            pDest[2] = static_cast<guchar>(b);
        }
        if (xEnd < xEnd2)
        {
            int xBase = xEnd * _scalingDivisor / _scalingMultiplier;
            double xDist = 1.0 * xEnd * _scalingDivisor / _scalingMultiplier - xBase;
            guchar* p1 = _frameBuffer->getData(xBase + 0, yBase + 0);
            guchar* p3 = _frameBuffer->getData(xBase + 0, yBase + 1);
            guchar* pDest = _frameBufferScaled->getData(xEnd, y);
            double r =
                p1[0] * (1.0 - xDist) * (1.0 - yDist) +
                p3[0] * (1.0 - xDist) * yDist;
            double g =
                p1[1] * (1.0 - xDist) * (1.0 - yDist) +
                p3[1] * (1.0 - xDist) * yDist;
            double b =
                p1[2] * (1.0 - xDist) * (1.0 - yDist) +
                p3[2] * (1.0 - xDist) * yDist;
            pDest[0] = static_cast<guchar>(r);
            pDest[1] = static_cast<guchar>(g);
            pDest[2] = static_cast<guchar>(b);
        }
    }
    if (yEnd < yEnd2)
    {
        int yBase = yEnd * _scalingDivisor / _scalingMultiplier;
        double yDist = 1.0 * yEnd * _scalingDivisor / _scalingMultiplier - yBase;
        for (int x = xStart; x < xEnd; x++)
        {
            int xBase = x * _scalingDivisor / _scalingMultiplier;
            double xDist = 1.0 * x * _scalingDivisor / _scalingMultiplier - xBase;
            guchar* p1 = _frameBuffer->getData(xBase + 0, yBase + 0);
            guchar* p2 = _frameBuffer->getData(xBase + 1, yBase + 0);
            guchar* pDest = _frameBufferScaled->getData(x, yEnd);
            double r =
                p1[0] * (1.0 - xDist) * (1.0 - yDist) +
                p2[0] * xDist * (1.0 - yDist);
            double g =
                p1[1] * (1.0 - xDist) * (1.0 - yDist) +
                p2[1] * xDist * (1.0 - yDist);
            double b =
                p1[2] * (1.0 - xDist) * (1.0 - yDist) +
                p2[2] * xDist * (1.0 - yDist);
            pDest[0] = static_cast<guchar>(r);
            pDest[1] = static_cast<guchar>(g);
            pDest[2] = static_cast<guchar>(b);
        }
        if (xEnd < xEnd2)
        {
            int xBase = xEnd * _scalingDivisor / _scalingMultiplier;
            double xDist = 1.0 * xEnd * _scalingDivisor / _scalingMultiplier - xBase;
            guchar* p1 = _frameBuffer->getData(xBase + 0, yBase + 0);
            guchar* pDest = _frameBufferScaled->getData(xEnd, yEnd);
            double r =
                p1[0] * (1.0 - xDist) * (1.0 - yDist);
            double g =
                p1[1] * (1.0 - xDist) * (1.0 - yDist);
            double b =
                p1[2] * (1.0 - xDist) * (1.0 - yDist);
            pDest[0] = static_cast<guchar>(r);
            pDest[1] = static_cast<guchar>(g);
            pDest[2] = static_cast<guchar>(b);
        }
    }
    rect.x = xStart;
    rect.y = yStart;
    rect.width = xEnd - xStart;
    rect.height = yEnd - yStart;
}


void ConsoleViewImpl::scaleByThreads(GdkRectangle& rect)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::scaleByThreads", this));
    Glib::Mutex::Lock lock(_mutexScale);
    const int heightMin = 16;
    if (rect.height <= heightMin)
    {
        scale(rect);
    }
    else
    {
        _mutexStart.lock();
        int t = sizeof(_scaleThreads) / sizeof(_scaleThreads[0]);
        int h = rect.height / t;
        if (h < heightMin)
        {
            h = heightMin;
        }
        int x = rect.x;
        int y = rect.y;
        int cx = rect.width;
        int cy = rect.height;
        int i;
        for (i = 0; i < t - 1 && cy > 0; i++)
        {
            _scaleRects[i].x = x;
            _scaleRects[i].y = y;
            _scaleRects[i].width = cx;
            _scaleRects[i].height = h < cy ? h : cy;
            y += _scaleRects[i].height;
            cy -= _scaleRects[i].height;
        }
        _scaleRects[i].x = x;
        _scaleRects[i].y = y;
        _scaleRects[i].width = cx;
        _scaleRects[i].height = cy;
        _remaining = _scaleCount = i + 1;
        TRACEPUT("Broadcasting...");
        _condStart.broadcast();
        _mutexCompleted.lock();
        _mutexStart.unlock();
        _condCompleted.wait(_mutexCompleted);
        _mutexCompleted.unlock();
        int xStart = (rect.x * _scalingMultiplier) / _scalingDivisor;
        int xEnd = ((rect.x + rect.width) * _scalingMultiplier + _scalingDivisor - 1) / _scalingDivisor;
        if (xEnd >= _frameBufferScaled->getWidth())
        {
            xEnd = _frameBufferScaled->getWidth() - 1;
        }
        int yStart = (rect.y * _scalingMultiplier) / _scalingDivisor;
        int yEnd = ((rect.y + rect.height) * _scalingMultiplier + _scalingDivisor - 1) / _scalingDivisor;
        if (yEnd >= _frameBufferScaled->getHeight())
        {
            yEnd = _frameBufferScaled->getHeight() - 1;
        }
        rect.x = xStart;
        rect.y = yStart;
        rect.width = xEnd - xStart;
        rect.height = yEnd - yStart;
    }
}


void ConsoleViewImpl::scaleWorker()
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::scaleWorker", this));
    _mutexStart.lock();
    for (;;)
    {
        if (_terminate)
        {
            break;
        }
        while (_scaleCount <= 0)
        {
            TRACEPUT("Waiting...");
            _condStart.wait(_mutexStart);
            if (_terminate)
            {
                break;
            }
        }
        int index = --_scaleCount;
        if (index < 0)
        {
            continue;
        }
        TRACEPUT("Resumed.");
        _mutexStart.unlock();
        scale(_scaleRects[index]);
        int ret = InterlockedDecrement(&_remaining);
        TRACEPUT("Done. remaining=%d", ret);
        if (!ret)
        {
            _mutexCompleted.lock();
            _condCompleted.signal();
            _mutexCompleted.unlock();
        }
        _mutexStart.lock();
    }
    _mutexStart.unlock();
}


void ConsoleViewImpl::enableScaleByThreads(bool value)
{
    Glib::Mutex::Lock lock(_mutexFb);
    _pScale = value ? &ConsoleViewImpl::scaleByThreads : &ConsoleViewImpl::scale;
}


void ConsoleViewImpl::onContainerResized(int cx, int cy)
{
    TRACE(StringBuffer().format("ConsoleViewImpl@%zx::onContainerResized", this));
    if (_scaleEnabled)
    {
        if (cx != _containerWidth || cy != _containerHeight)
        {
            _containerWidth = cx;
            _containerHeight = cy;
            _needInitScaling = true;
        }
    }
}


void ConsoleViewImpl::onContainerResized(Gtk::ScrolledWindow& sw)
{
    int cx = sw.get_width() - 2;
    int cy = sw.get_height() - 2;
    onContainerResized(cx, cy);
}


void ConsoleViewImpl::setKeyboardInputFilter(const RefPtr<ConsoleViewKeyboardInputFilter>& filter)
{
    _keyboardInputFilter = filter;
}


void ConsoleViewImpl::init(int width, int height, int bpp)
{
    {
        Glib::Mutex::Lock lock(_mutexFb);
        _bpp = bpp;
        if (width != _frameBuffer->getWidth() || height != _frameBuffer->getHeight())
        {
            _frameBuffer = FrameBuffer::create(width, height);
        }
    }
    dispatchResizeDesktop(width, height);
}


void ConsoleViewImpl::resize(int width, int height)
{
    {
        Glib::Mutex::Lock lock(_mutexFb);
        if (width != _frameBuffer->getWidth() || height != _frameBuffer->getHeight())
        {
            _frameBuffer = FrameBuffer::create(width, height);
        }
    }
    dispatchResizeDesktop(width, height);
}


void ConsoleViewImpl::copy(int x, int y, int width, int height, const unsigned char* data)
{
    {
        Glib::Mutex::Lock lock(_mutexFb);
        _frameBuffer->copy(x, y, width, height, data, _bpp, width * _bpp / 8);
    }
    dispatchUpdateDesktop(x, y, width, height);
}


void ConsoleViewImpl::bell()
{
    dispatchBeep();
}


int ConsoleViewImpl::getDefaultBpp()
{
    return _frameBuffer->getBpp();
}
