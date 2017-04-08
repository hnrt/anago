// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CONSOLEIMP_H
#define HNRT_CONSOLEIMP_H


#include <glibmm.h>
#include "Protocol/Rfb.h"
#include "Console.h"
#include "ConsoleConnector.h"


namespace hnrt
{
    class ConsoleImpl
        : public Console
        , public ConsoleConnector
    {
    public:

        ConsoleImpl(ConsoleView&);
        virtual ~ConsoleImpl();
        virtual bool isActive() const;
        virtual void open(const char* location, const char* authorization);
        virtual void close();
        virtual void run();
        virtual void terminate();
        virtual void sendPointerEvent(unsigned char buttonMask, unsigned short x, unsigned short y);
        virtual void sendKeyEvent(unsigned char downFlag, unsigned int keyval, unsigned int keycode);
        virtual void sendCtrlAltDelete();
        virtual int statusCode() const { return _statusCode; }

    protected:

        ConsoleImpl(const ConsoleImpl&);
        void operator =(const ConsoleImpl&);
        inline bool canContinue() const;
        inline bool isServerNotResponding() const;
        void rxMain();
        void txMain();
        void processIncomingData();
        bool processOutgoingData();

        ConsoleView& _view;
        Glib::Mutex _mutexPx;
        Glib::Cond _condPx;
        Glib::Mutex _mutexRx;
        Glib::Cond _condRx;
        Glib::Mutex _mutexTx;
        Glib::Cond _condTx;
        Glib::Mutex _mutexTxBuf;
        Glib::ustring _location;
        Glib::ustring _authorization;
        volatile bool _terminate;
        volatile int _state;
        int _protocolVersion;
        Rfb::PixelFormat _pixelFormat;
        int _width;
        int _height;
        Glib::ustring _name;
        unsigned char _incremental;
        int _numRects;
        int _rectIndex;
        time_t _fbupdateAt;
        time_t _fbupdateLimit;
        unsigned long _fbupdateCount;
        unsigned long _reconnectCount;
        bool _scanCodeEnabled;
    };
}


#endif //!HNRT_CONSOLEIMP_H
