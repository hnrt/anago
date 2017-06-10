// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHDOWNLOADER_H
#define HNRT_PATCHDOWNLOADER_H


#include <glibmm/thread.h>
#include <glibmm/ustring.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "Protocol/HttpClientHandler.h"


namespace hnrt
{
    class File;

    class PatchDownloader
        : public RefObj
        , public HttpClientHandler
    {
    public:

        static RefPtr<PatchDownloader> create();

        ~PatchDownloader();
        void run(const Glib::ustring&, const Glib::ustring&);

        virtual bool onSuccess(HttpClient&, int);
        virtual bool onFailure(HttpClient&, const char*);
        virtual bool onCancelled(HttpClient&);
        virtual size_t read(HttpClient&, void*, size_t) { return 0; }
        virtual bool write(HttpClient&, const void*, size_t);
        virtual void rewind(HttpClient&) {}

    private:

        PatchDownloader();
        PatchDownloader(const PatchDownloader&);
        void operator =(const PatchDownloader&);

        RefPtr<File> _file;
        Glib::Mutex _mutex;
        double _written;
    };
}


#endif //!HNRT_PATCHDOWNLOADER_H
