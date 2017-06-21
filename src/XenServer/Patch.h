// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCH_H
#define HNRT_PATCH_H


#include "XenObject.h"


namespace hnrt
{
    class ThinClientInterface;
    struct PatchRecord;

    class Patch
        : public XenObject
    {
    public:

        static RefPtr<Patch> create(Session&, const RefPtr<PatchRecord>&);

        virtual ~Patch();
        void init();
        void fini();
        RefPtr<PatchRecord> getRecord() const;
        bool upload();
        bool apply();
        const Glib::ustring& getOutput() const;
        const Glib::ustring& getErrorOutput() const;
        int getExitCode() const;
        const Glib::ustring& getPath() const { return _path; }

    protected:

        Patch(Session&, const RefPtr<PatchRecord>&);
        Patch(const Patch&);
        void operator =(const Patch&);
        void print(ThinClientInterface&);
        void printError(ThinClientInterface&);
        void exit(ThinClientInterface&);

        RefPtr<PatchRecord> _record;
        RefPtr<ThinClientInterface> _cli;
        Glib::ustring _path;
    };
}


#endif //!HNRT_PATCH_H
