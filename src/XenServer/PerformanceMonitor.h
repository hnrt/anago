// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PERFORMANCEMONITOR_H
#define HNRT_PERFORMANCEMONITOR_H


#include <glibmm.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <list>
#include <map>
#include <vector>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"


namespace hnrt
{
    class Session;
    class Scrambler;

    class PerformanceMonitor
        : public RefObj
    {
    public:

        enum Notification
        {
            CREATED = 512,
            UPDATED,
            DESTROYED,
        };

        typedef std::pair<Glib::ustring, Glib::ustring> MapEntry;
        typedef std::map<Glib::ustring, Glib::ustring> Map;
        typedef std::pair<unsigned long, Map*> ListEntry;
        typedef std::list<ListEntry> List;
        typedef std::pair<Glib::ustring, List*> StoreEntry;
        typedef std::map<Glib::ustring, List*> Store;

        static RefPtr<PerformanceMonitor> create(Session&);

        virtual ~PerformanceMonitor();
        Session& getSession() const;
        void terminate();
        void run();
        bool parse(void* ptr, size_t len);
        ListEntry getEntry(const Glib::ustring& uuid);

    protected:

        enum LegendType
        {
            LEGENDTYPE_UNKNOWN,
            LEGENDTYPE_HOST,
            LEGENDTYPE_VM,
        };

        struct Legend
        {
            LegendType type;
            Glib::ustring uuid;
            Glib::ustring name;

            Legend()
               : type(LEGENDTYPE_UNKNOWN)
            {
            }

            Legend(const Legend& src)
                : type(src.type)
                , uuid(src.uuid)
                , name(src.name)
            {
            }
        };

        PerformanceMonitor(Session&);
        PerformanceMonitor(const PerformanceMonitor&);
        void operator =(const PerformanceMonitor&);
        void parseRoot(xmlNode* pNode);
        void parseMeta(xmlNode* pNode, std::vector<Legend>& columns);
        void parseLegend(xmlNode* pNode, std::vector<Legend>& columns);
        void parseData(xmlNode* pNode, const std::vector<Legend>& columns);
        unsigned long parseTime(xmlNode* pNode);
        void parseValues(unsigned long t, xmlNode* pNode, const std::vector<Legend>& columns);

        Session& _session;
        Glib::ustring _location;
        Glib::ustring _authorization;
        Glib::Mutex _mutex;
        Glib::Cond _cond;
        volatile bool _terminate;
        void* _context;
        void* _doc;
        unsigned long _end;
        Store _store;
    };
}


#endif //!HNRT_PERFORMANCEMONITOR_H
