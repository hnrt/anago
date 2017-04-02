// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_REMOTE_FRAME_BUFFER_H
#define HNRT_REMOTE_FRAME_BUFFER_H


#include <stddef.h>


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

            PixelFormat();
            const U8* read(const U8* r);
            U8* write(U8* w);

        } __attribute__((__packed__));

        struct Rectangle
        {
            U16 x;
            U16 y;
            U16 width;
            U16 height;
            S32 encodingType;
            // followed by pixel data in the encoding given by encodingType

            Rectangle(const U8*& r, const U8* s);

        } __attribute__((__packed__));

        struct Colour
        {
            U16 red;
            U16 green;
            U16 blue;

            Colour(const U8*& r);

        } __attribute__((__packed__));

        struct CutText
        {
            U8 messageType; // 6
            U8 padding[3];
            U32 length;
            U8 text[]; // [length]

        } __attribute__((__packed__));

        struct CutTextPtr
        {
            CutText* ptr;

            CutTextPtr(const U8*& r, const U8* s);
            ~CutTextPtr();
            CutText* operator ->() { return ptr; }
        };

        //
        // Handshaking phase packets
        //

        struct ProtocolVersion
        {
            U8 value[12];

            ProtocolVersion(const U8*& r, const U8* s);
            ProtocolVersion(int version);
            int parse();
            void set(int version);
            void write(U8*& w, U8* s);

        } __attribute__((__packed__));

        struct Security37
        {
            U8 numberOfSecurityTypes;
            U8 securityTypes[]; // [numberOfSecurityTypes]

        } __attribute__((__packed__));

        struct Security37Ptr
        {
            Security37* ptr;

            Security37Ptr(const U8*& r, const U8* s);
            ~Security37Ptr();
            bool connectionFailure() const { return ptr ? false : true; }
            Security37* operator ->() { return ptr; }
        };

        struct Security37Response
        {
            U8 securityType;

            Security37Response(int value);
            void write(U8*& w, U8* s);

        } __attribute__((__packed__));

        struct SecurityResult
        {
            U32 status;

            SecurityResult(const U8*& r, const U8* s);

        } __attribute__((__packed__));

        struct FailureDescription
        {
            U32 reasonLength;
            U8 reasonString[]; // [reasonLength]

        } __attribute__((__packed__));

        struct FailureDescriptionPtr
        {
            FailureDescription* ptr;

            FailureDescriptionPtr(const U8*& r, const U8* s);
            ~FailureDescriptionPtr();
            FailureDescription* operator ->() { return ptr; }
        };

        struct Security33
        {
            U32 securityType;

            Security33(const U8*& r, const U8* s);

        } __attribute__((__packed__));

        //
        // Initialization phase packets
        //

        struct ClientInit
        {
            U8 sharedFlag;

            ClientInit(U8 value);
            void write(U8*& w, U8* s);

        } __attribute__((__packed__));

        struct ServerInit
        {
            U16 width;
            U16 height;
            PixelFormat pixelFormat;
            U32 nameLength;
            U8 nameString[]; // [nameLength]

        } __attribute__((__packed__));

        struct ServerInitPtr
        {
            ServerInit* ptr;

            ServerInitPtr(const U8*& r, const U8* s);
            ~ServerInitPtr();
            ServerInit* operator ->() { return ptr; }
        };

        //
        // Normal interaction phase packets
        //

        struct SetPixelFormat
        {
            U8 messageType; // 0
            U8 padding[3];
            PixelFormat pixelFormat;

            SetPixelFormat(const PixelFormat& value);
            void write(U8*& w, U8* s);

        } __attribute__((__packed__));

        struct SetEncodings
        {
            U8 messageType; // 2
            U8 padding;
            U16 numerOfEncodings;
            S32 encodingTypes[]; // [numerOfEncodings]

            SetEncodings(S32, S32);

        } __attribute__((__packed__));

        struct FramebufferUpdateRequest
        {
            U8 messageType; // 3
            U8 incremental;
            U16 x;
            U16 y;
            U16 width;
            U16 height;

            FramebufferUpdateRequest(U8 incremental_, U16 x_, U16 y_, U16 width_, U16 height_);
            void write(U8*& w, U8* s);

        } __attribute__((__packed__));

        struct KeyEvent
        {
            U8 messageType; // 4
            U8 downFlag;
            U8 padding[2];
            U32 key;

            KeyEvent(U8 downFlag_, U32 key_);
            U8* write(U8* w);

        } __attribute__((__packed__));


        struct ScanKeyEvent
        {
            U8 messageType; // 254
            U8 downFlag;
            U8 padding[2];
            U32 key;

            ScanKeyEvent(U8 downFlag_, U32 key_);
            U8* write(U8* w);

        } __attribute__((__packed__));


        struct PointerEvent
        {
            U8 messageType; // 5
            U8 buttonMask;
            U16 x;
            U16 y;

            PointerEvent(U8 buttonMask_, U16 x_, U16 y_);
            U8* write(U8* w);

        } __attribute__((__packed__));

        typedef CutText ClientCutText;
        typedef CutTextPtr ClientCutTextPtr;

        struct FramebufferUpdate
        {
            U8 messageType; // 0
            U8 padding;
            U16 numberOfRectangles;
            // followed by Rectangle rectangles[numberOfRectangles]

            FramebufferUpdate(const U8*& r, const U8* s);

        } __attribute__((__packed__));

        struct SetColourMapEntries
        {
            U8 messageType; // 1
            U8 padding;
            U16 firstColour;
            U16 numberOfColours;
            Colour colours[]; // [numberOfColors]

        } __attribute__((__packed__));

        struct SetColourMapEntriesPtr
        {
            SetColourMapEntries* ptr;

            SetColourMapEntriesPtr(const U8*& r, const U8* s);
            ~SetColourMapEntriesPtr();
            SetColourMapEntries* operator ->() { return ptr; }
        };

        struct Bell
        {
            U8 messageType; // 2

            Bell(const U8*& r);

        } __attribute__((__packed__));

        typedef CutText ServerCutText;
        typedef CutTextPtr ServerCutTextPtr;

        //
        // Exceptions
        //

        struct NeedMoreDataException
        {
            size_t size;

            NeedMoreDataException(size_t size_);
        };

        struct NotEnoughSpaceException
        {
            size_t size;

            NotEnoughSpaceException(size_t size_);
        };

        struct ProtocolException
        {
            const char* message;

            ProtocolException(const char* message_);
        };

    private:

        enum PrivateConstants
        {
            PRIVATE_REASON_MAX = 1023,
            PRIVATE_NAME_MAX = 1023,
            PRIVATE_TEXT_MAX = 65535,
        };
    };
}


#endif //!HNRT_REMOTE_FRAME_BUFFER_H
