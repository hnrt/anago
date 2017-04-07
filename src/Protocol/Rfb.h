// Copyright (C) 2012-2017 Hideaki Narita


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
            U8 padding1;
            U8 padding2;
            U8 padding3;

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

        } __attribute__((__packed__));

        struct CutText
        {
            U8 messageType; // 6
            U8 padding1;
            U8 padding2;
            U8 padding3;
            U32 length;
            U8* text; //[length]

            inline CutText(ByteBuffer&);
            inline ~CutText();

        } __attribute__((__packed__));

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

        } __attribute__((__packed__));

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

        } __attribute__((__packed__));

        struct FailureDescription
        {
            U32 reasonLength;
            U8* reasonString; // [reasonLength]

            inline FailureDescription(ByteBuffer&);
            inline ~FailureDescription();

        } __attribute__((__packed__));

        struct Security33
        {
            U32 securityType;

            inline Security33(ByteBuffer&);

        } __attribute__((__packed__));

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

        } __attribute__((__packed__));

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

        } __attribute__((__packed__));

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
    };

    inline Rfb::ProtocolVersion::ProtocolVersion(ByteBuffer& buf)
    {
        if (buf.remaining() < static_cast<int>(sizeof(value)))
        {
            throw NeedMoreDataException(sizeof(value));
        }
        buf.get(value, sizeof(value));
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
        if (buf.space() < static_cast<int64_t>(sizeof(value)))
        {
            throw NeedMoreSpaceException(sizeof(value));
        }
        buf.put(value, sizeof(value));
    }

    inline Rfb::Security37::Security37(ByteBuffer& buf)
        : securityTypes(NULL)
    {
        if (buf.remaining() < 1)
        {
            throw NeedMoreDataException(1);
        }
        numberOfSecurityTypes = buf.peekU8(0);
        int size = 1 + numberOfSecurityTypes;
        if (buf.remaining() < size)
        {
            throw NeedMoreDataException(size);
        }
        securityTypes = (U8*)malloc(sizeof(U8) * numberOfSecurityTypes);
        if (!securityTypes)
        {
            throw std::bad_alloc();
        }
        numberOfSecurityTypes = buf.getU8();
        buf.get(securityTypes, numberOfSecurityTypes);
    }

    inline Rfb::Security37::~Security37()
    {
        free(securityTypes);
    }

    inline Rfb::Security37Response::Security37Response(int value)
        : securityType((U8)value)
    {
    }

    inline void Rfb::Security37Response::write(ByteBuffer& buf)
    {
        if (buf.space() < static_cast<int64_t>(sizeof(securityType)))
        {
            throw NeedMoreSpaceException(sizeof(securityType));
        }
        buf.put(securityType);
    }

    inline Rfb::SecurityResult::SecurityResult(ByteBuffer& buf)
    {
        if (buf.remaining() < static_cast<int64_t>(sizeof(SecurityResult)))
        {
            throw NeedMoreDataException(sizeof(SecurityResult));
        }
        status = buf.getU32();
        if (status != OK && status != FAILED)
        {
            throw ProtocolException("Bad seurity result.");
        }
    }

    inline Rfb::FailureDescription::FailureDescription(ByteBuffer& buf)
        : reasonString(NULL)
    {
        const int sizeHeader = 4;
        if (buf.remaining() < sizeHeader)
        {
            throw NeedMoreDataException(sizeHeader);
        }
        reasonLength = buf.peekU32(0);
        int size = sizeHeader + reasonLength;
        if (buf.remaining() < size)
        {
            throw NeedMoreDataException(size);
        }
        reasonString = (U8*)malloc(sizeof(U8) * (reasonLength + 1));
        if (!reasonString)
        {
            throw std::bad_alloc();
        }
        reasonLength = buf.getU32();
        buf.get(reasonString, reasonLength);
        reasonString[reasonLength] = 0;
    }

    inline Rfb::FailureDescription::~FailureDescription()
    {
        free(reasonString);
    }

    inline Rfb::Security33::Security33(ByteBuffer& buf)
    {
        if (buf.remaining() < static_cast<int64_t>(sizeof(Security33)))
        {
            throw NeedMoreDataException(sizeof(Security33));
        }
        securityType = buf.getU32();
    }

    inline Rfb::ClientInit::ClientInit(U8 value)
        : sharedFlag(value)
    {
    }

    inline void Rfb::ClientInit::write(ByteBuffer& buf)
    {
        if (buf.space() < static_cast<int64_t>(sizeof(sharedFlag)))
        {
            throw NeedMoreSpaceException(sizeof(sharedFlag));
        }
        buf.put(sharedFlag);
    }

    inline Rfb::ServerInit::ServerInit(ByteBuffer& buf)
        : nameString(NULL)
    {
        const int offsetNameLength = 2 + 2 + static_cast<int>(sizeof(pixelFormat));
        const int sizeHeader = 2 + 2 + static_cast<int>(sizeof(pixelFormat)) + 4;
        if (buf.remaining() < sizeHeader)
        {
            throw NeedMoreDataException(sizeHeader);
        }
        nameLength = buf.peekU32(offsetNameLength);
        int size = sizeHeader + nameLength;
        if (buf.remaining() < size)
        {
            throw NeedMoreDataException(size);
        }
        nameString = (U8*)malloc(sizeof(U8) * (nameLength + 1));
        if (!nameString)
        {
            throw std::bad_alloc();
        }
        width = buf.getU16();
        height = buf.getU16();
        pixelFormat.read(buf);
        nameLength = buf.getU32();
        buf.get(nameString, nameLength);
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
        if (buf.space() < static_cast<int64_t>(sizeof(SetPixelFormat)))
        {
            throw NeedMoreSpaceException(sizeof(SetPixelFormat));
        }
        buf.put(messageType);
        buf.put(padding1);
        buf.put(padding2);
        buf.put(padding3);
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
        if (buf.space() < static_cast<int64_t>(sizeof(SetEncodings2)))
        {
            throw NeedMoreSpaceException(sizeof(SetEncodings2));
        }
        buf.put(messageType);
        buf.put(padding);
        buf.put(numerOfEncodings);
        buf.put(encodingTypes[0]);
        buf.put(encodingTypes[1]);
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
        if (buf.space() < static_cast<int64_t>(sizeof(FramebufferUpdateRequest)))
        {
            throw NeedMoreSpaceException(sizeof(FramebufferUpdateRequest));
        }
        buf.put(messageType);
        buf.put(incremental);
        buf.put(x);
        buf.put(y);
        buf.put(width);
        buf.put(height);
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
        if (buf.space() < static_cast<int64_t>(sizeof(KeyEvent)))
        {
            throw NeedMoreSpaceException(sizeof(KeyEvent));
        }
        buf.put(messageType);
        buf.put(downFlag);
        buf.put(padding1);
        buf.put(padding2);
        buf.put(key);
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
        if (buf.space() < static_cast<int64_t>(sizeof(ScanKeyEvent)))
        {
            throw NeedMoreSpaceException(sizeof(ScanKeyEvent));
        }
        buf.put(messageType);
        buf.put(downFlag);
        buf.put(padding1);
        buf.put(padding2);
        buf.put(key);
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
        if (buf.space() < static_cast<int64_t>(sizeof(PointerEvent)))
        {
            throw NeedMoreSpaceException(sizeof(PointerEvent));
        }
        buf.put(messageType);
        buf.put(buttonMask);
        buf.put(x);
        buf.put(y);
    }

    inline Rfb::FramebufferUpdate::FramebufferUpdate(ByteBuffer& buf)
    {
        if (buf.remaining() < static_cast<int64_t>(sizeof(FramebufferUpdate)))
        {
            throw NeedMoreDataException(sizeof(FramebufferUpdate));
        }
        messageType = buf.getU8();
        padding = buf.getU8();
        numberOfRectangles = buf.getU16();
    }

    inline Rfb::SetColourMapEntries::SetColourMapEntries(ByteBuffer& buf)
        : colours(NULL)
    {
        const int offsetNumberOfColours = 1 + 1 + 2;
        const int sizeHeader = 1 + 1 + 2 + 2;
        if (buf.remaining() < sizeHeader)
        {
            throw NeedMoreDataException(sizeHeader);
        }
        numberOfColours = buf.peekU16(offsetNumberOfColours);
        int size = sizeHeader + sizeof(Colour) * numberOfColours;
        if (buf.remaining() < size)
        {
            throw NeedMoreDataException(size);
        }
        if (numberOfColours)
        {
            colours = (Colour*)malloc(sizeof(Colour) * numberOfColours);
            if (!colours)
            {
                throw std::bad_alloc();
            }
        }
        messageType = buf.getU8();
        padding = buf.getU8();
        firstColour = buf.getU16();
        numberOfColours = buf.getU16();
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
        if (buf.remaining() < 1)
        {
            throw NeedMoreDataException(1);
        }
        messageType = buf.getU8();
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
        , padding1(0)
        , padding2(0)
        , padding3(0)
    {
    }

    inline void Rfb::PixelFormat::read(ByteBuffer& buf)
    {
        if (buf.remaining() < static_cast<int64_t>(sizeof(PixelFormat)))
        {
            throw NeedMoreDataException(sizeof(PixelFormat));
        }
        bitsPerPixel = buf.getU8();
        depth = buf.getU8();
        bigEndian = buf.getU8();
        trueColour = buf.getU8();
        rMax = buf.getU16();
        gMax = buf.getU16();
        bMax = buf.getU16();
        rShift = buf.getU8();
        gShift = buf.getU8();
        bShift = buf.getU8();
        padding1 = buf.getU8();
        padding2 = buf.getU8();
        padding3 = buf.getU8();
    }

    inline void Rfb::PixelFormat::write(ByteBuffer& buf)
    {
        buf.put(bitsPerPixel);
        buf.put(depth);
        buf.put(bigEndian);
        buf.put(trueColour);
        buf.put(rMax);
        buf.put(gMax);
        buf.put(bMax);
        buf.put(rShift);
        buf.put(gShift);
        buf.put(bShift);
        buf.put(padding1);
        buf.put(padding2);
        buf.put(padding3);
    }

    inline Rfb::Rectangle::Rectangle(ByteBuffer& buf, int bitsPerPixel)
    {
        const int offsetWidth = 2 + 2;
        const int offsetHeight = 2 + 2 + 2;
        const int offsetEncodingType = 2 + 2 + 2 + 2;
        const int sizeHeader = 2 + 2 + 2 + 2 + 4;
        if (buf.remaining() < sizeHeader)
        {
            throw NeedMoreDataException(sizeHeader);
        }
        encodingType = buf.peekU32(offsetEncodingType);
        if (encodingType == RAW)
        {
            width = buf.peekU16(offsetWidth);
            height = buf.peekU16(offsetHeight);
            int size = sizeHeader + dataSize(bitsPerPixel);
            if (buf.remaining() < size)
            {
                throw NeedMoreDataException(size);
            }
        }
        x = buf.getU16();
        y = buf.getU16();
        width = buf.getU16();
        height = buf.getU16();
        encodingType = buf.getU32();
    }

    inline Rfb::Colour::Colour(ByteBuffer& buf)
    {
        red = buf.getU16();
        green = buf.getU16();
        blue = buf.getU16();
    }

    inline Rfb::CutText::CutText(ByteBuffer& buf)
        : text(NULL)
    {
        const int offsetLength = 1 + 1 + 1 + 1;
        const int sizeHeader = 1 + 1 + 1 + 1 + 4;
        if (buf.remaining() < sizeHeader)
        {
            throw NeedMoreDataException(sizeHeader);
        }
        length = buf.peekU32(offsetLength);
        int size = sizeHeader + sizeof(U8) * length;
        if (buf.remaining() < size)
        {
            throw NeedMoreDataException(size);
        }
        text = (U8*)malloc(sizeof(U8) * (length + 1));
        if (!text)
        {
            throw std::bad_alloc();
        }
        messageType = buf.getU8();
        padding1 = buf.getU8();
        padding2 = buf.getU8();
        padding3 = buf.getU8();
        length = buf.getU32();
        buf.get(text, length);
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
