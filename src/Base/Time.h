// Copyright (C) 2017-2018 Hideaki Narita


#ifndef HNRT_TIME_H
#define HNRT_TIME_H


namespace hnrt
{
    class Time
    {
    public:

        ~Time();
        inline Time();
        inline Time(const Time&);
        inline Time& operator =(const Time&);
        Time& now();
        inline bool operator ==(const Time&) const;
        inline bool operator <(const Time&) const;
        inline bool operator <=(const Time&) const;
        inline bool operator >(const Time&) const;
        inline bool operator >=(const Time&) const;
        inline Time& addMicroseconds(long);
        inline Time& addMilliseconds(long);
        inline Time& addSeconds(long);
        inline Time& addMinutes(long);
        inline Time& addHours(long);
        inline Time& addDays(long);
        Time& addMonths(long);
        inline Time& minusMicroseconds(long);
        inline Time& minusMilliseconds(long);
        inline Time& minusSeconds(long);
        inline Time& minusMinutes(long);
        inline Time& minusHours(long);
        inline Time& minusDays(long);
        inline Time& minusMonths(long);
        inline long minus(const Time&) const;
        int year() const;
        int month() const;
        int dayOfMonth() const;
        int dayOfWeek() const;
        int hour() const;
        int minute() const;
        int second() const;
        inline int milliseond() const;
        inline int microseond() const;
        inline long epochSecond() const;

    private:

        const void* data() const;
        static void* copy(void*, const void*);

        long _seconds;
        long _microseconds;
        void* _data;
    };

    inline Time::Time()
        : _seconds(0)
        , _microseconds(0)
        , _data(0)
    {
        (void)now();
    }

    inline Time::Time(const Time& src)
        : _seconds(src._seconds)
        , _microseconds(src._microseconds)
        , _data(copy(0, src._data))
    {
    }

    inline Time& Time::operator =(const Time& rhs)
    {
        _seconds = rhs._seconds;
        _microseconds = rhs._microseconds;
        _data = copy(_data, rhs._data);
        return *this;
    }

    inline bool Time::operator ==(const Time& rhs) const
    {
        return _seconds == rhs._seconds && _microseconds == rhs._microseconds;
    }

    inline bool Time::operator <(const Time& rhs) const
    {
        return _seconds < rhs._seconds ||
            (_seconds == rhs._seconds && _microseconds < rhs._microseconds);
    }

    inline bool Time::operator <=(const Time& rhs) const
    {
        return _seconds < rhs._seconds ||
            (_seconds == rhs._seconds && _microseconds <= rhs._microseconds);
    }

    inline bool Time::operator >(const Time& rhs) const
    {
        return _seconds > rhs._seconds ||
            (_seconds == rhs._seconds && _microseconds > rhs._microseconds);
    }

    inline bool Time::operator >=(const Time& rhs) const
    {
        return _seconds > rhs._seconds ||
            (_seconds == rhs._seconds && _microseconds >= rhs._microseconds);
    }

    inline Time& Time::addMicroseconds(long value)
    {
        if (value != 0)
        {
            _microseconds += value;
            if (_microseconds > 1000000L)
            {
                _seconds += _microseconds / 1000000L;
                _microseconds %= 1000000L;
                _data = copy(_data, 0);
            }
            else if (_microseconds < 0L)
            {
                long a = _microseconds * (-1L);
                _seconds -= (a + 1000000L - 1L) / 1000000L;
                _microseconds = (1000000L - (a % 1000000L)) % 1000000L;
                _data = copy(_data, 0);
            }
        }
        return *this;
    }

    inline Time& Time::addMilliseconds(long value)
    {
        return addMicroseconds(value * 1000L);
    }

    inline Time& Time::addSeconds(long value)
    {
        if (value != 0)
        {
            _seconds += value;
            _data = copy(_data, 0);
        }
        return *this;
    }

    inline Time& Time::addMinutes(long value)
    {
        if (value != 0)
        {
            _seconds += value * 60L;
            _data = copy(_data, 0);
        }
        return *this;
    }

    inline Time& Time::addHours(long value)
    {
        if (value != 0)
        {
            _seconds += value * 60L * 60L;
            _data = copy(_data, 0);
        }
        return *this;
    }

    inline Time& Time::addDays(long value)
    {
        if (value != 0)
        {
            _seconds += value * 60L * 60L * 24L;
            _data = copy(_data, 0);
        }
        return *this;
    }

    inline Time& Time::minusMicroseconds(long value)
    {
        return addMicroseconds(value * (-1L));
    }

    inline Time& Time::minusMilliseconds(long value)
    {
        return addMicroseconds(value * (-1L) * 1000L);
    }

    inline Time& Time::minusSeconds(long value)
    {
        return addSeconds(value * (-1L));
    }

    inline Time& Time::minusMinutes(long value)
    {
        return addSeconds(value * (-1L) * 60L);
    }

    inline Time& Time::minusHours(long value)
    {
        return addSeconds(value * (-1L) * 60L * 60L);
    }

    inline Time& Time::minusDays(long value)
    {
        return addSeconds(value * (-1L) * 60L * 60L * 24L);
    }

    inline Time& Time::minusMonths(long value)
    {
        return addMonths(value * (-1L));
    }

    inline long Time::minus(const Time& other) const
    {
        return (_microseconds - other._microseconds) + (_seconds - other._seconds) * 1000000L;
    }

    inline int Time::milliseond() const
    {
        return static_cast<int>(_microseconds / 1000L);
    }

    inline int Time::microseond() const
    {
        return static_cast<int>(_microseconds);
    }

    inline long Time::epochSecond() const
    {
        return _seconds;
    }
}


#endif //!HNRT_TIME_H
