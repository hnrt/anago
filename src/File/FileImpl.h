// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_FILEIMPL_H
#define HNRT_FILEIMPL_H


#include "File.h"


namespace hnrt
{
    class FileImpl
        : public File
    {
    public:

        static RefPtr<File> create(const char* = 0, const char* = 0);

        virtual ~FileImpl();
        virtual void reset(const char* = 0, const char* = 0);
        virtual bool open(const char* = 0, const char* = 0);
        virtual bool createExcl(const char* = 0, const char* = 0);
        virtual bool close();
        virtual bool remove();
        virtual bool canRead() const;
        virtual bool canWrite() const;
        virtual bool seek(long, int = SEEK_SET);
        virtual long tell();
        virtual void rewind();
        virtual size_t read(void*, size_t);
        virtual bool write(const void*, size_t);
        virtual bool validate(volatile bool&);
        virtual bool validate(volatile bool* = 0);
        virtual const char* path() const { return _path; }
        virtual const char* mode() const { return _mode; }
        virtual int error() const { return _error; }
        virtual void clearError() { _error = 0; }
        virtual bool exists() const;
        virtual size_t size() const;
        virtual mode_t perm() const;
        virtual uid_t uid() const;
        virtual gid_t gid() const;
        virtual time_t atime() const;
        virtual time_t mtime() const;
        virtual time_t ctime() const;
        virtual size_t nbytes() const { return _nbytes; }
        virtual bool isRegular() const;
        virtual bool isDirectory() const;

    protected:

        FileImpl(const char*, const char*);
        FileImpl(const FileImpl&);
        void operator =(const FileImpl&);
        bool updateInfo();

        char* _path;
        char* _mode;
        FILE* _fp;
        struct stat _info;
        bool _infoOutOfDate;
        int _error;
        size_t _nbytes;
    };
}


#endif //!HNRT_FILEIMPL_H
