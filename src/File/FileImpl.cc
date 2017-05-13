// Copyright (C) 2012-2017 Hideaki Narita


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include "FileImpl.h"


using namespace hnrt;


RefPtr<File> FileImpl::create(const char* path, const char* mode)
{
    return RefPtr<File>(new FileImpl(path, mode));
}


FileImpl::FileImpl(const char* path, const char* mode)
    : _path(NULL)
    , _mode(NULL)
    , _fp(NULL)
    , _infoOutOfDate(true)
    , _error(0)
{
    if (path)
    {
        _path = strdup(path);
        if (!_path)
        {
            throw std::bad_alloc();
        }
    }

    _mode = strdup(mode ? mode : "r");
    if (!_mode)
    {
        throw std::bad_alloc();
    }

    memset(&_info, 0, sizeof(_info));
}


FileImpl::~FileImpl()
{
    if (_fp)
    {
        fclose(_fp);
    }
    free(_path);
    free(_mode);
}


void FileImpl::reset(const char* path, const char* mode)
{
    if (_fp && (path || mode))
    {
        fclose(_fp);
        _fp = NULL;
    }

    if (path)
    {
        free(_path);
        _path = strdup(path);
        if (!_path)
        {
            throw std::bad_alloc();
        }
    }

    if (mode)
    {
        free(_mode);
        _mode = strdup(mode);
        if (!_mode)
        {
            throw std::bad_alloc();
        }
    }

    _error = 0;
    _infoOutOfDate = true;
    _nbytes = 0;
}


bool FileImpl::open(const char* path, const char* mode)
{
    if (_fp)
    {
        fclose(_fp);
        _fp = NULL;
    }

    reset(path, mode);

    _fp = fopen(_path, _mode);
    if (!_fp)
    {
        _error = errno;
        return false;
    }

    if (!updateInfo())
    {
        fclose(_fp);
        _fp = NULL;
        return false;
    }

    return true;
}


bool FileImpl::close()
{
    bool retval = true;
    if (_fp)
    {
        retval = fclose(_fp) == 0 ? true : false;
        if (retval)
        {
            _error = errno;
        }
        _fp = NULL;
        _infoOutOfDate = true;
    }
    return retval;
}


bool FileImpl::updateInfo()
{
    struct stat buf = { 0 };
    if (_fp)
    {
        if (fstat(fileno(_fp), &buf))
        {
            _error = errno;
            return false;
        }
    }
    else if (_path && *_path)
    {
        if (stat(_path, &buf))
        {
            _error = errno;
            return false;
        }
    }
    else
    {
        _error = EINVAL;
        return false;
    }
    memcpy(&_info, &buf, sizeof(buf));
    _infoOutOfDate = false;
    return true;
}


bool FileImpl::canRead() const
{
    if (_fp)
    {
        if (strpbrk(_mode, "r+"))
        {
            return true;
        }
    }
    return false;
}


bool FileImpl::canWrite() const
{
    if (_fp)
    {
        if (strpbrk(_mode, "wa+"))
        {
            return true;
        }
    }
    return false;
}


bool FileImpl::seek(long offset, int whence)
{
    switch (whence)
    {
    case SEEK_SET:
    case SEEK_CUR:
    case SEEK_END:
        break;
    default:
        _error = EINVAL;
        return false;
    }
    if (fseek(_fp, offset, whence) == -1)
    {
        _error = errno;
        return false;
    }
    return true;
}


long FileImpl::tell()
{
    long retval = ftell(_fp);
    if (retval == -1)
    {
        _error = errno;
    }
    return retval;
}


void FileImpl::rewind()
{
    ::rewind(_fp);
}


size_t FileImpl::read(void* ptr, size_t len)
{
    size_t lenRead = fread(ptr, 1, len, _fp);
    if (lenRead < len && ferror(_fp))
    {
        _error = errno;
    }
    _nbytes += lenRead;
    return lenRead;
}


bool FileImpl::write(const void* ptr, size_t len)
{
    size_t wrote = fwrite(ptr, 1, len, _fp);
    _infoOutOfDate = true;
    _nbytes += wrote;
    if (wrote == len)
    {
        return true;
    }
    else
    {
        _error = errno;
        return false;
    }
}


bool FileImpl::validate(volatile bool& abortFlag)
{
    if (!_fp)
    {
        _error = EBADF;
        return false;
    }

    char buf[8192];
    size_t len;

    for (;;)
    {
        if (abortFlag)
        {
            _error = ECANCELED;
            return false;
        }

        len = read(buf, sizeof(buf));
        if (len < sizeof(buf))
        {
            break;
        }
    }

    return _nbytes == size();
}


bool FileImpl::validate(volatile bool* abortFlag)
{
    volatile bool dummyFlag = false;

    if (!abortFlag)
    {
        abortFlag = &dummyFlag;
    }

    return validate(*abortFlag);
}


size_t FileImpl::size() const
{
    if (_infoOutOfDate)
    {
        const_cast<FileImpl*>(this)->updateInfo();
    }
    return _info.st_size;
}


mode_t FileImpl::perm() const
{
    if (_infoOutOfDate)
    {
        const_cast<FileImpl*>(this)->updateInfo();
    }
    return _info.st_mode;
}


uid_t FileImpl::uid() const
{
    if (_infoOutOfDate)
    {
        const_cast<FileImpl*>(this)->updateInfo();
    }
    return _info.st_uid;
}


gid_t FileImpl::gid() const
{
    if (_infoOutOfDate)
    {
        const_cast<FileImpl*>(this)->updateInfo();
    }
    return _info.st_gid;
}


time_t FileImpl::atime() const
{
    if (_infoOutOfDate)
    {
        const_cast<FileImpl*>(this)->updateInfo();
    }
    return _info.st_atime;
}


time_t FileImpl::mtime() const
{
    if (_infoOutOfDate)
    {
        const_cast<FileImpl*>(this)->updateInfo();
    }
    return _info.st_mtime;
}


time_t FileImpl::ctime() const
{
    if (_infoOutOfDate)
    {
        const_cast<FileImpl*>(this)->updateInfo();
    }
    return _info.st_ctime;
}
