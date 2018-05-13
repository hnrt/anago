// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_CONSOLEVIEWIMPL_H
#define HNRT_CONSOLEVIEWIMPL_H


#include <gtkmm.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "Logger/Logger.h"
#include "ConsoleView.h"
#include "ConsoleViewKeyboardInputFilter.h"
#include "FrameScaler.h"


namespace hnrt
{
    class Console;
    class FrameBuffer;

    class ConsoleViewImpl
        : public Gtk::DrawingArea
        , public ConsoleView
        , public RefObj
    {
    public:

        static RefPtr<ConsoleViewImpl> create() { return RefPtr<ConsoleViewImpl>(new ConsoleViewImpl()); }

        virtual ~ConsoleViewImpl();
        const RefPtr<Console>& getConsole() const { return _console; }
        void open(const char* location, const char* authorization);
        void close();
        int getFrameWidth();
        int getFrameHeight();
        void enableScale(bool);
        void enableScaleByThreads(bool);
        void onContainerResized(int cx, int cy);
        void onContainerResized(Gtk::ScrolledWindow&);
        void setKeyboardInputFilter(const RefPtr<ConsoleViewKeyboardInputFilter>&);

        // implements ConsoleView
        virtual void init(int width, int height, int bpp);
        virtual void resize(int width, int height);
        virtual void copy(int x, int y, int width, int height, const unsigned char* data, int remaining);
        virtual void bell();
        virtual int getDefaultBpp();

    protected:

        class FrameBufferManager
        {
        public:

            inline FrameBufferManager(ConsoleViewImpl&);
            inline void init(int, int, int);
            inline void init(int, int);
            inline void setScaleFunc(FrameScaler::ScaleFunc);
            inline void setContainerSize(int, int);
            inline RefPtr<FrameBuffer> getFrameBuffer();
            inline RefPtr<FrameBuffer> getScaledFrameBuffer();
            inline void requestResize();
            inline void scale(GdkRectangle&);
            inline void translateVirtualCoordinates(int&, int&);
            inline int bpp() const { return _bpp; }
            inline bool scaleEnabled() const { return _scaleEnabled; }
            inline bool setScaleEnabled(bool, int&, int&);

        private:

            FrameBufferManager(const FrameBufferManager&);
            void operator =(const FrameBufferManager&);
            inline void resize();

            ConsoleViewImpl& _view;
            Glib::Mutex _mutex;
            RefPtr<FrameBuffer> _frameBuffer;
            RefPtr<FrameBuffer> _frameBufferScaled;
            int _bpp;
            bool _scaleEnabled;
            int _scalingMultiplier;
            int _scalingDivisor;
            bool _resizeRequested;
            int _containerWidth;
            int _containerHeight;
            FrameScaler::ScaleFunc _scale;
        };

        class MessageQueue
        {
        public:

            inline MessageQueue();
            inline ~MessageQueue();
            inline void enqueue(Message::Type, int = 0, int = 0, int = 0, int = 0);
            inline bool dequeue(Message&);

        private:

            MessageQueue(const MessageQueue&);
            void operator =(const MessageQueue&);
            Glib::Mutex _mutex;
            size_t _head;
            size_t _tail;
            size_t _length;
            Message* _slots;
        };

        ConsoleViewImpl();
        ConsoleViewImpl(const ConsoleViewImpl&);
        void operator =(const ConsoleViewImpl&);
        void run(Glib::ustring location, Glib::ustring authorization);
        virtual bool on_configure_event(GdkEventConfigure*);
        virtual bool on_expose_event(GdkEventExpose*);
        virtual bool on_enter_notify_event(GdkEventCrossing*);
        virtual bool on_leave_notify_event(GdkEventCrossing*);
        virtual bool on_motion_notify_event(GdkEventMotion*);
        virtual bool on_button_press_event(GdkEventButton*);
        virtual bool on_button_release_event(GdkEventButton*);
        virtual bool on_scroll_event(GdkEventScroll*);
        virtual bool on_focus_in_event(GdkEventFocus*);
        virtual bool on_focus_out_event(GdkEventFocus*);
        virtual bool on_key_press_event(GdkEventKey*);
        virtual bool on_key_release_event(GdkEventKey*);
        inline void dispatchResizeDesktop(int, int);
        inline void dispatchUpdateDesktop(int, int, int, int);
        inline void dispatchBeep();
        void onDispatched();
        void sendPointerEvent(unsigned char buttonMask, int x, int y);
        void sendKeyEvent(unsigned char downFlag, unsigned int keyval, unsigned int state, unsigned int keycode);
        guint convertKey(guint keycode, guint modstate, guint keyval);
        void update(Message&);

        RefPtr<Console> _console;
        Glib::Thread* _consoleThread;
        bool _consoleClosing;
        sigc::connection _connection;
        FrameBufferManager _fbMgr;
        FrameScaler& _scaler;
        bool _hasFocus;
        RefPtr<ConsoleViewKeyboardInputFilter> _keyboardInputFilter;
        unsigned char _keyvals[256];
        MessageQueue _msgQueue;
        Glib::Dispatcher _dispatcher;
        GdkRectangle _updatedRectangle;
    };

    inline ConsoleViewImpl::MessageQueue::MessageQueue()
        : _head(0)
        , _tail(0)
        , _length(32)
        , _slots(new Message[_length])
    {
    }

    inline ConsoleViewImpl::MessageQueue::~MessageQueue()
    {
        delete[] _slots;
    }

    inline void ConsoleViewImpl::MessageQueue::enqueue(Message::Type type, int x, int y, int width, int height)
    {
        Glib::Mutex::Lock lock(_mutex);
        size_t count = _tail - _head;
        if (count >= _length)
        {
            size_t newLength = _length < 65536 ? _length * 2 : _length + 65536;
            Logger::instance().trace("ConsoleViewImpl::MessageQueue: Message queue is FULL! length %zu ==>> %zu", _length, newLength);
            Message* newSlots = new Message[newLength];
            for (size_t current = _head; current < _tail; current++)
            {
                newSlots[current % newLength] = _slots[current % _length];
            }
            delete[] _slots;
            _slots = newSlots;
            _length = newLength;
        }
        Message& message = _slots[_tail++ % _length];
        message.type = type;
        message.rect.x = x;
        message.rect.y = y;
        message.rect.width = width;
        message.rect.height = height;
    }

    inline bool ConsoleViewImpl::MessageQueue::dequeue(Message& message)
    {
        Glib::Mutex::Lock lock(_mutex);
        if (_head < _tail)
        {
            message = _slots[_head++ % _length];
            return true;
        }
        else
        {
            return false;
        }
    }
}


#endif //!HNRT_CONSOLEVIEWIMPL_H
