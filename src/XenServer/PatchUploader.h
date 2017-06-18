// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHUPLOADER_H
#define HNRT_PATCHUPLOADER_H


#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class Session;

    class PatchUploader
        : public RefObj
    {
    public:

        static RefPtr<PatchUploader> create();

        ~PatchUploader();
        bool run(Session&, const char*);

    private:

        PatchUploader();
        PatchUploader(const PatchUploader&);
        void operator =(const PatchUploader&);
    };
}


#endif //!HNRT_PATCHUPLOADER_H
