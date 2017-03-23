// Copyright (C) 2012-2017 Hideaki Narita


#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <vector>
#include "Base/Atomic.h"
#include "Base/StringBuffer.h"
#include "Controller/Controller.h"
#include "Logger/Trace.h"
#include "Util/Base64.h"
#include "PerformanceMonitor.h"
#include "Session.h"


using namespace hnrt;


RefPtr<PerformanceMonitor> PerformanceMonitor::create(Session& session)
{
    return RefPtr<PerformanceMonitor>(new PerformanceMonitor(session));
}


PerformanceMonitor::PerformanceMonitor(Session& session)
    : _session(session)
    , _terminate(false)
    , _context(NULL)
    , _doc(NULL)
    , _end(static_cast<unsigned long>(time(NULL)) - 1)
{
    Trace trace("PerformanceMonitor::ctor");
    _session.incRef();
}


PerformanceMonitor::~PerformanceMonitor()
{
    Trace trace("PerformanceMonitor::dtor");

    if (_doc)
    {
        xmlFreeDoc(reinterpret_cast<xmlDocPtr>(_doc));
    }

    for (Store::iterator i = _store.begin(); i != _store.end(); i++)
    {
        List* pList = i->second;
        i->second = NULL;
        for (List::iterator j = pList->begin(); j != pList->end(); j++)
        {
            Map* pMap = j->second;
            j->second = NULL;
            delete pMap;
        }
        delete pList;
    }

    _session.decRef();
}


Session& PerformanceMonitor::getSession() const
{
    return _session;
}


void PerformanceMonitor::terminate()
{
    _terminate = true;
    _cond.signal();
}


static size_t receive(void* ptr, size_t size, size_t nmemb, PerformanceMonitor* pThis)
{
    size_t len = size * nmemb;
    size_t ret = pThis->parse(ptr, len) ? len : 0;
    return ret;
}


class Traverse
{
public:

    Traverse(xmlDocPtr pDoc)
        : _pDoc(pDoc)
    {
    }

    void print()
    {
        print(xmlDocGetRootElement(_pDoc));
    }

    void print(xmlNode* pNode, int depth = 0)
    {
        for (xmlNode* pCur = pNode; pCur; pCur = pCur->next)
        {
            if (pCur->type == XML_ELEMENT_NODE)
            {
                if (hasTextChild(pCur->children))
                {
                    xmlChar* pContent = xmlNodeGetContent(pCur);
                    printf("# %*s%s=(%s)\n", depth * 2, "", pCur->name, pContent ? reinterpret_cast<char*>(pContent) : "");
                }
                else
                {
                    printf("# %*s%s\n", depth * 2, "", pCur->name);
                }
            }
            print(pCur->children, depth + 1);
        }
    }

    bool hasTextChild(xmlNode* pNode)
    {
        return pNode && pNode->type == XML_TEXT_NODE && !pNode->next;
    }

private:

    xmlDocPtr _pDoc;
};


void PerformanceMonitor::run()
{
    Glib::Mutex::Lock lock(_mutex);
    Glib::TimeVal tv;

    const ConnectSpec& cs = _session.getConnectSpec();
    _location = cs.hostname;
    _authorization = cs.getBasicAuthString();
    _terminate = false;

    for (;;)
    {
        // wait until the next 5-second interval
        tv.assign_current_time();
        long ct = static_cast<long>(tv.as_double() * 1000.0);
        long nt = ((ct + 5000L) / 5000L) * 5000L;
        long dt = nt - ct + 1000L; // 1 second delay for the latest stats to be ready
        tv.add_milliseconds(dt);
        _cond.timed_wait(_mutex, tv);
        if (_terminate || !_session.isConnected())
        {
            break;
        }

        CURL* curl = curl_easy_init();
        if (!curl)
        {
            continue;
        }

        xmlDocPtr pDoc = NULL;
        xmlParserCtxtPtr pContext = NULL;

        try
        {
            StringBuffer url;
            Base64Decoder bd(_authorization.c_str());
            url.format("https://%s@%s/rrd_updates?host=true&start=%lu",
                       reinterpret_cast<const char*>(bd.getValue()),
                       _location.c_str(),
                       _end + 1);

            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
            curl_easy_setopt(curl, CURLOPT_URL, url.str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

            //printf("%lu %s\n", time(NULL), url.str());

            CURLcode result = curl_easy_perform(curl);

            url.clear();

            pContext = reinterpret_cast<xmlParserCtxtPtr>(InterlockedExchangePointer(&_context, reinterpret_cast<void*>(NULL)));

            if (result == CURLE_OK)
            {
                if (pContext)
                {
                    xmlParseChunk(pContext, "", 0, 1);
                    pDoc = pContext->myDoc;
                    //Traverse(pDoc).print();
                    parseRoot(xmlDocGetRootElement(pDoc));
                    _session.emit(XenObject::PERFORMANCE_STATS_UPDATED);
                }
            }
            else
            {
                Logger::instance().error("PerformanceMonitor: curl_error=%d", result);
            }
        }
        catch (...)
        {
        }

        curl_easy_cleanup(curl);

        if (pContext)
        {
            xmlFreeParserCtxt(pContext);
        }

        pDoc = reinterpret_cast<xmlDocPtr>(InterlockedExchangePointer(&_doc, reinterpret_cast<void*>(pDoc)));
        if (pDoc)
        {
            xmlFreeDoc(pDoc);
        }
    }
}


bool PerformanceMonitor::parse(void* ptr, size_t len)
{
    //printf("#PerformanceMonitor::parse(%zu)\n", len);

    if (!_context)
    {
        _context = xmlCreatePushParserCtxt(NULL, NULL, reinterpret_cast<char*>(ptr), len, "quux");
    }
    else if (len)
    {
        xmlParseChunk(reinterpret_cast<xmlParserCtxtPtr>(_context), reinterpret_cast<char*>(ptr), len, 0);
    }

    //printf("%.*s\n", static_cast<int>(len), reinterpret_cast<char*>(ptr));

    return true;
}


void PerformanceMonitor::parseRoot(xmlNode* pNode)
{
    if (pNode && pNode->type == XML_ELEMENT_NODE)
    {
        std::vector<Legend> columns;
        for (xmlNode* pCur = pNode->children; pCur; pCur = pCur->next)
        {
            if (pCur->type == XML_ELEMENT_NODE)
            {
                if (!strcmp(reinterpret_cast<const char*>(pCur->name), "meta"))
                {
                    parseMeta(pCur->children, columns);
                }
                else if (!strcmp(reinterpret_cast<const char*>(pCur->name), "data"))
                {
                    parseData(pCur->children, columns);
                }
            }
        }
    }
}


void PerformanceMonitor::parseMeta(xmlNode* pNode, std::vector<Legend>& columns)
{
    for (xmlNode* pCur = pNode; pCur; pCur = pCur->next)
    {
        if (pCur->type == XML_ELEMENT_NODE)
        {
            if (!strcmp(reinterpret_cast<const char*>(pCur->name), "start"))
            {
            }
            else if (!strcmp(reinterpret_cast<const char*>(pCur->name), "step"))
            {
            }
            else if (!strcmp(reinterpret_cast<const char*>(pCur->name), "end"))
            {
                xmlChar* pContent = xmlNodeGetContent(pCur);
                if (pContent)
                {
                    unsigned long t = strtoul(reinterpret_cast<const char*>(pContent), NULL, 10);
                    if (_end < t)
                    {
                        _end = t;
                    }
                }
            }
            else if (!strcmp(reinterpret_cast<const char*>(pCur->name), "rows"))
            {
            }
            else if (!strcmp(reinterpret_cast<const char*>(pCur->name), "columns"))
            {
            }
            else if (!strcmp(reinterpret_cast<const char*>(pCur->name), "legend"))
            {
                parseLegend(pCur->children, columns);
            }
        }
    }
}


void PerformanceMonitor::parseLegend(xmlNode* pNode, std::vector<Legend>& columns)
{
    for (xmlNode* pCur = pNode; pCur; pCur = pCur->next)
    {
        if (pCur->type == XML_ELEMENT_NODE)
        {
            if (!strcmp(reinterpret_cast<const char*>(pCur->name), "entry"))
            {
                xmlChar* pContent = xmlNodeGetContent(pCur);
                if (pContent)
                {
                    Legend legend;
                    if (!strncmp(reinterpret_cast<const char*>(pContent), "AVERAGE:host:", 13))
                    {
                        if (strlen(reinterpret_cast<const char*>(pContent)) > 50)
                        {
                            legend.type = LEGENDTYPE_HOST;
                            legend.uuid = Glib::ustring(reinterpret_cast<const char*>(pContent) + 13, 36);
                            legend.name = Glib::ustring(reinterpret_cast<const char*>(pContent) + 50);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if (!strncmp(reinterpret_cast<const char*>(pContent), "AVERAGE:vm:", 11))
                    {
                        if (strlen(reinterpret_cast<const char*>(pContent)) > 48)
                        {
                            legend.type = LEGENDTYPE_VM;
                            legend.uuid = Glib::ustring(reinterpret_cast<const char*>(pContent) + 11, 36);
                            legend.name = Glib::ustring(reinterpret_cast<const char*>(pContent) + 48);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }
                    columns.push_back(legend);
                    Store::iterator i = _store.find(legend.uuid);
                    if (i == _store.end())
                    {
                        _store.insert(StoreEntry(legend.uuid, new List));
                    }
                }
            }
        }
    }
}


void PerformanceMonitor::parseData(xmlNode* pNode, const std::vector<Legend>& columns)
{
    std::map<unsigned long, xmlNode*> tMap;
    for (xmlNode* pCur = pNode; pCur; pCur = pCur->next)
    {
        if (pCur->type == XML_ELEMENT_NODE)
        {
            if (!strcmp(reinterpret_cast<const char*>(pCur->name), "row"))
            {
                unsigned long t = parseTime(pCur->children);
                if (t)
                {
                    if (tMap.find(t) == tMap.end())
                    {
                        tMap.insert(std::pair<unsigned long, xmlNode*>(t, pCur->children));
                    }
                }
            }
        }
    }
    for (std::map<unsigned long, xmlNode*>::iterator i = tMap.begin(); i != tMap.end(); i++)
    {
        parseValues(i->first, i->second, columns);
    }
}


unsigned long PerformanceMonitor::parseTime(xmlNode* pNode)
{
    for (xmlNode* pCur = pNode; pCur; pCur = pCur->next)
    {
        if (pCur->type == XML_ELEMENT_NODE)
        {
            if (!strcmp(reinterpret_cast<const char*>(pCur->name), "t"))
            {
                xmlChar* pContent = xmlNodeGetContent(pCur);
                if (pContent)
                {
                    return static_cast<unsigned long>(strtod(reinterpret_cast<const char*>(pContent), NULL));
                }
            }
        }
    }
    return 0;
}


void PerformanceMonitor::parseValues(unsigned long t, xmlNode* pNode, const std::vector<Legend>& columns)
{
    int i = 0;
    int n = static_cast<int>(columns.size());
    Glib::ustring uuid("");
    Map* pMap = NULL;
    std::map<Glib::ustring, Map*> uMap;
    for (xmlNode* pCur = pNode; pCur && i < n; pCur = pCur->next)
    {
        if (pCur->type == XML_ELEMENT_NODE)
        {
            if (!strcmp(reinterpret_cast<const char*>(pCur->name), "v"))
            {
                const Legend& legend = columns[i++];
                if (legend.uuid != uuid)
                {
                    uuid = legend.uuid;
                    std::map<Glib::ustring, Map*>::iterator j = uMap.find(uuid);
                    if (j != uMap.end())
                    {
                        pMap = j->second;
                    }
                    else
                    {
                        pMap = new Map;
                        List* pList = _store.find(uuid)->second;
                        while (pList->size() > 100)
                        {
                            ListEntry& entry = pList->front();
                            delete entry.second;
                            pList->pop_front();
                        }
                        pList->push_back(ListEntry(t, pMap));
                        uMap.insert(std::pair<Glib::ustring, Map*>(uuid, pMap));
                    }
                }
                xmlChar* pContent = xmlNodeGetContent(pCur);
                if (pContent)
                {
                    pMap->insert(MapEntry(legend.name, Glib::ustring(reinterpret_cast<const char*>(pContent))));
                }
            }
        }
    }
}


PerformanceMonitor::ListEntry PerformanceMonitor::getEntry(const Glib::ustring& uuid)
{
    Glib::Mutex::Lock lock(_mutex);
    Store::iterator iter = _store.find(uuid);
    if (iter != _store.end())
    {
        List* pList = iter->second;
        if (!pList->empty())
        {
            ListEntry entry = pList->front();
            pList->pop_front();
            return entry;
        }
    }
    return ListEntry(0, NULL);
}
