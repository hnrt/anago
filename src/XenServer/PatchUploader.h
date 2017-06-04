// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHUPLOADER_H
#define HNRT_PATCHUPLOADER_H


#include <stdio.h>
#include <glibmm/ustring.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "Protocol/HttpClientHandler.h"


namespace hnrt
{
    class File;
    class Session;

    class PatchUploader
        : public RefObj
        , public HttpClientHandler
    {
    public:

        static RefPtr<PatchUploader> create();

        ~PatchUploader();
        bool run(Session&, const char*);

        virtual bool onSuccess(HttpClient&, int);
        virtual bool onFailure(HttpClient&, const char*);
        virtual bool onCancelled(HttpClient&);
        virtual size_t read(HttpClient&, void*, size_t);
        virtual void rewind(HttpClient&);

    private:

        PatchUploader();
        PatchUploader(const PatchUploader&);
        void operator =(const PatchUploader&);

        RefPtr<File> _file;
    };
}


#endif //!HNRT_PATCHUPLOADER_H
