// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_STRINGBUFFER_H
#define HNRT_STRINGBUFFER_H


#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>


namespace hnrt
{
    class StringBuffer
    {
    public:

        StringBuffer(size_t = 0);
        StringBuffer(const StringBuffer&);
        ~StringBuffer();
        int size() const { return static_cast<int>(_capacity); }
        char* ptr() const { return _ptr; }
        int len() const { return static_cast<int>(_length); }
        const char* str() const { return _ptr ? _ptr : ""; }
        char& charAt(size_t) const;
        StringBuffer& resize(size_t);
        StringBuffer& setLength(size_t);
        StringBuffer& clear() { return setLength(0); }
        StringBuffer& assign(const StringBuffer&);
        StringBuffer& assign(char);
        StringBuffer& assign(const char*, ssize_t = -1);
        StringBuffer &format(const char*, ...);
        StringBuffer& formatV(const char*, va_list);
        StringBuffer& append(const StringBuffer&);
        StringBuffer& append(char);
        StringBuffer& append(const char*, ssize_t = -1);
        StringBuffer& appendFormat(const char*, ...);
        StringBuffer& appendFormatV(const char*, va_list);
        operator const char* () const { return _ptr ? _ptr : ""; }
        char& operator [](size_t index) const { return charAt(index); }
        StringBuffer& operator =(const StringBuffer& src) { return assign(src); }
        StringBuffer& operator =(char c) { return assign(c); }
        StringBuffer& operator =(const char* s) { return assign(s); }
        StringBuffer& operator +=(const StringBuffer& src) { return append(src); }
        StringBuffer& operator +=(char c) { return append(c); }
        StringBuffer& operator +=(const char* s) { return append(s); }

    protected:

        void expand(size_t);
        void free();

        char* _ptr;
        size_t _capacity;
        size_t _length;
    };
}


#endif //!HNRT_STRINGBUFFER_H
