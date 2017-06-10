// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHBASE_H
#define HNRT_PATCHBASE_H


#include <stdio.h>
#include <time.h>
#include <glibmm/ustring.h>
#include <list>
#include <map>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "Protocol/HttpClientHandler.h"


namespace hnrt
{
    class File;
    struct PatchRecord;

    class PatchBase
        : public RefObj
        , public HttpClientHandler
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

        virtual bool onSuccess(HttpClient&, int) { return true; }
        virtual bool onFailure(HttpClient&, const char*)  { return false; }
        virtual bool onCancelled(HttpClient&)  { return false; }
        virtual size_t read(HttpClient&, void*, size_t) { return 0; }
        virtual bool write(HttpClient&, const void* ptr, size_t len);
        virtual void rewind(HttpClient&) {}

    protected:

        PatchBase();
        PatchBase(const PatchBase&);
        void operator =(const PatchBase&);
        bool download();

        Glib::ustring _path;
        RefPtr<File> _file;
        RecordMap _recordMap;
        ServerRecordMap _serverRecordMap;
    };
}


#endif //!HNRT_PATCHBASE_H
