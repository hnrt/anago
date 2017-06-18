// Copyright (C) 2017 Hideaki Narita


#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdexcept>
#include "Time.h"


using namespace hnrt;


Time::~Time()
{
    free(_data);
}


Time& Time::now()
{
    struct timeval tv = { 0 };
    gettimeofday(&tv, NULL);
    _seconds = tv.tv_sec;
    _microseconds = tv.tv_usec;
    _data = copy(_data, 0);
    return *this;
}


const void* Time::data() const
{
    if (!_data)
    {
        void* ptr = calloc(1, sizeof(struct tm));
        if (!ptr)
        {
            throw std::bad_alloc();
        }
        time_t t = static_cast<time_t>(_seconds);
        localtime_r(&t, reinterpret_cast<struct tm*>(ptr));
        const_cast<Time*>(this)->_data = ptr;
    }
    return _data;
}


void* Time::copy(void* dst, const void* src)
{
    if (!src)
    {
        if (dst)
        {
            free(dst);
        }
        return 0;
    }
    if (!dst)
    {
        dst = malloc(sizeof(struct tm));
        if (!dst)
        {
            throw std::bad_alloc();
        }
    }
    return memcpy(dst, src, sizeof(struct tm));
}


inline static bool IsLeapYear(int year)
{
    return ((year % 400) == 0) || (((year % 4) == 0) && ((year % 100) != 0));
}


inline static int EndOfMonth(int year, int month)
{
    static const int d[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int eom = d[month - 1];
    if (month == 2 && IsLeapYear(year))
    {
        eom++;
    }
    return eom;
}


Time& Time::addMonths(long value)
{
    int y = year();
    int m = month();
    int ym = y * 12 + m - 1;
    ym += static_cast<int>(value);
    if (ym < (1900 * 12))
    {
        throw std::invalid_argument("Time::addMonths");
    }
    y = ym / 12;
    m = ym % 12 + 1;
    int d = dayOfMonth();
    int e = EndOfMonth(y, m);
    if (d > e)
    {
        d = e;
    }
    struct tm* ptr = reinterpret_cast<struct tm*>(_data);
    ptr->tm_year = y - 1900;
    ptr->tm_mon = m - 1;
    ptr->tm_mday = d;
    ptr->tm_isdst = -1; // let mktime decide daylight saving time
    time_t t = mktime(ptr);
    _seconds = static_cast<long>(t);
    return *this;
}


int Time::year() const
{
    return reinterpret_cast<const struct tm*>(data())->tm_year + 1900;
}


int Time::month() const
{
    return reinterpret_cast<const struct tm*>(data())->tm_mon + 1;
}


int Time::dayOfMonth() const
{
    return reinterpret_cast<const struct tm*>(data())->tm_mday;
}


// Sun=0 Mon=1 Tue=2 Wed=3 Thu=4 Fri=5 Sat=6
int Time::dayOfWeek() const
{
    return reinterpret_cast<const struct tm*>(data())->tm_wday;
}


int Time::hour() const
{
    return reinterpret_cast<const struct tm*>(data())->tm_hour;
}


int Time::minute() const
{
    return reinterpret_cast<const struct tm*>(data())->tm_min;
}


int Time::second() const
{
    return reinterpret_cast<const struct tm*>(data())->tm_sec;
}
