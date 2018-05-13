// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_PATCH_H
#define HNRT_PATCH_H


#include "Base/Time.h"
#include "XenObject.h"


namespace hnrt
{
    class File;
    class HttpClient;
    class ThinClientInterface;
    struct PatchRecord;

    class Patch
        : public XenObject
    {
    public:

        static RefPtr<Patch> create(Session&, const RefPtr<PatchRecord>&);

        virtual ~Patch();
        void init();
        void initDownload();
        void fini();
        RefPtr<PatchRecord> getRecord() const;
        bool download();
        bool upload();
        bool apply();
        const Glib::ustring& getOutput() const { return _output; }
        const Glib::ustring& getErrorOutput() const { return _errorOutput; }
        int getExitCode() const { return _exitCode; }
        const Glib::ustring& getPath() const { return _path; }
        size_t getExpected() const { return _expected > 0 ? _expected : 0; }
        size_t getActual() const { return _actual; }

    protected:

        enum ProtectedConstants
        {
            UPDATE_INTERVAL = 250,
        };

        Patch(Session&, const RefPtr<PatchRecord>&);
        Patch(const Patch&);
        void operator =(const Patch&);
        bool write(const void*, size_t, RefPtr<File>);
        void print(const char*);
        void printError(const char*);
        void exit(int);
        void reportProgress(size_t);

        RefPtr<PatchRecord> _record;
        Glib::ustring _path;
        RefPtr<HttpClient> _httpClient;
        ssize_t _expected;
        ssize_t _actual;
        Time _updateAfter;
        RefPtr<ThinClientInterface> _cli;
        Glib::ustring _output;
        Glib::ustring _errorOutput;
        int _exitCode;
    };
}


#endif //!HNRT_PATCH_H
