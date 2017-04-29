// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_FILE_H
#define HNRT_FILE_H


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class File
        : public RefObj
    {
    public:

        static RefPtr<File> create(const char* = 0, const char* = 0);

        virtual ~File();
        virtual void reset(const char* = 0, const char* = 0) = 0;
        virtual bool open(const char* = 0, const char* = 0) = 0;
        virtual bool close() = 0;
        virtual bool canRead() const = 0;
        virtual bool canWrite() const = 0;
        virtual bool seek(long, int = SEEK_SET) = 0;
        virtual long tell() = 0;
        virtual void rewind() = 0;
        virtual size_t read(void*, size_t) = 0;
        virtual bool write(const void*, size_t) = 0;
        virtual bool validate(volatile bool&) = 0;
        virtual bool validate(volatile bool* = 0) = 0;
        virtual const char* path() const = 0;
        virtual const char* mode() const = 0;
        virtual int error() const = 0;
        virtual void clearError() = 0;
        virtual size_t size() const = 0;
        virtual mode_t perm() const = 0;
        virtual uid_t uid() const = 0;
        virtual gid_t gid() const = 0;
        virtual time_t atime() const = 0;
        virtual time_t mtime() const = 0;
        virtual time_t ctime() const = 0;
        virtual size_t nbytes() const = 0;

    protected:

        File();
        File(const File&);
        void operator =(const File&);
    };
}


#endif //!HNRT_FILE_H
