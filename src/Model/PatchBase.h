// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_PATCHBASE_H
#define HNRT_PATCHBASE_H


#include <stdio.h>
#include <time.h>
#include <glibmm/ustring.h>
#include <list>
#include <map>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class File;
    struct PatchRecord;

    class PatchBase
        : public RefObj
    {
    public:

        static RefPtr<PatchBase> create();

        virtual ~PatchBase();
        void init();
        void fini();
        bool load();

        struct ServerRecord
            : public RefObj
        {
            Glib::ustring name;
            Glib::ustring build;
            time_t timestamp;
            Glib::ustring url;
            Glib::ustring version;
            std::list<Glib::ustring> patches;
            ~ServerRecord() {}
            static RefPtr<ServerRecord> create() { return RefPtr<ServerRecord>(new ServerRecord); }
        protected:
            ServerRecord() : timestamp(0) {}
        };

        struct ServerRecordMap
            : public std::map<Glib::ustring, RefPtr<ServerRecord> >
        {
            ServerRecordMap();
            ~ServerRecordMap();
            void put(const RefPtr<ServerRecord>&);
            RefPtr<ServerRecord> get(const Glib::ustring&) const;
        };

        struct RecordMap
            : public std::map<Glib::ustring, RefPtr<PatchRecord> >
        {
            RecordMap();
            ~RecordMap();
            void put(const RefPtr<PatchRecord>&);
            RefPtr<PatchRecord> get(const Glib::ustring&) const;
        };

        struct RecordIterator
        {
            PatchBase* patchBase;
            RefPtr<ServerRecord> serverRecord;
            std::list<Glib::ustring>::const_iterator iterator;
            RecordIterator();
            RecordIterator(const RecordIterator&);
            void operator =(const RecordIterator&);
            RefPtr<PatchRecord> next();
        };

        RecordIterator getRecordIterator(const char*);
        RefPtr<PatchRecord> getRecord(const Glib::ustring&) const;

    protected:

        PatchBase();
        PatchBase(const PatchBase&);
        void operator =(const PatchBase&);
        bool download();
        bool write(const void* ptr, size_t len);

        Glib::ustring _path;
        RefPtr<File> _file;
        RecordMap _recordMap;
        ServerRecordMap _serverRecordMap;
    };
}


#endif //!HNRT_PATCHBASE_H
