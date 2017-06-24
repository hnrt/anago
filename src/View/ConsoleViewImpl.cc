// Copyright (C) 2012-2017 Hideaki Narita


#define NO_TRACE


#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "Base/Atomic.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "Net/Console.h"
#include "Thread/ThreadManager.h"
#include "ConsoleViewImpl.h"
#include "FrameBuffer.h"
#include "FrameScalerImpl.h"


using namespace hnrt;


ConsoleViewImpl::ConsoleViewImpl()
    : _console(Console::create(*this))
    , _consoleThread(NULL)
    , _fbMgr(*this)
    , _scaler(*new FrameScalerImpl())
    , _hasFocus(false)
{
    TRACEFUN(this, "ConsoleViewImpl::ctor");

    _fbMgr.setScaleFunc(&FrameScaler::scaleInParallel);

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

    memset(_keyvals, 0, sizeof(_keyvals));
    memset(&_updatedRectangle, 0, sizeof(_updatedRectangle));

    _connection = _dispatcher.connect(sigc::mem_fun(*this, &ConsoleViewImpl::onDispatched));
}


ConsoleViewImpl::~ConsoleViewImpl()
{
    TRACEFUN(this, "ConsoleViewImpl::dtor");

    close();

    _connection.disconnect();

    delete &_scaler;
}


inline ConsoleViewImpl::FrameBufferManager::FrameBufferManager(ConsoleViewImpl& view)
    : _view(view)
    , _frameBuffer(FrameBuffer::create(0, 0))
    , _bpp(0)
    , _scaleEnabled(false)
    , _scalingMultiplier(0)
    , _scalingDivisor(0)
    , _resizeRequested(true)
    , _containerWidth(65535)
    , _containerHeight(65535)
{
}


inline void ConsoleViewImpl::FrameBufferManager::init(int width, int height, int bpp)
{
    TRACEFUN(this, "FrameBufferManager::init");
    Glib::Mutex::Lock lock(_mutex);
    _bpp = bpp;
    if (width != _frameBuffer->getWidth() || height != _frameBuffer->getHeight())
    {
        TRACEPUT("%dx%d", width, height);
        _frameBuffer = FrameBuffer::create(width, height);
    }
}


inline void ConsoleViewImpl::FrameBufferManager::init(int width, int height)
{
    init(width, height, _bpp);
}


inline void ConsoleViewImpl::FrameBufferManager::setScaleFunc(FrameScaler::ScaleFunc scale)
{
    Glib::Mutex::Lock lock(_mutex);
    _scale = scale;
}


inline void ConsoleViewImpl::FrameBufferManager::setContainerSize(int width, int height)
{
    TRACEFUN(this, "FrameBufferManager::setContainerSize");
    Glib::Mutex::Lock lock(_mutex);
    if (width != _containerWidth || height != _containerHeight)
    {
        _containerWidth = width;
        _containerHeight = height;
        if (_scaleEnabled)
        {
            _resizeRequested = true;
        }
    }
}


inline RefPtr<FrameBuffer> ConsoleViewImpl::FrameBufferManager::getFrameBuffer()
{
    Glib::Mutex::Lock lock(_mutex);
    return _frameBuffer;
}


inline RefPtr<FrameBuffer> ConsoleViewImpl::FrameBufferManager::getScaledFrameBuffer()
{
    Glib::Mutex::Lock lock(_mutex);
    if (_resizeRequested)
    {
        _resizeRequested = false;
        resize();
    }
    return _frameBufferScaled ? _frameBufferScaled : _frameBuffer;
}


inline void ConsoleViewImpl::FrameBufferManager::requestResize()
{
    _resizeRequested = true;
}


inline void ConsoleViewImpl::FrameBufferManager::scale(GdkRectangle& rect)
{
    Glib::Mutex::Lock lock(_mutex);
    if (_frameBufferScaled && !_resizeRequested)
    {
        (_view._scaler.*_scale)(_frameBuffer, _frameBufferScaled, _scalingMultiplier, _scalingDivisor, rect);
    }
}


inline void ConsoleViewImpl::FrameBufferManager::translateVirtualCoordinates(int& x, int& y)
{
    Glib::Mutex::Lock lock(_mutex);
    if (_scalingMultiplier && _scalingDivisor)
    {
        x = (x * _scalingDivisor) / _scalingMultiplier;
        y = (y * _scalingDivisor) / _scalingMultiplier;
    }
    int width = _frameBuffer->getWidth();
    int height = _frameBuffer->getHeight();
    if (x < 0)
    {
        x = 0;
    }
    else if (x > width - 1)
    {
        x = width - 1;
    }
    if (y < 0)
    {
        y = 0;
    }
    else if (y > height - 1)
    {
        y = height - 1;
    }
}


inline bool ConsoleViewImpl::FrameBufferManager::setScaleEnabled(bool value, int& width, int& height)
{
    Glib::Mutex::Lock lock(_mutex);
    if ((_scaleEnabled && !value) || (!_scaleEnabled && value))
    {
        _scaleEnabled = value;
        width = _frameBuffer->getWidth();
        height = _frameBuffer->getHeight();
        return true;
    }
    else
    {
        return false;
    }
}


// note: lock _mutex before calling this function.
inline void ConsoleViewImpl::FrameBufferManager::resize()
{
    TRACEFUN(this, "FrameBufferManager::resize");

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
        (_view._scaler.*_scale)(_frameBuffer, _frameBufferScaled, _scalingMultiplier, _scalingDivisor, rect);
    }
    else
    {
        _frameBufferScaled.reset();
    }

    _view.set_size_request(cxDtp, cyDtp);
}


void ConsoleViewImpl::open(const char* location, const char* authorization)
{
    TRACEFUN(this, "ConsoleViewImpl::open");
    close();
    _scaler.init();
    _consoleThread = ThreadManager::instance().create(sigc::bind<Glib::ustring, Glib::ustring>(sigc::mem_fun(*this, &ConsoleViewImpl::run), Glib::ustring(location), Glib::ustring(authorization)), true, "Console");
    _consoleThread->set_priority(Glib::THREAD_PRIORITY_HIGH);
}


void ConsoleViewImpl::close()
{
    TRACEFUN(this, "ConsoleViewImpl::close");
    Glib::Thread* thread = InterlockedExchangePointer(&_consoleThread, (Glib::Thread*)NULL);
    if (thread)
    {
        _console->terminate();
        thread->join();
        if (_console->statusCode() == 200)
        {
            RefPtr<FrameBuffer> fb = _fbMgr.getFrameBuffer();
            int width = fb->getWidth();
            int height = fb->getHeight();
            if (width > 0 && height > 0)
            {
                fb->changeColor(0.5);
                dispatchUpdateDesktop(0, 0, width, height);
            }
        }
    }
    _scaler.fini();
}


void ConsoleViewImpl::run(Glib::ustring location, Glib::ustring authorization)
{
    TRACEFUN(this, "ConsoleViewImpl::run");
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
    return _fbMgr.getFrameBuffer()->getWidth();
}


int ConsoleViewImpl::getFrameHeight()
{
    return _fbMgr.getFrameBuffer()->getHeight();
}


bool ConsoleViewImpl::on_configure_event(GdkEventConfigure* event)
{
    TRACEFUN(this, "ConsoleViewImpl::on_configure_event");
    _fbMgr.requestResize();
    return false;
}


bool ConsoleViewImpl::on_expose_event(GdkEventExpose* event)
{
    TRACEFUN(this, "ConsoleViewImpl::on_expose_event");
    Glib::RefPtr<Gdk::Window> window = get_window();
    RefPtr<FrameBuffer> fb = _fbMgr.getScaledFrameBuffer();
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
    TRACEFUN(this, "ConsoleViewImpl::on_enter_notify_event");
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
    TRACEFUN(this, "ConsoleViewImpl::on_leave_notify_event");
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
    TRACEFUN(this, "ConsoleViewImpl::on_focus_in_event");
    _hasFocus = true;
    return true;
}


bool ConsoleViewImpl::on_focus_out_event(GdkEventFocus* event)
{
    TRACEFUN(this, "ConsoleViewImpl::on_focus_out_event");
    _hasFocus = false;
    for (unsigned int keycode = 0; keycode < 256; keycode++)
    {
        if (_keyvals[keycode])
        {
            TRACEPUT("sendKeyEvent(0,%u,%u)", _keyvals[keycode], keycode);
            _console->sendKeyEvent(0, _keyvals[keycode], keycode);
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
    _msgQueue.enqueue(Message::RESIZE_DESKTOP, 0, 0, width, height);
    incRef();
    _dispatcher();
}


inline void ConsoleViewImpl::dispatchUpdateDesktop(int x, int y, int width, int height)
{
    _msgQueue.enqueue(Message::UPDATE_DESKTOP, x, y, width, height);
    incRef();
    _dispatcher();
}


inline void ConsoleViewImpl::dispatchBeep()
{
    _msgQueue.enqueue(Message::BEEP);
    incRef();
    _dispatcher();
}


void ConsoleViewImpl::onDispatched()
{
    Message message;
    if (_msgQueue.dequeue(message))
    {
        update(message);
        decRef();
    }
}


void ConsoleViewImpl::sendPointerEvent(unsigned char buttonMask, int x, int y)
{
    _fbMgr.translateVirtualCoordinates(x, y);
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
            _keyvals[keycode] = keyval;
        }
        else
        {
            _keyvals[keycode] = 0;
        }
    }
    _console->sendKeyEvent(downFlag, keyval, keycode);
}


#define SEND_EXPOSE 1


void ConsoleViewImpl::update(Message& msg)
{
    TRACEFUN(this, "ConsoleViewImpl::update");
    Glib::RefPtr<Gdk::Window> window = get_window();
    if (window)
    {
        switch (msg.type)
        {
        case Message::RESIZE_DESKTOP:
            _fbMgr.requestResize();
            TRACEPUT("invalidate_rect(true)");
            window->invalidate(true);
            break;
        case Message::UPDATE_DESKTOP:
        {
            _fbMgr.scale(msg.rect);
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
    TRACEFUN(this, "ConsoleViewImpl::enableScale(%d)", value);
    int width = 0, height = 0;
    if (_fbMgr.setScaleEnabled(value, width, height))
    {
        dispatchResizeDesktop(width, height);
    }
}


void ConsoleViewImpl::enableScaleByThreads(bool value)
{
    _fbMgr.setScaleFunc(value ? &FrameScaler::scaleInParallel : &FrameScaler::scale);
}


void ConsoleViewImpl::onContainerResized(int cx, int cy)
{
    TRACEFUN(this, "ConsoleViewImpl::onContainerResized(%d,%d)", cx, cy);
    _fbMgr.setContainerSize(cx, cy);
}


void ConsoleViewImpl::onContainerResized(Gtk::ScrolledWindow& sw)
{
    TRACEFUN(this, "ConsoleViewImpl::onContainerResized");
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
    _fbMgr.init(width, height, bpp);
    dispatchResizeDesktop(width, height);
    memset(&_updatedRectangle, 0, sizeof(_updatedRectangle));
}


void ConsoleViewImpl::resize(int width, int height)
{
    _fbMgr.init(width, height);
    dispatchResizeDesktop(width, height);
    memset(&_updatedRectangle, 0, sizeof(_updatedRectangle));
}


void ConsoleViewImpl::copy(int x, int y, int width, int height, const unsigned char* data, int remaining)
{
    RefPtr<FrameBuffer> fb = _fbMgr.getFrameBuffer();
    int bpp = _fbMgr.bpp();
    fb->copy(x, y, width, height, data, bpp, width * bpp / 8);
    if (!_updatedRectangle.width)
    {
        _updatedRectangle.x = x;
        _updatedRectangle.width = width;
    }
    else
    {
        int xendCur = _updatedRectangle.x + _updatedRectangle.width;
        int xendNew = x + width;
        if (_updatedRectangle.x > x)
        {
            _updatedRectangle.x = x;
        }
        if (xendCur < xendNew)
        {
            xendCur = xendNew;
        }
        _updatedRectangle.width = xendCur - _updatedRectangle.x;
    }
    if (!_updatedRectangle.height)
    {
        _updatedRectangle.y = y;
        _updatedRectangle.height = height;
    }
    else
    {
        int yendCur = _updatedRectangle.y + _updatedRectangle.height;
        int yendNew = y + height;
        if (_updatedRectangle.y > y)
        {
            _updatedRectangle.y = y;
        }
        if (yendCur < yendNew)
        {
            yendCur = yendNew;
        }
        _updatedRectangle.height = yendCur - _updatedRectangle.y;
    }
    if (remaining <= 0)
    {
        dispatchUpdateDesktop(_updatedRectangle.x, _updatedRectangle.y, _updatedRectangle.width, _updatedRectangle.height);
        memset(&_updatedRectangle, 0, sizeof(_updatedRectangle));
    }
}


void ConsoleViewImpl::bell()
{
    dispatchBeep();
}


int ConsoleViewImpl::getDefaultBpp()
{
    return _fbMgr.getFrameBuffer()->getBpp();
}
