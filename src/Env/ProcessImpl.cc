// Copyright (C) 2012-2018 Hideaki Narita


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <stdexcept>
#include "ProcessImpl.h"


using namespace hnrt;


static Glib::ustring GetExecutablePath()
{
#ifdef LINUX

    std::vector<char> path(32);
    while (1)
    {
        ssize_t len = readlink("/proc/self/exe", &path[0], path.size());
        if (len < 0)
        {
            static char msg[256];
            snprintf(msg, sizeof(msg), "Unable to read /proc/self/exe: %s", strerror(errno));
            throw std::runtime_error(msg);
        }
        if (len < (ssize_t)path.size())
        {
            path[len] = '\0';
            break;
        }
        path.resize(path.size() * 2);
    }
    return Glib::ustring(&path[0]);

#else
#error Unsupported platform.
#endif
}


ProcessImpl::ProcessImpl()
    : _executablePath(GetExecutablePath())
{
}


ProcessImpl::~ProcessImpl()
{
}


Glib::ustring ProcessImpl::getExecutableDirectory() const
{
    ssize_t i = _executablePath.rfind('/');
    return i >= 0 ? _executablePath.substr(0, i + 1) : Glib::ustring();
}
