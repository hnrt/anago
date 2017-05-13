// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_LOGLEVEL_H
#define HNRT_LOGLEVEL_H


namespace hnrt
{
    class LogLevel
    {
    public:

        enum Value
        {
            TRACE = 1,
            DEBUG,
            INFO,
            WARNING,
            ERROR,
            FATAL,
            UNDEFINED = 99,
        };

        static LogLevel parse(const char*);

        LogLevel(Value value = UNDEFINED) : _value(value) {}
        LogLevel(const LogLevel& src) : _value(src._value) {}
        LogLevel& operator =(const LogLevel& src) { _value = src._value; return *this; }
        operator Value() const { return _value; }
        const char* toString() const;
        const char* toLocalizedString() const;

    private:

        Value _value;
    };
}


#endif //!HNRT_LOGLEVEL_H
