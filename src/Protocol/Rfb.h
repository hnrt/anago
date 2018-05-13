// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_REMOTE_FRAME_BUFFER_H
#define HNRT_REMOTE_FRAME_BUFFER_H


#include <stddef.h>
#include <stdlib.h>
#include "Base/ByteBuffer.h"


namespace hnrt
{
    class Rfb
    {
    public:

        enum SecurityType
        {
            INVALID = 0,
            NONE = 1,
            VNC_AUTHENTICATION = 2,
        };

        enum SecurityResultType
        {
            OK = 0,
            FAILED = 1,
        };

        enum MessageType
        {
            // Client to Server
            SET_PIXEL_FORMAT = 0,
            SET_ENCODINGS = 2,
            FRAME_BUFFER_UPDATE_REQUEST = 3,
            KEY_EVENT = 4,
            POINTER_EVENT = 5,
            CLIENT_CUT_TEXT = 6,
            SCAN_KEY_EVENT = 254,

            // Server to Client
            FRAME_BUFFER_UPDATE = 0,
            SET_COLOR_MAP_ENTRIES = 1,
            BELL = 2,
            SERVER_CUT_TEXT = 3,
        };

        enum EncodingType
        {
            RAW = 0,
            COPY_RECT = 1,
            RRE = 2,
            HEXTILE = 5,
            ZRLE = 6,
            CURSOR_PSEUDO = -239,
            DESKTOP_SIZE_PSEUDO = -223,
        };

        typedef unsigned char U8;
        typedef unsigned short U16;
        typedef unsigned int U32;
        typedef signed int S32;

        //
        // Utility structures
        //

        struct PixelFormat
        {
            U8 bitsPerPixel;
            U8 depth;
            U8 bigEndian;
            U8 trueColour;
            U16 rMax;
            U16 gMax;
            U16 bMax;
            U8 rShift;
            U8 gShift;
            U8 bShift;
            U8 padding[3];

            inline PixelFormat();
            inline void read(ByteBuffer&);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));

        struct Rectangle
        {
            U16 x;
            U16 y;
            U16 width;
            U16 height;
            S32 encodingType;
            // followed by pixel data in the encoding given by encodingType

            inline Rectangle(ByteBuffer&, int bitsPerPixel);
            inline int dataSize(int bitsPerPixel) const { return width * height * bitsPerPixel / 8; }

        } __attribute__((__packed__));

        struct Colour
        {
            U16 red;
            U16 green;
            U16 blue;

            inline Colour(ByteBuffer&);
        }  __attribute__((__packed__));

        struct CutText
        {
            U8 messageType; // 6
            U8 padding[3];
            U32 length;
            U8* text; //[length]

            inline CutText(ByteBuffer&);
            inline ~CutText();
        };

        //
        // Handshaking phase packets
        //

        struct ProtocolVersion
        {
            U8 value[12];

            inline ProtocolVersion(ByteBuffer&);
            inline ProtocolVersion(int version);
            inline int parse();
            inline void set(int version);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));

        struct Security37
        {
            U8 numberOfSecurityTypes;
            U8* securityTypes; // [numberOfSecurityTypes]

            inline Security37(ByteBuffer&);
            inline ~Security37();
        };

        struct Security37Response
        {
            U8 securityType;

            inline Security37Response(int value);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));

        struct SecurityResult
        {
            U32 status;

            inline SecurityResult(ByteBuffer&);
        };

        struct FailureDescription
        {
            U32 reasonLength;
            U8* reasonString; // [reasonLength]

            inline FailureDescription(ByteBuffer&);
            inline ~FailureDescription();
        };

        struct Security33
        {
            U32 securityType;

            inline Security33(ByteBuffer&);
        };

        //
        // Initialization phase packets
        //

        struct ClientInit
        {
            U8 sharedFlag;

            inline ClientInit(U8 value);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));

        struct ServerInit
        {
            U16 width;
            U16 height;
            PixelFormat pixelFormat;
            U32 nameLength;
            U8* nameString; //[nameLength]

            inline ServerInit(ByteBuffer&);
            inline ~ServerInit();
        };

        //
        // Normal interaction phase packets
        //

        struct SetPixelFormat
        {
            U8 messageType; // 0
            U8 padding1;
            U8 padding2;
            U8 padding3;
            PixelFormat pixelFormat;

            inline SetPixelFormat(const PixelFormat& value);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));

        struct SetEncodings2
        {
            U8 messageType; // 2
            U8 padding;
            U16 numerOfEncodings; // 2
            S32 encodingTypes[2];

            inline SetEncodings2(S32, S32);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));

        struct FramebufferUpdateRequest
        {
            U8 messageType; // 3
            U8 incremental;
            U16 x;
            U16 y;
            U16 width;
            U16 height;

            inline FramebufferUpdateRequest(U8 incremental_, U16 x_, U16 y_, U16 width_, U16 height_);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));

        struct KeyEvent
        {
            U8 messageType; // 4
            U8 downFlag;
            U8 padding1;
            U8 padding2;
            U32 key;

            inline KeyEvent(U8 downFlag_, U32 key_);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));


        struct ScanKeyEvent
        {
            U8 messageType; // 254
            U8 downFlag;
            U8 padding1;
            U8 padding2;
            U32 key;

            inline ScanKeyEvent(U8 downFlag_, U32 key_);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));


        struct PointerEvent
        {
            U8 messageType; // 5
            U8 buttonMask;
            U16 x;
            U16 y;

            inline PointerEvent(U8 buttonMask_, U16 x_, U16 y_);
            inline void write(ByteBuffer&);

        } __attribute__((__packed__));

        typedef CutText ClientCutText;

        struct FramebufferUpdate
        {
            U8 messageType; // 0
            U8 padding;
            U16 numberOfRectangles;
            // followed by Rectangle rectangles[numberOfRectangles]

            inline FramebufferUpdate(ByteBuffer&);

        } __attribute__((__packed__));

        struct SetColourMapEntries
        {
            U8 messageType; // 1
            U8 padding;
            U16 firstColour;
            U16 numberOfColours;
            Colour* colours; // [numberOfColors]

            inline SetColourMapEntries(ByteBuffer&);
            inline ~SetColourMapEntries();
        };

        struct Bell
        {
            U8 messageType; // 2

            inline Bell(ByteBuffer&);

        } __attribute__((__packed__));

        typedef CutText ServerCutText;

        //
        // Exceptions
        //

        struct NeedMoreDataException
        {
            size_t size;

            inline NeedMoreDataException(size_t size_);
        };

        struct NeedMoreSpaceException
        {
            size_t size;

            inline NeedMoreSpaceException(size_t size_);
        };

        struct ProtocolException
        {
            const char* message;

            inline ProtocolException(const char* message_);
        };

    protected:

        static inline void checkReadBySize(ByteBuffer&, size_t);
        static inline void checkWriteBySize(ByteBuffer&, size_t);

        template<typename T> static inline void checkRead(ByteBuffer&, T&);
        template<typename T> static inline void checkWrite(ByteBuffer&, T&);
        template<typename T> static inline void allocate(T*&, size_t);
    };

    inline void Rfb::checkReadBySize(ByteBuffer& buf, size_t required)
    {
        if (buf.rLen() < static_cast<int64_t>(required))
        {
            throw NeedMoreDataException(required);
        }
    }

    inline void Rfb::checkWriteBySize(ByteBuffer& buf, size_t required)
    {
        if (buf.wLen() < static_cast<int64_t>(required))
        {
            throw NeedMoreSpaceException(required);
        }
    }

    template<typename T> inline void Rfb::checkRead(ByteBuffer& buf, T&)
    {
        checkReadBySize(buf, sizeof(T));
    }

    template<typename T> inline void Rfb::checkWrite(ByteBuffer& buf, T&)
    {
        checkWriteBySize(buf, sizeof(T));
    }

    template<typename T> inline void Rfb::allocate(T*& ptr, size_t count)
    {
        ptr = (T*)malloc(sizeof(T) * count);
        if (!ptr)
        {
            throw std::bad_alloc();
        }
    }

    inline Rfb::ProtocolVersion::ProtocolVersion(ByteBuffer& buf)
    {
        checkRead(buf, value);
        buf.read(value, sizeof(value));
    }

    inline Rfb::ProtocolVersion::ProtocolVersion(int version)
    {
        set(version);
    }

    inline int Rfb::ProtocolVersion::parse()
    {
        if (value[0] == 'R' &&
            value[1] == 'F' &&
            value[2] == 'B' &&
            value[3] == ' ' &&
            value[4] - '0' <= '9' - '0' &&
            value[5] - '0' <= '9' - '0' &&
            value[6] - '0' <= '9' - '0' &&
            value[7] == '.' &&
            value[8] - '0' <= '9' - '0' &&
            value[9] - '0' <= '9' - '0' &&
            value[10] - '0' <= '9' - '0' &&
            value[11] == '\n')
        {
            int version
                = (value[4] - '0') * 100000
                + (value[5] - '0') * 10000
                + (value[6] - '0') * 1000
                + (value[8] - '0') * 100
                + (value[9] - '0') * 10
                + (value[10] - '0') * 1;
            if (version >= 3003)
            {
                return version;
            }
            else
            {
                throw ProtocolException("Bad protocol version (less than 3.3).");
            }
        }
        else
        {
            throw ProtocolException("Malformed protocol version.");
        }
    }

    inline void Rfb::ProtocolVersion::set(int version)
    {
        value[0] = 'R';
        value[1] = 'F';
        value[2] = 'B';
        value[3] = ' ';
        value[7] = '.';
        value[11] = '\n';
        value[10] = version % 10 + '0';
        version /= 10;
        value[9] = version % 10 + '0';
        version /= 10;
        value[8] = version % 10 + '0';
        version /= 10;
        value[6] = version % 10 + '0';
        version /= 10;
        value[5] = version % 10 + '0';
        version /= 10;
        value[4] = version % 10 + '0';
    }

    inline void Rfb::ProtocolVersion::write(ByteBuffer& buf)
    {
        checkWrite(buf, value);
        buf.write(value, sizeof(value));
    }

    inline Rfb::Security37::Security37(ByteBuffer& buf)
        : securityTypes(0)
    {
        checkRead(buf, numberOfSecurityTypes);
        buf.peek(numberOfSecurityTypes, 0);
        checkReadBySize(buf, sizeof(numberOfSecurityTypes) + numberOfSecurityTypes);
        allocate(securityTypes, numberOfSecurityTypes);
        buf.read(numberOfSecurityTypes);
        buf.read(securityTypes, numberOfSecurityTypes);
    }

    inline Rfb::Security37::~Security37()
    {
        free(securityTypes);
    }

    inline Rfb::Security37Response::Security37Response(int value)
        : securityType(static_cast<U8>(value))
    {
    }

    inline void Rfb::Security37Response::write(ByteBuffer& buf)
    {
        checkWrite(buf, securityType);
        buf.write(securityType);
    }

    inline Rfb::SecurityResult::SecurityResult(ByteBuffer& buf)
    {
        checkReadBySize(buf, sizeof(status));
        buf.read(status);
        if (status != OK && status != FAILED)
        {
            throw ProtocolException("Bad seurity result.");
        }
    }

    inline Rfb::FailureDescription::FailureDescription(ByteBuffer& buf)
        : reasonString(NULL)
    {
        checkRead(buf, reasonLength);
        buf.peek(reasonLength, 0);
        checkReadBySize(buf, sizeof(reasonLength) + reasonLength);
        allocate(reasonString, reasonLength + 1);
        buf.read(reasonLength);
        buf.read(reasonString, reasonLength);
        reasonString[reasonLength] = 0;
    }

    inline Rfb::FailureDescription::~FailureDescription()
    {
        free(reasonString);
    }

    inline Rfb::Security33::Security33(ByteBuffer& buf)
    {
        checkRead(buf, securityType);
        buf.read(securityType);
    }

    inline Rfb::ClientInit::ClientInit(U8 value)
        : sharedFlag(value)
    {
    }

    inline void Rfb::ClientInit::write(ByteBuffer& buf)
    {
        checkWrite(buf, sharedFlag);
        buf.write(sharedFlag);
    }

    inline Rfb::ServerInit::ServerInit(ByteBuffer& buf)
        : nameString(0)
    {
        const size_t offsetNameLength = sizeof(width) + sizeof(height) + sizeof(pixelFormat);
        const size_t sizeHeader = offsetNameLength + sizeof(nameLength);
        checkReadBySize(buf, sizeHeader);
        buf.peek(nameLength, offsetNameLength);
        checkReadBySize(buf, sizeHeader + nameLength);
        allocate(nameString, nameLength + 1);
        buf.read(width);
        buf.read(height);
        pixelFormat.read(buf);
        buf.read(nameLength);
        buf.read(nameString, nameLength);
        nameString[nameLength] = 0;
    }

    inline Rfb::ServerInit::~ServerInit()
    {
        free(nameString);
    }

    inline Rfb::SetPixelFormat::SetPixelFormat(const PixelFormat& value)
        : messageType(SET_PIXEL_FORMAT)
        , padding1(0)
        , padding2(0)
        , padding3(0)
        , pixelFormat(value)
    {
    }

    inline void Rfb::SetPixelFormat::write(ByteBuffer& buf)
    {
        checkWrite(buf, *this);
        buf.write(messageType);
        buf.write(padding1);
        buf.write(padding2);
        buf.write(padding3);
        pixelFormat.write(buf);
    }

    inline Rfb::SetEncodings2::SetEncodings2(S32 encoding1, S32 encoding2)
        : messageType(SET_ENCODINGS)
        , padding(0)
        , numerOfEncodings(2)

    {
        encodingTypes[0] = encoding1;
        encodingTypes[1] = encoding2;
    }

    inline void Rfb::SetEncodings2::write(ByteBuffer& buf)
    {
        checkWrite(buf, *this);
        buf.write(messageType);
        buf.write(padding);
        buf.write(numerOfEncodings);
        buf.write(encodingTypes[0]);
        buf.write(encodingTypes[1]);
    }

    inline Rfb::FramebufferUpdateRequest::FramebufferUpdateRequest(U8 incremental_, U16 x_, U16 y_, U16 width_, U16 height_)
        : messageType(FRAME_BUFFER_UPDATE_REQUEST)
        , incremental(incremental_)
        , x(x_)
        , y(y_)
        , width(width_)
        , height(height_)
    {
    }

    inline void Rfb::FramebufferUpdateRequest::write(ByteBuffer& buf)
    {
        checkWrite(buf, *this);
        buf.write(messageType);
        buf.write(incremental);
        buf.write(x);
        buf.write(y);
        buf.write(width);
        buf.write(height);
    }

    inline Rfb::KeyEvent::KeyEvent(U8 downFlag_, U32 key_)
        : messageType(KEY_EVENT)
        , downFlag(downFlag_)
        , padding1(0)
        , padding2(0)
        , key(key_)
    {
    }

    inline void Rfb::KeyEvent::write(ByteBuffer& buf)
    {
        checkWrite(buf, *this);
        buf.write(messageType);
        buf.write(downFlag);
        buf.write(padding1);
        buf.write(padding2);
        buf.write(key);
    }

    inline Rfb::ScanKeyEvent::ScanKeyEvent(U8 downFlag_, U32 key_)
        : messageType(SCAN_KEY_EVENT)
        , downFlag(downFlag_)
        , padding1(0)
        , padding2(0)
        , key(key_)
    {
    }

    inline void Rfb::ScanKeyEvent::write(ByteBuffer& buf)
    {
        checkWrite(buf, *this);
        buf.write(messageType);
        buf.write(downFlag);
        buf.write(padding1);
        buf.write(padding2);
        buf.write(key);
    }

    inline Rfb::PointerEvent::PointerEvent(U8 buttonMask_, U16 x_, U16 y_)
        : messageType(POINTER_EVENT)
        , buttonMask(buttonMask_)
        , x(x_)
        , y(y_)
    {
    }

    inline void Rfb::PointerEvent::write(ByteBuffer& buf)
    {
        checkWrite(buf, *this);
        buf.write(messageType);
        buf.write(buttonMask);
        buf.write(x);
        buf.write(y);
    }

    inline Rfb::FramebufferUpdate::FramebufferUpdate(ByteBuffer& buf)
    {
        checkRead(buf, *this);
        messageType = buf.readU8();
        padding = buf.readU8();
        numberOfRectangles = buf.readU16();
    }

    inline Rfb::SetColourMapEntries::SetColourMapEntries(ByteBuffer& buf)
        : colours(NULL)
    {
        const size_t offsetNumberOfColours = sizeof(messageType) + sizeof(padding) + sizeof(firstColour);
        const size_t sizeHeader = offsetNumberOfColours + sizeof(numberOfColours);
        checkReadBySize(buf, sizeHeader);
        buf.peek(numberOfColours, offsetNumberOfColours);
        checkReadBySize(buf, sizeHeader + sizeof(Colour) * numberOfColours);
        if (numberOfColours)
        {
            allocate(colours, numberOfColours);
        }
        buf.read(messageType);
        buf.read(padding);
        buf.read(firstColour);
        buf.read(numberOfColours);
        for (U16 i = 0; i < numberOfColours; i++)
        {
            new(colours + i) Colour(buf);
        }
    }

    inline Rfb::SetColourMapEntries::~SetColourMapEntries()
    {
        free(colours);
    }

    inline Rfb::Bell::Bell(ByteBuffer& buf)
    {
        checkRead(buf, messageType);
        buf.read(messageType);
    }

    inline Rfb::PixelFormat::PixelFormat()
        : bitsPerPixel(0) // to be determined at SERVERINIT time
        , depth(24)
        , bigEndian(0)
        , trueColour(1)
        , rMax(0xFF)
        , gMax(0xFF)
        , bMax(0xFF)
        , rShift(0)
        , gShift(8)
        , bShift(16)
    {
        memset(padding, 0, sizeof(padding));
    }

    inline void Rfb::PixelFormat::read(ByteBuffer& buf)
    {
        bitsPerPixel = buf.readU8();
        depth = buf.readU8();
        bigEndian = buf.readU8();
        trueColour = buf.readU8();
        rMax = buf.readU16();
        gMax = buf.readU16();
        bMax = buf.readU16();
        rShift = buf.readU8();
        gShift = buf.readU8();
        bShift = buf.readU8();
        buf.read(padding, sizeof(padding));
    }

    inline void Rfb::PixelFormat::write(ByteBuffer& buf)
    {
        buf.write(bitsPerPixel);
        buf.write(depth);
        buf.write(bigEndian);
        buf.write(trueColour);
        buf.write(rMax);
        buf.write(gMax);
        buf.write(bMax);
        buf.write(rShift);
        buf.write(gShift);
        buf.write(bShift);
        buf.write(padding, sizeof(padding));
    }

    inline Rfb::Rectangle::Rectangle(ByteBuffer& buf, int bitsPerPixel)
    {
        checkRead(buf, *this);
        const size_t offsetWidth = sizeof(x) + sizeof(y);
        const size_t offsetHeight = offsetWidth + sizeof(width);
        encodingType = buf.peekS32(offsetHeight + sizeof(height));
        if (encodingType == RAW)
        {
            width = buf.peekU16(offsetWidth);
            height = buf.peekU16(offsetHeight);
            checkReadBySize(buf, sizeof(*this) + dataSize(bitsPerPixel));
        }
        x = buf.readU16();
        y = buf.readU16();
        width = buf.readU16();
        height = buf.readU16();
        encodingType = buf.readS32();
    }

    inline Rfb::Colour::Colour(ByteBuffer& buf)
    {
        red = buf.readU16();
        green = buf.readU16();
        blue = buf.readU16();
    }

    inline Rfb::CutText::CutText(ByteBuffer& buf)
        : text(NULL)
    {
        const size_t offsetLength = sizeof(messageType) + sizeof(padding);
        const int sizeHeader = offsetLength + sizeof(length);
        checkReadBySize(buf, sizeHeader);
        buf.peek(length, offsetLength);
        checkReadBySize(buf, sizeHeader + length);
        allocate(text, length + 1);
        buf.read(messageType);
        buf.read(padding, sizeof(padding));
        buf.read(length);
        buf.read(text, length);
        text[length] = 0;
    }

    inline Rfb::CutText::~CutText()
    {
        free(text);
    }

    inline Rfb::NeedMoreDataException::NeedMoreDataException(size_t size_)
        : size(size_)
    {
    }

    inline Rfb::NeedMoreSpaceException::NeedMoreSpaceException(size_t size_)
        : size(size_)
    {
    }

    inline Rfb::ProtocolException::ProtocolException(const char* message_)
        : message(message_)
    {
    }
}


#endif //!HNRT_REMOTE_FRAME_BUFFER_H
