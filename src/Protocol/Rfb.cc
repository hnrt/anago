// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include "Rfb.h"


#define READ8(v,p) v=*p++
#define READ16(v,p) do{v=*p++;v<<=8;v|=*p++;}while(0)
#define READ32(v,p) do{v=*p++;v<<=8;v|=*p++;v<<=8;v|=*p++;v<<=8;v|=*p++;}while(0)
#define SKIP(v,p) p+=sizeof(v)


using namespace hnrt;


Rfb::ProtocolVersion::ProtocolVersion(const U8*& r, const U8* s)
{
    if (r + sizeof(value) > s)
    {
        throw NeedMoreDataException(sizeof(value));
    }
    memcpy(value, r, sizeof(value));
    r += sizeof(value);
}


Rfb::ProtocolVersion::ProtocolVersion(int version)
{
    set(version);
}


int Rfb::ProtocolVersion::parse()
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


void Rfb::ProtocolVersion::set(int version)
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


void Rfb::ProtocolVersion::write(ByteBuffer& buf)
{
    buf.put(value, sizeof(value));
}


Rfb::Security37Ptr::Security37Ptr(const U8*& r, const U8* s)
    : ptr(NULL)
{
    U8 n = *r++;
    if (n == 0)
    {
        return;
    }
    if (r + n > s)
    {
        r--;
        throw NeedMoreDataException(sizeof(Security37) + n);
    }
    ptr = (Security37*)malloc(sizeof(Security37) + n);
    if (!ptr)
    {
        throw std::bad_alloc();
    }
    ptr->numberOfSecurityTypes = n;
    memcpy(ptr->securityTypes, r, n);
    r += n;
}


Rfb::Security37Ptr::~Security37Ptr()
{
    free(ptr);
}


Rfb::Security37Response::Security37Response(int value)
    : securityType((U8)value)
{
}


void Rfb::Security37Response::write(ByteBuffer& buf)
{
    buf.put(securityType);
}


Rfb::SecurityResult::SecurityResult(const U8*& r, const U8* s)
{
    if (r + sizeof(SecurityResult) > s)
    {
        throw NeedMoreDataException(sizeof(SecurityResult));
    }
    READ32(status, r);
    if (status != OK && status != FAILED)
    {
        throw ProtocolException("Bad seurity result.");
    }
}


Rfb::FailureDescriptionPtr::FailureDescriptionPtr(const U8*& r, const U8* s)
    : ptr(NULL)
{
    U32 n;
    if (r + sizeof(n) > s)
    {
        throw NeedMoreDataException(sizeof(n));
    }
    READ32(n, r);
    if (r + n > s)
    {
        r -= sizeof(n);
        if (n > PRIVATE_REASON_MAX)
        {
            throw ProtocolException("ReasonLength too big.");
        }
        else
        {
            throw NeedMoreDataException(sizeof(n) + n);
        }
    }
    ptr = (FailureDescription*)malloc(sizeof(FailureDescription) + n + 1);
    if (!ptr)
    {
        throw std::bad_alloc();
    }
    ptr->reasonLength = n;
    memcpy(ptr->reasonString, r, n);
    r += n;
    ptr->reasonString[n] = '\0';
}


Rfb::FailureDescriptionPtr::~FailureDescriptionPtr()
{
    free(ptr);
}


Rfb::Security33::Security33(const U8*& r, const U8* s)
{
    if (r + sizeof(Security33) > s)
    {
        throw NeedMoreDataException(sizeof(Security33));
    }
    READ32(securityType, r);
}


Rfb::ClientInit::ClientInit(U8 value)
    : sharedFlag(value)
{
}


void Rfb::ClientInit::write(ByteBuffer& buf)
{
    buf.put(sharedFlag);
}


Rfb::ServerInitPtr::ServerInitPtr(const U8*& r, const U8* s)
{
    if (r + sizeof(ServerInit) > s)
    {
        throw NeedMoreDataException(sizeof(ServerInit));
    }
    ServerInit t;
    READ16(t.width, r);
    READ16(t.height, r);
    r = t.pixelFormat.read(r);
    READ32(t.nameLength, r);
    if (r + t.nameLength > s)
    {
        if (t.nameLength > PRIVATE_NAME_MAX)
        {
            throw ProtocolException("NameLength too big.");
        }
        else
        {
            throw NeedMoreDataException(sizeof(ServerInit) + t.nameLength);
        }
    }
    ptr = (ServerInit*)malloc(sizeof(ServerInit) + t.nameLength + 1);
    if (!ptr)
    {
        throw std::bad_alloc();
    }
    *ptr = t;
    memcpy(ptr->nameString, r, ptr->nameLength);
    r += ptr->nameLength;
    ptr->nameString[ptr->nameLength] = '\0';
}


Rfb::ServerInitPtr::~ServerInitPtr()
{
    free(ptr);
}


Rfb::SetPixelFormat::SetPixelFormat(const PixelFormat& value)
    : messageType(SET_PIXEL_FORMAT)
    , padding1(0)
    , padding2(0)
    , padding3(0)
    , pixelFormat(value)
{
}


void Rfb::SetPixelFormat::write(ByteBuffer& buf)
{
    buf.put(messageType);
    buf.put(padding1);
    buf.put(padding2);
    buf.put(padding3);
    pixelFormat.write(buf);
}


Rfb::SetEncodings2::SetEncodings2(S32 encoding1, S32 encoding2)
    : messageType(SET_ENCODINGS)
    , padding(0)
    , numerOfEncodings(2)

{
    encodingTypes[0] = encoding1;
    encodingTypes[1] = encoding2;
}


void Rfb::SetEncodings2::write(ByteBuffer& buf)
{
    buf.put(messageType);
    buf.put(padding);
    buf.put(numerOfEncodings);
    buf.put(encodingTypes[0]);
    buf.put(encodingTypes[1]);
}


Rfb::FramebufferUpdateRequest::FramebufferUpdateRequest(U8 incremental_, U16 x_, U16 y_, U16 width_, U16 height_)
    : messageType(FRAME_BUFFER_UPDATE_REQUEST)
    , incremental(incremental_)
    , x(x_)
    , y(y_)
    , width(width_)
    , height(height_)
{
}


void Rfb::FramebufferUpdateRequest::write(ByteBuffer& buf)
{
    buf.put(messageType);
    buf.put(incremental);
    buf.put(x);
    buf.put(y);
    buf.put(width);
    buf.put(height);
}


Rfb::KeyEvent::KeyEvent(U8 downFlag_, U32 key_)
    : messageType(KEY_EVENT)
    , downFlag(downFlag_)
    , padding1(0)
    , padding2(0)
    , key(key_)
{
}


void Rfb::KeyEvent::write(ByteBuffer& buf)
{
    buf.put(messageType);
    buf.put(downFlag);
    buf.put(padding1);
    buf.put(padding2);
    buf.put(key);
}


Rfb::ScanKeyEvent::ScanKeyEvent(U8 downFlag_, U32 key_)
    : messageType(SCAN_KEY_EVENT)
    , downFlag(downFlag_)
    , padding1(0)
    , padding2(0)
    , key(key_)
{
}


void Rfb::ScanKeyEvent::write(ByteBuffer& buf)
{
    buf.put(messageType);
    buf.put(downFlag);
    buf.put(padding1);
    buf.put(padding2);
    buf.put(key);
}


Rfb::PointerEvent::PointerEvent(U8 buttonMask_, U16 x_, U16 y_)
    : messageType(POINTER_EVENT)
    , buttonMask(buttonMask_)
    , x(x_)
    , y(y_)
{
}


void Rfb::PointerEvent::write(ByteBuffer& buf)
{
    buf.put(messageType);
    buf.put(buttonMask);
    buf.put(x);
    buf.put(y);
}


Rfb::FramebufferUpdate::FramebufferUpdate(const U8*& r, const U8* s)
{
    if (r + sizeof(FramebufferUpdate) > s)
    {
        throw NeedMoreDataException(sizeof(FramebufferUpdate));
    }
    READ8(messageType, r);
    SKIP(padding, r);
    READ16(numberOfRectangles, r);
}


Rfb::SetColourMapEntriesPtr::SetColourMapEntriesPtr(const U8*& r, const U8* s)
    : ptr(NULL)
{
    if (r + sizeof(SetColourMapEntries) > s)
    {
        throw NeedMoreDataException(sizeof(SetColourMapEntries));
    }
    SetColourMapEntries t;
    READ8(t.messageType, r);
    SKIP(t.padding, r);
    READ16(t.firstColour, r);
    READ16(t.numberOfColours, r);
    if (r + t.numberOfColours * sizeof(Colour) > s)
    {
        r -= sizeof(SetColourMapEntries);
        throw NeedMoreDataException(sizeof(SetColourMapEntries));
    }
    ptr = (SetColourMapEntries*)malloc(sizeof(SetColourMapEntries) + t.numberOfColours * sizeof(Colour));
    if (!ptr)
    {
        throw std::bad_alloc();
    }
    *ptr = t;
    for (U16 i = 0; i < ptr->numberOfColours; i++)
    {
        new(ptr->colours + i) Colour(r);
    }
}


Rfb::SetColourMapEntriesPtr::~SetColourMapEntriesPtr()
{
    free(ptr);
}


Rfb::Bell::Bell(const U8*& r)
{
    READ8(messageType, r);
}


Rfb::PixelFormat::PixelFormat()
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


const Rfb::U8* Rfb::PixelFormat::read(const U8* r)
{
    READ8(bitsPerPixel, r);
    READ8(depth, r);
    READ8(bigEndian, r);
    READ8(trueColour, r);
    READ16(rMax, r);
    READ16(gMax, r);
    READ16(bMax, r);
    READ8(rShift, r);
    READ8(gShift, r);
    READ8(bShift, r);
    SKIP(padding1, r);
    SKIP(padding2, r);
    SKIP(padding3, r);
    return r;
}


void Rfb::PixelFormat::write(ByteBuffer& buf)
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


Rfb::Rectangle::Rectangle(const U8*& r, const U8* s)
{
    if (r + sizeof(Rectangle) > s)
    {
        throw NeedMoreDataException(sizeof(Rectangle));
    }
    READ16(x, r);
    READ16(y, r);
    READ16(width, r);
    READ16(height, r);
    READ32(encodingType, r);
}


Rfb::Colour::Colour(const U8*& r)
{
    READ16(red, r);
    READ16(green, r);
    READ16(blue, r);
}


Rfb::CutTextPtr::CutTextPtr(const U8*& r, const U8* s)
    : ptr(NULL)
{
    if (r + sizeof(CutText) > s)
    {
        throw NeedMoreDataException(sizeof(CutText));
    }
    CutText t;
    READ8(t.messageType, r);
    SKIP(t.padding, r);
    READ32(t.length, r);
    if (r + t.length > s)
    {
        r -= sizeof(CutText);
        if (t.length > PRIVATE_TEXT_MAX)
        {
            throw ProtocolException("TextLength too big.");
        }
        else
        {
            throw NeedMoreDataException(sizeof(CutText) + t.length);
        }
    }
    ptr = (CutText*)malloc(sizeof(CutText) + t.length + 1);
    if (!ptr)
    {
        throw std::bad_alloc();
    }
    *ptr = t;
    memcpy(ptr->text, r, ptr->length);
    r += ptr->length;
    ptr->text[ptr->length] = '\0';
}


Rfb::CutTextPtr::~CutTextPtr()
{
    free(ptr);
}


Rfb::NeedMoreDataException::NeedMoreDataException(size_t size_)
    : size(size_)
{
}


Rfb::NotEnoughSpaceException::NotEnoughSpaceException(size_t size_)
    : size(size_)
{
}


Rfb::ProtocolException::ProtocolException(const char* message_)
    : message(message_)
{
}
