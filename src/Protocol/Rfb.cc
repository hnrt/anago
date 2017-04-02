// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include "Rfb.h"


#define READ8(v,p) v=*p++
#define READ16(v,p) do{v=*p++;v<<=8;v|=*p++;}while(0)
#define READ32(v,p) do{v=*p++;v<<=8;v|=*p++;v<<=8;v|=*p++;v<<=8;v|=*p++;}while(0)
#define SKIP(v,p) p+=sizeof(v)

#define WRITE8(p,v) *p++=v
#define WRITE16(p,v) do{U16 _x=(U16)(v);p+=2;p[-1]=_x&0xFF;_x>>=8;p[-2]=_x&0xFF;}while(0)
#define WRITE32(p,v) do{U32 _x=(U32)(v);p+=4;p[-1]=_x&0xFF;_x>>=8;p[-2]=_x&0xFF;_x>>=8;p[-3]=_x&0xFF;_x>>=8;p[-4]=_x&0xFF;}while(0)
#define PAD(p,v) do{size_t _n=sizeof(v);memset(p,0,_n);p+=_n;}while(0)


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


void Rfb::ProtocolVersion::write(U8*& w, U8* s)
{
    if (w + sizeof(value) > s)
    {
        throw NotEnoughSpaceException(sizeof(value));
    }
    memcpy(w, value, sizeof(value));
    w += sizeof(value);
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


void Rfb::Security37Response::write(U8*& w, U8* s)
{
    if (w + sizeof(Security37Response) > s)
    {
        throw NotEnoughSpaceException(sizeof(Security37Response));
    }
    WRITE8(w, securityType);
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


void Rfb::ClientInit::write(U8*& w, U8* s)
{
    if (w + sizeof(ClientInit) > s)
    {
        throw NotEnoughSpaceException(sizeof(ClientInit));
    }
    WRITE8(w, sharedFlag);
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


Rfb::SetEncodings::SetEncodings(S32 encoding1, S32 encoding2)
    : messageType(SET_ENCODINGS)
    , padding(0)
    , numerOfEncodings(2)

{
    encodingTypes[0] = encoding1;
    encodingTypes[1] = encoding2;
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


Rfb::KeyEvent::KeyEvent(U8 downFlag_, U32 key_)
    : messageType(KEY_EVENT)
    , downFlag(downFlag_)
    , padding1(0)
    , padding2(0)
    , key(key_)
{
}


Rfb::ScanKeyEvent::ScanKeyEvent(U8 downFlag_, U32 key_)
    : messageType(SCAN_KEY_EVENT)
    , downFlag(downFlag_)
    , padding1(0)
    , padding2(0)
    , key(key_)
{
}


Rfb::PointerEvent::PointerEvent(U8 buttonMask_, U16 x_, U16 y_)
    : messageType(POINTER_EVENT)
    , buttonMask(buttonMask_)
    , x(x_)
    , y(y_)
{
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
