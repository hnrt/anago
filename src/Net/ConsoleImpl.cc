// Copyright (C) 2012-2017 Hideaki Narita


#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Exception/ConsoleException.h"
#include "Logger/Trace.h"
#include "Thread/ThreadManager.h"
#include "View/ConsoleView.h"
#include "ConsoleImpl.h"


// cf. X11/keysymdef.h
#define XK_Insert    0xff63
#define XK_KP_0      0xffb0
#define XK_KP_1      0xffb1
#define XK_KP_2      0xffb2
#define XK_KP_3      0xffb3
#define XK_KP_4      0xffb4
#define XK_KP_5      0xffb5
#define XK_KP_6      0xffb6
#define XK_KP_7      0xffb7
#define XK_KP_8      0xffb8
#define XK_KP_9      0xffb9
#define XK_Shift_L   0xffe1
#define XK_Shift_R   0xffe2
#define XK_Control_L 0xffe3
#define XK_Control_R 0xffe4
#define XK_Meta_L    0xffe7
#define XK_Meta_R    0xffe8
#define XK_Alt_L     0xffe9
#define XK_Alt_R     0xffea
#define XK_Delete    0xffff


using namespace hnrt;


#define STATE_CONNECT_RESPONSE     0


#define STATE_HANDSHAKING_PHASE    10
#define STATE_START                (STATE_HANDSHAKING_PHASE+0)
#define STATE_START_RESPONSE       (STATE_HANDSHAKING_PHASE+1)
#define STATE_SECURITY37           (STATE_HANDSHAKING_PHASE+2)
#define STATE_SECURITY37_RESPONSE  (STATE_HANDSHAKING_PHASE+3)
#define STATE_SECURITY33           (STATE_HANDSHAKING_PHASE+4)
#define STATE_CONNECTION_FAILURE   (STATE_HANDSHAKING_PHASE+5)
#define STATE_SECURITY_RESULT      (STATE_HANDSHAKING_PHASE+6)
#define STATE_SECURITY_FAILURE     (STATE_HANDSHAKING_PHASE+7)

#define STATE_INITIALIZATION_PHASE 20
#define STATE_CLIENTINIT           (STATE_INITIALIZATION_PHASE+0)
#define STATE_SERVERINIT           (STATE_INITIALIZATION_PHASE+1)

#define STATE_INTERACTION_PHASE    30
#define STATE_SETPIXELFORMAT       (STATE_INTERACTION_PHASE+0)
#define STATE_SETENCODINGS         (STATE_INTERACTION_PHASE+1)
#define STATE_FBUPDATEREQUEST      (STATE_INTERACTION_PHASE+2)
#define STATE_READY                (STATE_INTERACTION_PHASE+3)
#define STATE_FBUPDATE             (STATE_INTERACTION_PHASE+4)

#define STATE_COMPLETED            99
#define STATE_ERROR                100
#define STATE_UNSUPPORTED          101
#define STATE_DISCONNECTED_BY_HOST 102
#define STATE_CLOSED               199


#define READY_COUNT_THRESHOLD(x)   ((unsigned long)(3000 * pow(2, (x) / 10)))


ConsoleImpl::ConsoleImpl(ConsoleView& view)
    : _view(view)
    , _terminate(false)
    , _state(STATE_CLOSED)
    , _protocolVersion(0)
    , _width(0)
    , _height(0)
    , _name()
    , _incremental(0)
    , _numRects(0)
    , _rectIndex(0)
    , _updateCount(0)
    , _readyCount(0)
    , _readyCountThreshold(READY_COUNT_THRESHOLD(0))
    , _reconnectCount(0)
    , _scanCodeEnabled(true)
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::ctor", this));
}


ConsoleImpl::~ConsoleImpl()
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::dtor", this));
}


bool ConsoleImpl::isActive() const
{
    return _curl != NULL;
}


void ConsoleImpl::open(const char* location, const char* authorization)
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::open", this));

    try
    {
        _terminate = false;
        _state = STATE_CONNECT_RESPONSE;
        _updateCount = 0;
        ConsoleConnector::open(location, authorization);
        _location = location;
        _authorization = authorization;
        _reconnectCount = 0;
        _readyCount = 0;
        _readyCountThreshold = READY_COUNT_THRESHOLD(_reconnectCount);
    }
    catch (LocationConsoleException ex)
    {
        ConsoleConnector::close();
        _state = STATE_CLOSED;
        // ignore this kind of failure
    }
    catch (ConsoleException ex)
    {
        ConsoleConnector::close();
        _state = STATE_CLOSED;
        throw ex;
    }
}


void ConsoleImpl::close()
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::close", this));

    if (InterlockedExchange(&_state, STATE_CLOSED) != STATE_CLOSED)
    {
        ConsoleConnector::close();
    }
}


void ConsoleImpl::run()
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::run", this));

    Glib::Thread* rx = NULL;
    Glib::Thread* tx = NULL;

    try
    {
        rx = ThreadManager::instance().create(sigc::mem_fun(*this, &ConsoleImpl::rxMain), true, "ConsoleRx");
        tx = ThreadManager::instance().create(sigc::mem_fun(*this, &ConsoleImpl::txMain), true, "ConsoleTx");

        Glib::Mutex::Lock lock(_mutexPx);
        while (!_terminate && _state < STATE_COMPLETED)
        {
            processIncomingData();

            if (_state < STATE_COMPLETED)
            {
                if (processOutgoingData())
                {
                    Glib::TimeVal timeout;
                    timeout.assign_current_time();
                    timeout.add_milliseconds(100);
                    _condPx.timed_wait(_mutexPx, timeout);
                }
            }

#if 0
            if (_readyCount >= _readyCountThreshold)
            {
                if (_terminate)
                {
                    break;
                }
                // CentOS 7 is likely to respond no framebuffer update while it is starting up.
                // Reconnect is needed to avoid such situation after confirming no send data.
                try
                {
                    Glib::Mutex::Lock lock(_mutexTx);
                    for (;;)
                    {
                        ssize_t n = send();
                        if (n == 0)
                        {
                            fd_set fds;
                            FD_ZERO(&fds);
                            FD_SET(_sockHost, &fds);
                            struct timeval timeout;
                            timeout.tv_sec = 0;
                            timeout.tv_usec = 1000;
                            int rc = select(_sockHost + 1, NULL, &fds, NULL, &timeout);
                            if (rc < 0)
                            {
                                throw ConsoleException("select failed: %s", strerror(errno));
                            }
                        }
                        else if (n < 0)
                        {
                            // no more data
                            break;
                        }
                    }
                    TRACEPUT("Reconnecting...");
                    close();
                    _state = STATE_CONNECT_RESPONSE;
                    _updateCount = 0;
                    ConsoleConnector::open(_location.c_str(), _authorization.c_str());
                    _readyCount = 0;
                    _reconnectCount++;
                    if (_reconnectCount % 10 == 0)
                    {
                        _readyCountThreshold = READY_COUNT_THRESHOLD(_reconnectCount);
                    }
                }
                catch (Exception ex)
                {
                    _state = STATE_CLOSED;
                    ConsoleConnector::close();
                }
            }
            else if (_readyCount > 0 && _readyCount % 1000 == 0)
            {
                TRACEPUT("Server not responding.");
            }
#endif
        }
    }
    catch (std::bad_alloc)
    {
        Logger::instance().error("ConsoleImpl::run: Out of memory.");
    }
    catch (ConsoleException ex)
    {
        Logger::instance().error("ConsoleImpl::run: %s", ex.what().c_str());
    }
    catch (...)
    {
        Logger::instance().error("ConsoleImpl::run: Unhandled exception caught.");
    }

    _terminate = true;

    _condRx.signal();
    _condTx.signal();

    if (rx)
    {
        rx->join();
    }

    if (tx)
    {
        tx->join();
    }
}


void ConsoleImpl::rxMain()
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::rxMain", this));
    try
    {
        Glib::Mutex::Lock lock(_mutexRx);
        while (!_terminate && _state < STATE_COMPLETED)
        {
            if (_ibuf.space() <= 0)
            {
                TRACEPUT("BUFFER FULL!");
                _condPx.signal();
                Glib::TimeVal timeout;
                timeout.assign_current_time();
                timeout.add_milliseconds(1000);
                _condRx.timed_wait(_mutexRx, timeout);
            }
            else
            {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(_sockHost, &fds);
                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                int rc = select(_sockHost + 1, &fds, NULL, NULL, &timeout);
                if (rc < 0)
                {
                    Logger::instance().error("ConsoleImpl::rxMain: select failed: %s", strerror(errno));
                    _state = STATE_ERROR;
                }
                else if (recv() > 0)
                {
                    _condPx.signal();
                }
            }
        }
    }
    catch (ConsoleException ex)
    {
        Logger::instance().error("ConsoleImpl::rxMain: %s", ex.what().c_str());
        _state = STATE_ERROR;
    }
    catch (...)
    {
        Logger::instance().error("ConsoleImpl::rxMain: Unhandled exception caught.");
        _state = STATE_ERROR;
    }
}


void ConsoleImpl::txMain()
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::txMain", this));
    try
    {
        Glib::Mutex::Lock lock(_mutexTx);
        while (!_terminate && _state < STATE_COMPLETED)
        {
            if (_obuf.remaining() <= 0)
            {
                Glib::TimeVal timeout;
                timeout.assign_current_time();
                timeout.add_milliseconds(1000);
                _condTx.timed_wait(_mutexTx, timeout);
            }
            else if (send() == 0)
            {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(_sockHost, &fds);
                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                int rc = select(_sockHost + 1, NULL, &fds, NULL, &timeout);
                if (rc < 0)
                {
                    Logger::instance().error("ConsoleImpl::txMain: select failed: %s", strerror(errno));
                    _state = STATE_ERROR;
                }
            }
        }
    }
    catch (ConsoleException ex)
    {
        Logger::instance().error("ConsoleImpl::txMain: %s", ex.what().c_str());
        _state = STATE_ERROR;
    }
    catch (...)
    {
        Logger::instance().error("ConsoleImpl::txMain: Unhandled exception caught.");
        _state = STATE_ERROR;
    }
}


void ConsoleImpl::terminate()
{
    _terminate = true;
}


void ConsoleImpl::processIncomingData()
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::processIncomingData", this));

    try
    {
        while (_ibuf.remaining() > 0)
        {
            switch (_state)
            {
            case STATE_CONNECT_RESPONSE:
            {
                size_t headerLength = 0;
                if (!getHeaderLength(headerLength))
                {
                    // not yet received all of the response -- waiting for the rest
                    goto done;
                }
                if (!parseHeader(headerLength))
                {
                    _state = STATE_ERROR;
                    throw ConsoleException("Malformed response headers: %.*s", static_cast<int>(headerLength), _ibuf.rPtr());
                }
                _ibuf.rPos(_ibuf.rPos() + headerLength);
                TRACEPUT("CONNECT status code %d", _statusCode);
                if (_statusCode == 200)
                {
                    InterlockedCompareExchange(&_state, STATE_START, STATE_CONNECT_RESPONSE);
                }
                else if (_statusCode == 404)
                {
                    // server side console object cannot be found probably because of shutdown
                    InterlockedCompareExchange(&_state, STATE_COMPLETED, STATE_CONNECT_RESPONSE);
                    goto done;
                }
                else
                {
                    _state = STATE_ERROR;
                    throw ConsoleException("CONNECT response status: %d", _statusCode);
                }
                //FALLTHROUGH
            }

            case STATE_START:
            {
                Rfb::ProtocolVersion pv(_ibuf);
                _protocolVersion = pv.parse();
                TRACEPUT("ProtocolVersion: %d.%d", _protocolVersion / 1000, _protocolVersion % 1000);
                if (_protocolVersion > 3008)
                {
                    _protocolVersion = 3008;
                }
                else if (3003 < _protocolVersion && _protocolVersion < 3007)
                {
                    _protocolVersion = 3003;
                }
                InterlockedCompareExchange(&_state, STATE_START_RESPONSE, STATE_START);
                goto done;
            }

            case STATE_SECURITY37:
            {
                Rfb::Security37 s(_ibuf);
                TRACEPUT("number-of-security-types=%d", s.numberOfSecurityTypes);
                for (Rfb::U32 i = 0; i < s.numberOfSecurityTypes; i++)
                {
                    TRACEPUT("security-type[%u]=%d", i, s.securityTypes[i]);
                }
                InterlockedCompareExchange(&_state, STATE_SECURITY37_RESPONSE, STATE_SECURITY37);
                goto done;
            }

            case STATE_SECURITY33:
            {
                Rfb::Security33 s(_ibuf);
                TRACEPUT("security-type=%d", s.securityType);
                if (s.securityType == Rfb::INVALID)
                {
                    _state = STATE_CONNECTION_FAILURE;
                    break;
                }
                else if (s.securityType == Rfb::NONE)
                {
                    InterlockedCompareExchange(&_state, STATE_CLIENTINIT, STATE_SECURITY33);
                    goto done;
                }
                else
                {
                    _state = STATE_UNSUPPORTED;
                    goto done;
                }
            }

            case STATE_CONNECTION_FAILURE:
            case STATE_SECURITY_FAILURE:
            {
                Rfb::FailureDescription fd(_ibuf);
                _state = STATE_ERROR;
                throw ConsoleException(
                    _state == STATE_CONNECTION_FAILURE ? "Connection failure: %s" :
                    _state == STATE_SECURITY_FAILURE ? "Security failure: %s" :
                    "%s",
                    fd.reasonString);
            }

            case STATE_SECURITY_RESULT:
            {
                Rfb::SecurityResult sr(_ibuf);
                TRACEPUT("SecurityResult: %u", sr.status);
                if (sr.status == Rfb::OK)
                {
                    InterlockedCompareExchange(&_state, STATE_CLIENTINIT, STATE_SECURITY_RESULT);
                    goto done;
                }
                else // if (result.status == FailedSecurityResult)
                {
                    _state = STATE_SECURITY_FAILURE;
                    break;
                }
            }

            case STATE_SERVERINIT:
            {
                Rfb::ServerInit si(_ibuf);
                _name.assign((char*)si.nameString, si.nameLength);
#if 0
                if (_name == "QEMU")
                {
                    _scanCodeEnabled = true;
                }
                else
#endif
                if (_name == "XenServer Virtual Terminal")
                {
                    _scanCodeEnabled = false;
                }
                TRACEPUT("ServerInit: width=%d", si.width);
                TRACEPUT("ServerInit: height=%d", si.height);
                TRACEPUT("ServerInit: bpp=%d", si.pixelFormat.bitsPerPixel);
                TRACEPUT("ServerInit: depth=%d", si.pixelFormat.depth);
                TRACEPUT("ServerInit: big-endian=%d", si.pixelFormat.bigEndian);
                TRACEPUT("ServerInit: true-colour=%d", si.pixelFormat.trueColour);
                TRACEPUT("ServerInit: r-max=%d", si.pixelFormat.rMax);
                TRACEPUT("ServerInit: g-max=%d", si.pixelFormat.gMax);
                TRACEPUT("ServerInit: b-max=%d", si.pixelFormat.bMax);
                TRACEPUT("ServerInit: r-shift=%d", si.pixelFormat.rShift);
                TRACEPUT("ServerInit: g-shift=%d", si.pixelFormat.gShift);
                TRACEPUT("ServerInit: b-shift=%d", si.pixelFormat.bShift);
                TRACEPUT("ServerInit: name=%s", si.nameString);
                if (si.pixelFormat.bitsPerPixel == 24 || si.pixelFormat.bitsPerPixel == 32)
                {
                    _pixelFormat.bitsPerPixel = si.pixelFormat.bitsPerPixel;
                }
                else if (_protocolVersion == 3003 && _name == "XenServer Virtual Terminal")
                {
                    // It is observed that vncterm working with 3.3 fails with 24 bpp
                    _pixelFormat.bitsPerPixel = 32;
                }
                else
                {
                    _pixelFormat.bitsPerPixel = _view.getDefaultBpp();
                }
                _width = si.width;
                _height = si.height;
                _view.init(_width, _height, _pixelFormat.bitsPerPixel);
                _incremental = 0;
                InterlockedCompareExchange(&_state, STATE_SETPIXELFORMAT, STATE_SERVERINIT);
                break;
            }

            case STATE_READY:
            {
                switch (_ibuf.peekU8(0))
                {
                case Rfb::FRAME_BUFFER_UPDATE:
                {
                    Rfb::FramebufferUpdate fu(_ibuf);
                    _numRects = fu.numberOfRectangles;
                    if (_numRects)
                    {
                        TRACEPUT("FramebufferUpdate: number-of-rectangles=%d", _numRects);
                        _rectIndex = 0;
                        _updateCount++;
                        InterlockedCompareExchange(&_state, STATE_FBUPDATE, STATE_READY);
                    }
                    break;
                }
                case Rfb::SET_COLOR_MAP_ENTRIES:
                {
                    Rfb::SetColourMapEntries cm(_ibuf);
                    TRACEPUT("SetColourMapEntries: first-color=0x%04x num-colours=%u", cm.firstColour, cm.numberOfColours);
                    break;
                }
                case Rfb::BELL:
                {
                    Rfb::Bell b(_ibuf);
                    TRACEPUT("Bell");
                    _view.bell();
                    break;
                }
                case Rfb::SERVER_CUT_TEXT:
                {
                    Rfb::ServerCutText ct(_ibuf);
                    TRACEPUT("ServerCutText: %s", ct.text);
                    break;
                }
                default:
                    _state = STATE_ERROR;
                    throw ConsoleException("Unsupported RFB message type: %d", _ibuf.peekU8(0));
                }
                break;
            }

            case STATE_FBUPDATE:
            {
                Rfb::Rectangle fu(_ibuf, _pixelFormat.bitsPerPixel);
                if (fu.encodingType == Rfb::RAW)
                {
                    TRACEPUT("FramebufferUpdate: %u %u %u %u", fu.x, fu.y, fu.width, fu.height);
                    _view.copy(fu.x, fu.y, fu.width, fu.height, _ibuf.rPtr());
                    _ibuf.rPos(_ibuf.rPos() + fu.dataSize(_pixelFormat.bitsPerPixel));
                }
                else if (fu.encodingType == Rfb::DESKTOP_SIZE_PSEUDO)
                {
                    TRACEPUT("Desktop size changed: %u %u", fu.width, fu.height);
                    _width = fu.width;
                    _height = fu.height;
                    _view.resize(_width, _height);
                    _incremental = 0;
                }
                else
                {
                    _state = STATE_UNSUPPORTED;
                    throw ConsoleException("FramebufferUpdate message including unsupported encoding type: %d", fu.encodingType);
                }
                if (++_rectIndex >= _numRects)
                {
                    InterlockedCompareExchange(&_state, STATE_FBUPDATEREQUEST, STATE_FBUPDATE);
                }
                break;
            }

            default:
                goto done;
            }
        }
    }
    catch (std::bad_alloc)
    {
        Logger::instance().error("Out of memory.");
        _state = STATE_ERROR;
    }
    catch (Rfb::NeedMoreDataException ex)
    {
        if (_ibuf.space() <= 0)
        {
            TRACEPUT("BUFFER FULL!");
            Glib::Mutex::Lock lock(_mutexRx);
            if (_ibuf.remaining() < _ibuf.capacity())
            {
                _ibuf.push();
            }
            else
            {
                _ibuf.capacity(_ibuf.capacity() * 2);
            }
            TRACEPUT("position=%zd length=%zd capacity=%zd", _ibuf.rPos(), _ibuf.remaining());
        }
    }
    catch (Rfb::ProtocolException ex)
    {
        throw ConsoleException("RFB Protocol failure: %s", ex.message);
        _state = STATE_ERROR;
    }

done:

    (void)0;
}


bool ConsoleImpl::processOutgoingData()
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::processOutgoingData", this));

    try
    {
        switch (_state)
        {
        case STATE_START_RESPONSE:
        {
            Rfb::ProtocolVersion pv(_protocolVersion);
            TRACEPUT("ProtocolVersion: response=%d.%d", _protocolVersion / 1000, _protocolVersion % 1000);
            {
                Glib::Mutex::Lock lock(_mutexTxBuf);
                pv.write(_obuf);
            }
            _condTx.signal();
            InterlockedCompareExchange(&_state, _protocolVersion >= 3007 ? STATE_SECURITY37 : STATE_SECURITY33, STATE_START_RESPONSE);
            break;
        }

        case STATE_SECURITY37_RESPONSE:
        {
            Rfb::Security37Response sr(Rfb::NONE);
            TRACEPUT("Security: response=%d", sr.securityType);
            {
                Glib::Mutex::Lock lock(_mutexTxBuf);
                sr.write(_obuf);
            }
            _condTx.signal();
            InterlockedCompareExchange(&_state, _protocolVersion >= 3008 ? STATE_SECURITY_RESULT : STATE_CLIENTINIT, STATE_SECURITY37_RESPONSE);
            break;
        }

        case STATE_CLIENTINIT:
        {
            Rfb::ClientInit ci(0);
            TRACEPUT("ClientInit: shared-flag=%d", ci.sharedFlag);
            {
                Glib::Mutex::Lock lock(_mutexTxBuf);
                ci.write(_obuf);
            }
            _condTx.signal();
            InterlockedCompareExchange(&_state, STATE_SERVERINIT, STATE_CLIENTINIT);
            break;
        }

        case STATE_SETPIXELFORMAT:
        {
            Rfb::SetPixelFormat pf(_pixelFormat);
            TRACEPUT("SetPixelFormat: messageType=%d", pf.messageType);
            TRACEPUT("SetPixelFormat: bpp=%d", pf.pixelFormat.bitsPerPixel);
            TRACEPUT("SetPixelFormat: depth=%d", pf.pixelFormat.depth);
            TRACEPUT("SetPixelFormat: big-endian=%d", pf.pixelFormat.bigEndian);
            TRACEPUT("SetPixelFormat: true-colour=%d", pf.pixelFormat.trueColour);
            TRACEPUT("SetPixelFormat: r-max=%d", pf.pixelFormat.rMax);
            TRACEPUT("SetPixelFormat: g-max=%d", pf.pixelFormat.gMax);
            TRACEPUT("SetPixelFormat: b-max=%d", pf.pixelFormat.bMax);
            TRACEPUT("SetPixelFormat: r-shift=%d", pf.pixelFormat.rShift);
            TRACEPUT("SetPixelFormat: g-shift=%d", pf.pixelFormat.gShift);
            TRACEPUT("SetPixelFormat: b-shift=%d", pf.pixelFormat.bShift);
            {
                Glib::Mutex::Lock lock(_mutexTxBuf);
                pf.write(_obuf);
            }
            _condTx.signal();
            InterlockedCompareExchange(&_state, STATE_SETENCODINGS, STATE_SETPIXELFORMAT);
            // FALLTHROUGH
        }

        case STATE_SETENCODINGS:
        {
            Rfb::SetEncodings2 se(Rfb::RAW, Rfb::DESKTOP_SIZE_PSEUDO);
            TRACEPUT("SetEncodings: messageType=%d", se.messageType);
            TRACEPUT("SetEncodings: number-of-encodings=%d", se.numerOfEncodings);
            TRACEPUT("SetEncodings: encoding-type[0]=%d", se.encodingTypes[0]);
            TRACEPUT("SetEncodings: encoding-type[1]=%d", se.encodingTypes[1]);
            {
                Glib::Mutex::Lock lock(_mutexTxBuf);
                se.write(_obuf);
            }
            _condTx.signal();
            InterlockedCompareExchange(&_state, STATE_FBUPDATEREQUEST, STATE_SETENCODINGS);
            // FALLTHROUGH
        }

        case STATE_FBUPDATEREQUEST:
        {
            Rfb::FramebufferUpdateRequest fu(_incremental, 0, 0, _width, _height);
            TRACEPUT("FramebufferUpdateRequest: messageType=%d", fu.messageType);
            TRACEPUT("FramebufferUpdateRequest: incremental=%d", fu.incremental);
            TRACEPUT("FramebufferUpdateRequest: x=%d", fu.x);
            TRACEPUT("FramebufferUpdateRequest: y=%d", fu.y);
            TRACEPUT("FramebufferUpdateRequest: cx=%d", fu.width);
            TRACEPUT("FramebufferUpdateRequest: cy=%d", fu.height);
            {
                Glib::Mutex::Lock lock(_mutexTxBuf);
                while (_obuf.remaining() > 0);
                fu.write(_obuf);
            }
            _condTx.signal();
            _incremental = 1;
            InterlockedCompareExchange(&_state, STATE_READY, STATE_FBUPDATEREQUEST);
            _readyCount = 0;
            break;
        }

        case STATE_READY: // in case of server not responding
        {
            _readyCount++;
            break;
        }

        default:
            break;
        }
    }
    catch (std::bad_alloc)
    {
        Logger::instance().error("Out of memory.");
        _state = STATE_ERROR;
    }
    catch (Rfb::NeedMoreSpaceException e)
    {
        TRACEPUT("NeedMoreSpaceException caught.");
        Glib::Mutex::Lock lock(_mutexTx);
        _obuf.push();
        return false;
    }

    return true;
}


void ConsoleImpl::sendPointerEvent(unsigned char buttonMask, unsigned short x, unsigned short y)
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::sendKeyEvent", this));

    if (STATE_INTERACTION_PHASE <= _state && _state < STATE_COMPLETED)
    {
        try
        {
            Rfb::PointerEvent pe(buttonMask, x, y);
            TRACEPUT("PointerEvent: %02X %u %u", pe.buttonMask, pe.x, pe.y);
            {
                Glib::Mutex::Lock lock(_mutexTxBuf);
                pe.write(_obuf);
            }
            _condTx.signal();
        }
        catch (std::bad_alloc)
        {
            _state = STATE_ERROR;
            Logger::instance().error("Out of memory.");
        }
        catch (...)
        {
            _state = STATE_ERROR;
            Logger::instance().error("Unexpected exception caught in ConsoleImpl::sendPointerEvent.");
        }
    }
}


static const guint keycodeToScancode[256] =
{
    0, // [0]
    0, // [1]
    0, // [2]
    0, // [3]
    0, // [4]
    0, // [5]
    0, // [6]
    0, // [7]
    0, // [8]
    0x01, // [9] ESC

    0x02, // [10] 1
    0x03, // [11] 2
    0x04, // [12] 3
    0x05, // [13] 4
    0x06, // [14] 5
    0x07, // [15] 6
    0x08, // [16] 7
    0x09, // [17] 8
    0x0A, // [18] 9
    0x0B, // [19] 0

    0x0C, // [20] -
    0x0D, // [21] =
    0x0E, // [22] Backspace
    0x0F, // [23] Tab
    0x10, // [24] q
    0x11, // [25] w
    0x12, // [26] e
    0x13, // [27] r
    0x14, // [28] t
    0x15, // [29] y

    0x16, // [30] u
    0x17, // [31] i
    0x18, // [32] o
    0x19, // [33] p
    0x1A, // [34] [
    0x1B, // [35] ]
    0x1C, // [36] Return
    0x1D, // [37] Control_L
    0x1E, // [38] a
    0x1F, // [39] s

    0x20, // [40] d
    0x21, // [41] f
    0x22, // [42] g
    0x23, // [43] h
    0x24, // [44] j
    0x25, // [45] k
    0x26, // [46] l
    0x27, // [47] ;
    0x28, // [48] '
    0x29, // [49] ` / JP=Zenkaku_Hankaku

    0x2A, // [50] Shift_L
    0x2B, // [51] US=\ JP=]
    0x2C, // [52] z
    0x2D, // [53] x
    0x2E, // [54] c
    0x2F, // [55] v
    0x30, // [56] b
    0x31, // [57] n
    0x32, // [58] m
    0x33, // [59] <

    0x34, // [60] >
    0x35, // [61] ?
    0x36, // [62] Shift_R
    0x37, // [63] KP_Multiply
    0x38, // [64] Alt_L
    0x39, // [65] Space
    0x3A, // [66] CapsLock / JP=Eisu_toggle
    0x3B, // [67] F1
    0x3C, // [68] F2
    0x3D, // [69] F3

    0x3E, // [70] F4
    0x3F, // [71] F5
    0x40, // [72] F6
    0x41, // [73] F7
    0x42, // [74] F8
    0x43, // [75] F9
    0x44, // [76] F10
    0x45, // [77] NumLock
    0x46, // [78] Scroll_Lock
    0x47, // [79] KP_7

    0x48, // [80] KP_8
    0x49, // [81] KP_9
    0x4A, // [82] KP_Subtract
    0x4B, // [83] KP_4
    0x4C, // [84] KP_5
    0x4D, // [85] KP_6
    0x4E, // [86] KP_Add
    0x4F, // [87] KP_1
    0x50, // [88] KP_2
    0x51, // [89] KP_3

    0x52, // [90] KP_0
    0x53, // [91] KP_Decimal
    0x54, // [92]
    0x55, // [93]
    0x56, // [94]
    0x57, // [95] F11
    0x58, // [96] F12
    0x73, // [97] JP=\_
    0, // [98]
    0, // [99]

    0x79, // [100] JP=Henkan_Mode
    0x70, // [101] JP=Hiragana_Katakana
    0x7B, // [102] JP=Muhenkan
    0, // [103]
    0x1C|0x80, // [104] KP_Enter
    0x1D|0x80, // [105] Control_R
    0x35|0x80, // [106] KP_Divide
    0, // [107]
    0x38|0x80, // [108] Alt_R
    0, // [109]

    0x47|0x80, // [110] Home
    0x48|0x80, // [111] Up
    0x49|0x80, // [112] Prior
    0x4B|0x80, // [113] Left
    0x4D|0x80, // [114] Right
    0x4F|0x80, // [115] End
    0x50|0x80, // [116] Down
    0x51|0x80, // [117] Next
    0x52|0x80, // [118] Insert
    0x53|0x80, // [119] Delete

    0, // [120]
    0, // [121]
    0, // [122]
    0, // [123]
    0, // [124]
    0, // [125]
    0, // [126]
    0, // [127]
    0, // [128]
    0, // [129]

    0, // [130]
    0, // [131]
    0x7D, // [132] JP=\|
    0x5B|0x80, // [133] Super_L
    0x5C|0x80, // [134] Super_R
    0x5D|0x80, // [135] Menu
    0, // [136]
    0, // [137]
    0, // [138]
    0, // [139]

    0, // [140]
    0, // [141]
    0, // [142]
    0, // [143]
    0, // [144]
    0, // [145]
    0, // [146]
    0, // [147]
    0, // [148]
    0, // [149]

    0, // [150]
    0, // [151]
    0, // [152]
    0, // [153]
    0, // [154]
    0, // [155]
    0, // [156]
    0, // [157]
    0, // [158]
    0, // [159]

    0, // [160]
    0, // [161]
    0, // [162]
    0, // [163]
    0, // [164]
    0, // [165]
    0, // [166]
    0, // [167]
    0, // [168]
    0, // [169]

    0, // [170]
    0, // [171]
    0, // [172]
    0, // [173]
    0, // [174]
    0, // [175]
    0, // [176]
    0, // [177]
    0, // [178]
    0, // [179]

    0, // [180]
    0, // [181]
    0, // [182]
    0, // [183]
    0, // [184]
    0, // [185]
    0, // [186]
    0, // [187]
    0, // [188]
    0, // [189]

    0, // [190]
    0, // [191]
    0, // [192]
    0, // [193]
    0, // [194]
    0, // [195]
    0, // [196]
    0, // [197]
    0, // [198]
    0, // [199]

    0, // [200]
    0, // [201]
    0, // [202]
    0, // [203]
    0, // [204]
    0, // [205]
    0, // [206]
    0, // [207]
    0, // [208]
    0, // [209]

    0, // [210]
    0, // [211]
    0, // [212]
    0, // [213]
    0, // [214]
    0, // [215]
    0, // [216]
    0, // [217]
    0, // [218]
    0, // [219]

    0, // [220]
    0, // [221]
    0, // [222]
    0, // [223]
    0, // [224]
    0, // [225]
    0, // [226]
    0, // [227]
    0, // [228]
    0, // [229]

    0, // [230]
    0, // [231]
    0, // [232]
    0, // [233]
    0, // [234]
    0, // [235]
    0, // [236]
    0, // [237]
    0, // [238]
    0, // [239]

    0, // [240]
    0, // [241]
    0, // [242]
    0, // [243]
    0, // [244]
    0, // [245]
    0, // [246]
    0, // [247]
    0, // [248]
    0, // [249]

    0, // [250]
    0, // [251]
    0, // [252]
    0, // [253]
    0, // [254]
    0, // [255]
};


void ConsoleImpl::sendKeyEvent(unsigned char downFlag, unsigned int keyval, unsigned int keycode)
{
    TRACE(StringBuffer().format("ConsoleImpl@%zx::sendKeyEvent", this));

    if (STATE_INTERACTION_PHASE <= _state && _state < STATE_COMPLETED)
    {
        try
        {
            if (_scanCodeEnabled)
            {
                guint scancode = keycodeToScancode[keycode & 0xFF];
                if (scancode)
                {
                    Rfb::ScanKeyEvent ke(downFlag, scancode);
                    TRACEPUT("ScanKeyEvent: %d %04X", ke.downFlag, ke.key);
                    {
                        Glib::Mutex::Lock lock(_mutexTxBuf);
                        ke.write(_obuf);
                    }
                    _condTx.signal();
                    return;
                }
            }
            if (keyval)
            {
                Rfb::KeyEvent ke(downFlag, keyval);
                TRACEPUT("KeyEvent: %d %04X", ke.downFlag, ke.key);
                {
                    Glib::Mutex::Lock lock(_mutexTxBuf);
                    ke.write(_obuf);
                }
                _condTx.signal();
            }
        }
        catch (std::bad_alloc)
        {
            _state = STATE_ERROR;
            Logger::instance().error("Out of memory.");
        }
        catch (...)
        {
            _state = STATE_ERROR;
            Logger::instance().error("Unexpected exception caught in ConsoleImpl::sendKeyEvent.");
        }
    }
}


void ConsoleImpl::sendCtrlAltDelete()
{
    sendKeyEvent(1, XK_Control_L, 37);
    sendKeyEvent(1, XK_Alt_L, 64);
    sendKeyEvent(1, XK_Delete, 119);
    sendKeyEvent(0, XK_Delete, 119);
    sendKeyEvent(0, XK_Alt_L, 64);
    sendKeyEvent(0, XK_Control_L, 37);
}
