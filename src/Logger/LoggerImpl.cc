// Copyright (C) 2012-2017 Hideaki Narita


#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdexcept>
#include "App/Constants.h"
#include "Model/ThreadManager.h"
#include "LoggerImpl.h"


using namespace hnrt;


static Glib::ustring GetLogPath()
{
    return Glib::ustring::compose("%1/.%2.log", getenv("HOME"), APPNAME);
}


static FILE* OpenLogFile(const char* path)
{
    int fd = open(path, O_WRONLY | O_APPEND);
    if (fd == -1)
    {
        if (errno == ENOENT)
        {
            return NULL;
        }
        else
        {
            static char msg[256];
            snprintf(msg, sizeof(msg), "Unable to open %s: %s", path, strerror(errno));
            throw std::runtime_error(msg);
        }
    }

    FILE *fp = fdopen(fd, "a");
    if (!fp)
    {
        static char msg[256];
        snprintf(msg, sizeof(msg), "Unable to open %s: %s", path, strerror(errno));
        close(fd);
        throw std::runtime_error(msg);
    }

    return fp;
}


static char* GetHeader(char* buf, size_t len, LogLevel level)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    struct tm lt;
    localtime_r(&now.tv_sec, &lt);
    snprintf(buf, len, "%d-%02d-%02d %02d:%02d:%02d.%06ld [%s] %s ",
             lt.tm_year + 1900,
             lt.tm_mon + 1,
             lt.tm_mday,
             lt.tm_hour,
             lt.tm_min,
             lt.tm_sec,
             now.tv_usec,
             level.toLocalizedString(),
             ThreadManager::instance().find().c_str());
    return buf;
}


static char* GetSimpleHeader(char* buf, size_t len, LogLevel level)
{
    snprintf(buf, len, "%s: ",
             level.toLocalizedString());
    return buf;
}


LoggerImpl::LoggerImpl()
    : _level(LogLevel::parse(getenv("LOGLEVEL")))
    , _path(GetLogPath())
    , _fp(OpenLogFile(_path.c_str()))
{
}


LoggerImpl::~LoggerImpl()
{
    if (_fp)
    {
        fclose(_fp);
    }
}


void LoggerImpl::trace(const char* format, ...)
{
    if (_fp && _level == LogLevel::TRACE)
    {
        Glib::Mutex::Lock lock(_mutex);
        char buf[64];
        fputs(GetHeader(buf, sizeof(buf), LogLevel::TRACE), _fp);
        va_list argList;
        va_start(argList, format);
        vfprintf(_fp, format, argList);
        va_end(argList);
        putc('\n', _fp);
        fflush(_fp);
    }
}


void LoggerImpl::trace2(const char* name, const char* format, va_list argList)
{
    if (_fp && _level == LogLevel::TRACE)
    {
        Glib::Mutex::Lock lock(_mutex);
        char buf[64];
        fputs(GetHeader(buf, sizeof(buf), LogLevel::TRACE), _fp);
        fprintf(_fp, "%s: ", name);
        vfprintf(_fp, format, argList);
        putc('\n', _fp);
        fflush(_fp);
    }
}


void LoggerImpl::debug(const char* format, ...)
{
    if (_fp && _level <= LogLevel::DEBUG)
    {
        Glib::Mutex::Lock lock(_mutex);
        char buf[64];
        fputs(GetHeader(buf, sizeof(buf), LogLevel::DEBUG), _fp);
        va_list argList;
        va_start(argList, format);
        vfprintf(_fp, format, argList);
        va_end(argList);
        putc('\n', _fp);
        fflush(_fp);
    }
}


void LoggerImpl::debug2(const char* name, const char* format, va_list argList)
{
    if (_fp && _level <= LogLevel::DEBUG)
    {
        Glib::Mutex::Lock lock(_mutex);
        char buf[64];
        fputs(GetHeader(buf, sizeof(buf), LogLevel::DEBUG), _fp);
        fprintf(_fp, "%s: ", name);
        vfprintf(_fp, format, argList);
        putc('\n', _fp);
        fflush(_fp);
    }
}


void LoggerImpl::info(const char* format, ...)
{
    if (_fp && _level <= LogLevel::INFO)
    {
        Glib::Mutex::Lock lock(_mutex);
        char buf[64];
        fputs(GetHeader(buf, sizeof(buf), LogLevel::INFO), _fp);
        va_list argList;
        va_start(argList, format);
        vfprintf(_fp, format, argList);
        va_end(argList);
        putc('\n', _fp);
        fflush(_fp);
    }
}


void LoggerImpl::warn(const char* format, ...)
{
    if (_fp && _level <= LogLevel::WARNING)
    {
        Glib::Mutex::Lock lock(_mutex);
        char buf[64];
        fputs(GetHeader(buf, sizeof(buf), LogLevel::WARNING), _fp);
        va_list argList;
        va_start(argList, format);
        vfprintf(_fp, format, argList);
        va_end(argList);
        putc('\n', _fp);
        fflush(_fp);
    }
}


void LoggerImpl::error(const char* format, ...)
{
    Glib::Mutex::Lock lock(_mutex);
    char buf[64];
    if (_fp && _level <= LogLevel::ERROR)
    {
        fputs(GetHeader(buf, sizeof(buf), LogLevel::ERROR), _fp);
        va_list argList;
        va_start(argList, format);
        vfprintf(_fp, format, argList);
        va_end(argList);
        putc('\n', _fp);
        fflush(_fp);
    }
    fputs(GetSimpleHeader(buf, sizeof(buf), LogLevel::ERROR), stderr);
    va_list argList;
    va_start(argList, format);
    vfprintf(stderr, format, argList);
    va_end(argList);
    putc('\n', stderr);
    fflush(stderr);
}


void LoggerImpl::fatal(const char* format, ...)
{
    Glib::Mutex::Lock lock(_mutex);
    char buf[64];
    if (_fp && _level <= LogLevel::FATAL)
    {
        fputs(GetHeader(buf, sizeof(buf), LogLevel::FATAL), _fp);
        va_list argList;
        va_start(argList, format);
        vfprintf(_fp, format, argList);
        va_end(argList);
        putc('\n', _fp);
        fflush(_fp);
    }
    fputs(GetSimpleHeader(buf, sizeof(buf), LogLevel::FATAL), stderr);
    va_list argList;
    va_start(argList, format);
    vfprintf(stderr, format, argList);
    va_end(argList);
    putc('\n', stderr);
    fflush(stderr);
}
