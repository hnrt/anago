// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHDOWNLOADER_H
#define HNRT_PATCHDOWNLOADER_H


#include <glibmm/thread.h>
#include <glibmm/ustring.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class File;

    class PatchDownloader
        : public RefObj
    {
    public:

        static RefPtr<PatchDownloader> create();

        ~PatchDownloader();
        void init();
        void fini();
        void run(const Glib::ustring&, const Glib::ustring&);
        bool parse(void*, size_t);

    private:

        PatchDownloader();
        PatchDownloader(const PatchDownloader&);
        void operator =(const PatchDownloader&);

        RefPtr<File> _file;
        Glib::Mutex _mutex;
        void* _context;
        double _contentLength;
        double _written;
    };
}


#endif //!HNRT_PATCHDOWNLOADER_H
