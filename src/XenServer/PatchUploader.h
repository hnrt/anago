// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHUPLOADER_H
#define HNRT_PATCHUPLOADER_H


#include <stdio.h>
#include <glibmm/ustring.h>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class Host;
    class File;

    class PatchUploader
        : public RefObj
    {
    public:

        static RefPtr<PatchUploader> create();

        ~PatchUploader();
        void init();
        void fini();
        void run(Host&, const Glib::ustring&);
        size_t read(void*, size_t);
        void rewind();

    private:

        PatchUploader();
        PatchUploader(const PatchUploader&);
        void operator =(const PatchUploader&);
        void updateStatus();

        RefPtr<Host> _host;
        RefPtr<File> _file;
    };
}


#endif //!HNRT_PATCHUPLOADER_H
