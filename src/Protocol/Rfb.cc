// Copyright (C) 2012-2017 Hideaki Narita


#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include "Rfb.h"


using namespace hnrt;


Rfb::ProtocolVersion::ProtocolVersion(ByteBuffer& buf)
{
    if (buf.remaining() < static_cast<int>(sizeof(value)))
    {
        throw NeedMoreDataException(sizeof(value));
    }
    buf.get(value, sizeof(value));
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


Rfb::Security37::Security37(ByteBuffer& buf)
    : securityTypes(NULL)
{
    if (!buf.remaining())
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


Rfb::Security37::~Security37()
{
    free(securityTypes);
}


Rfb::Security37Response::Security37Response(int value)
    : securityType((U8)value)
{
}


void Rfb::Security37Response::write(ByteBuffer& buf)
{
    buf.put(securityType);
}


Rfb::SecurityResult::SecurityResult(ByteBuffer& buf)
{
    if (buf.remaining() < static_cast<int>(sizeof(SecurityResult)))
    {
        throw NeedMoreDataException(sizeof(SecurityResult));
    }
    status = buf.getU32();
    if (status != OK && status != FAILED)
    {
        throw ProtocolException("Bad seurity result.");
    }
}


Rfb::FailureDescription::FailureDescription(ByteBuffer& buf)
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


Rfb::FailureDescription::~FailureDescription()
{
    free(reasonString);
}


Rfb::Security33::Security33(ByteBuffer& buf)
{
    if (buf.remaining() < static_cast<int>(sizeof(Security33)))
    {
        throw NeedMoreDataException(sizeof(Security33));
    }
    securityType = buf.getU32();
}


Rfb::ClientInit::ClientInit(U8 value)
    : sharedFlag(value)
{
}


void Rfb::ClientInit::write(ByteBuffer& buf)
{
    buf.put(sharedFlag);
}


Rfb::ServerInit::ServerInit(ByteBuffer& buf)
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


Rfb::ServerInit::~ServerInit()
{
    free(nameString);
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


Rfb::FramebufferUpdate::FramebufferUpdate(ByteBuffer& buf)
{
    if (buf.remaining() < static_cast<int>(sizeof(FramebufferUpdate)))
    {
        throw NeedMoreDataException(sizeof(FramebufferUpdate));
    }
    messageType = buf.getU8();
    padding = buf.getU8();
    numberOfRectangles = buf.getU16();
}


Rfb::SetColourMapEntries::SetColourMapEntries(ByteBuffer& buf)
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


Rfb::SetColourMapEntries::~SetColourMapEntries()
{
    free(colours);
}


Rfb::Bell::Bell(ByteBuffer& buf)
{
    if (!buf.remaining())
    {
        throw NeedMoreDataException(1);
    }
    messageType = buf.getU8();
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


void Rfb::PixelFormat::read(ByteBuffer& buf)
{
    if (buf.remaining() < static_cast<int>(sizeof(PixelFormat)))
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


Rfb::Rectangle::Rectangle(ByteBuffer& buf, int bitsPerPixel)
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


Rfb::Colour::Colour(ByteBuffer& buf)
{
    if (buf.remaining() < static_cast<int>(sizeof(Colour)))
    {
        throw NeedMoreDataException(sizeof(Colour));
    }
    red = buf.getU16();
    green = buf.getU16();
    blue = buf.getU16();
}


Rfb::CutText::CutText(ByteBuffer& buf)
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


Rfb::CutText::~CutText()
{
    free(text);
}


Rfb::NeedMoreDataException::NeedMoreDataException(size_t size_)
    : size(size_)
{
}


Rfb::ProtocolException::ProtocolException(const char* message_)
    : message(message_)
{
}
