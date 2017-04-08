// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLEVIEWIMPL_H
#define HNRT_CONSOLEVIEWIMPL_H


#include <gtkmm.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "ConsoleView.h"
#include "ConsoleViewKeyboardInputFilter.h"


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

        enum ProtectedConstants
        {
            MSG_MAX = 8192,
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
        void initScaling(Glib::RefPtr<Gdk::Window>);
        void scale(GdkRectangle&);
        void scaleByThreads(GdkRectangle&);
        void scaleWorker();

        RefPtr<Console> _console;
        Glib::Thread* _consoleThread;
        sigc::connection _connection;
        Glib::Mutex _mutexFb;
        RefPtr<FrameBuffer> _frameBuffer;
        RefPtr<FrameBuffer> _frameBufferScaled;
        int _bpp;
        bool _hasFocus;
        bool _scaleEnabled;
        int _scalingMultiplier;
        int _scalingDivisor;
        bool _needInitScaling;
        int _containerWidth;
        int _containerHeight;
        Glib::Mutex _mutexScale;
        Glib::Mutex _mutexStart;
        Glib::Cond _condStart;
        Glib::Mutex _mutexCompleted;
        Glib::Cond _condCompleted;
        Glib::Thread* _scaleThreads[4];
        GdkRectangle _scaleRects[4];
        bool _terminate;
        int _scaleCount;
        int _remaining;
        void (ConsoleViewImpl::*_pScale)(GdkRectangle&);
        RefPtr<ConsoleViewKeyboardInputFilter> _keyboardInputFilter;
        unsigned char _keyvals[256];
        Glib::Mutex _mutexMsg;
        volatile int _msgIndex;
        volatile int _msgCount;
        Message _msg[MSG_MAX];
        Glib::Dispatcher _dispatcher;
        GdkRectangle _updatedRectangle;
    };
}


#endif //!HNRT_CONSOLEVIEWIMPL_H
