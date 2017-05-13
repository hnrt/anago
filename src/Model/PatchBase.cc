// Copyright (C) 2012-2017 Hideaki Narita


#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glibmm.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "PatchBase.h"
#include "PatchRecord.h"


using namespace hnrt;


RefPtr<PatchBase> PatchBase::create()
{
    return RefPtr<PatchBase>(new PatchBase);
}


PatchBase::PatchBase()
    : _fp(0)
    , _size(0)
{
    TRACE(StringBuffer().format("PatchBase@%zx::ctor", this));
}


PatchBase::~PatchBase()
{
    TRACE(StringBuffer().format("PatchBase@%zx::dtor", this));
}


void PatchBase::init()
{
    TRACE(StringBuffer().format("PatchBase@%zx::init", this));
    _path = Glib::ustring::compose("%1%2", Model::instance().getAppDir(), "updates.xml");
    TRACEPUT("path=%s", _path.c_str());
}


void PatchBase::fini()
{
    TRACE(StringBuffer().format("PatchBase@%zx::fini", this));
}


#if 0
static const char* GetNodeTypeText(xmlElementType type)
{
    switch (type)
    {
    case XML_ELEMENT_NODE: return "ELEMENT";
    case XML_ATTRIBUTE_NODE: return "ATTRIBUTE";
    case XML_TEXT_NODE: return "TEXT";
    default: break;
    }
    static char buf[64];
    sprintf(buf, "%d", (int)type);
    return buf;
}
#endif


static bool ParseTimestamp(const char* s, time_t& value)
{
    value = 0;
    struct tm x = { 0 };
    if (isdigit(s[0]) &&
        isdigit(s[1]) &&
        isdigit(s[2]) &&
        isdigit(s[3]))
    {
        x.tm_year = ((s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0') * 1) - 1900;
        s += 4;
    }
    else
    {
        return false;
    }
    if (*s == '-')
    {
        s++;
    }
    if (isdigit(s[0]) &&
        isdigit(s[1]))
    {
        x.tm_mon = (s[0] - '0') * 10 + (s[1] - '0') * 1;
        s += 2;
    }
    else
    {
        return false;
    }
    if (*s == '-')
    {
        s++;
    }
    if (isdigit(s[0]) &&
        isdigit(s[1]))
    {
        x.tm_mday = (s[0] - '0') * 10 + (s[1] - '0') * 1;
        s += 2;
    }
    else
    {
        return false;
    }
    if (*s == 'T')
    {
        s++;
    }
    else
    {
        return false;
    }
    if (isdigit(s[0]) &&
        isdigit(s[1]))
    {
        x.tm_hour = (s[0] - '0') * 10 + (s[1] - '0') * 1;
        s += 2;
    }
    else
    {
        return false;
    }
    if (*s == ':')
    {
        s++;
    }
    if (isdigit(s[0]) &&
        isdigit(s[1]))
    {
        x.tm_min = (s[0] - '0') * 10 + (s[1] - '0') * 1;
        s += 2;
    }
    else
    {
        return false;
    }
    if (*s == ':')
    {
        s++;
    }
    if (isdigit(s[0]) &&
        isdigit(s[1]))
    {
        x.tm_sec = (s[0] - '0') * 10 + (s[1] - '0') * 1;
        s += 2;
    }
    int tz = 0;
    if (*s == 'Z')
    {
        s++;
    }
    else if (*s == '+' || *s == '-')
    {
        char c = *s++;
        int h = 0;
        int m = 0;
        if (isdigit(s[0]) &&
            isdigit(s[1]))
        {
            h = (s[0] - '0') * 10 + (s[1] - '0') * 1;
            s += 2;
        }
        else
        {
            return false;
        }
        if (*s == ':')
        {
            s++;
        }
        if (isdigit(s[0]) &&
            isdigit(s[1]))
        {
            m = (s[0] - '0') * 10 + (s[1] - '0') * 1;
            s += 2;
        }
        else
        {
            return false;
        }
        if (c == '+')
        {
            tz -= h * 60 * 60 + m * 60;
        }
        else
        {
            tz += h * 60 * 60 + m * 60;
        }
    }
    else
    {
        return false;
    }
    if (*s == '\0')
    {
        value = timegm(&x) + tz;
        return true;
    }
    else
    {
        return false;
    }
}


static bool ParsePatch(xmlNode* node, PatchRecord& record)
{
    for (xmlAttr* attr = node->properties; attr != NULL; attr = attr->next)
    {
        if (attr->type == XML_ATTRIBUTE_NODE)
        {
            const char* value;
            if (attr->children != NULL && attr->children->type == XML_TEXT_NODE)
            {
                value = reinterpret_cast<const char*>(attr->children->content);
            }
            else
            {
                value = "";
            }
            if (!strcmp(reinterpret_cast<const char*>(attr->name), "after-apply-guidance"))
            {
                record.afterApplyGuidance = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "guidance-mandatory"))
            {
                if (!strcmp(value, "true"))
                {
                    record.guidanceMandatory = 1;
                }
                else if (!strcmp(value, "false"))
                {
                    record.guidanceMandatory = 0;
                }
                else
                {
                    record.guidanceMandatory = -1;
                }
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "installation-size"))
            {
                record.size = strtoul(value, NULL, 10);
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "name-description"))
            {
                record.description = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "name-label"))
            {
                record.label = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "patch-url"))
            {
                record.patchUrl = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "releasenotes"))
            {
                record.releaseNotes = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "timestamp"))
            {
                ParseTimestamp(value, record.timestamp);
#if 0
                struct tm result = { 0 };
                gmtime_r(&record.timestamp, &result);
                g_print("\n%s\n%04d-%02d-%02d*%02d:%02d:%02d*\n", value, result.tm_year + 1900, result.tm_mon, result.tm_mday, result.tm_hour, result.tm_min, result.tm_sec);
#endif
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "url"))
            {
                record.url = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "uuid"))
            {
                record.uuid = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "version"))
            {
                record.version = value;
            }
        }
    }

    if (record.uuid.bytes() == 0)
    {
        return false;
    }

    for (xmlNode* node2 = node->children; node2 != NULL; node2 = node2->next)
    {
        if (node2->type == XML_ELEMENT_NODE)
        {
            if (!strcmp(reinterpret_cast<const char*>(node2->name), "conflictingpatches"))
            {
                for (xmlNode* node3 = node2->children; node3 != NULL; node3 = node3->next)
                {
                    if (node3->type == XML_ELEMENT_NODE)
                    {
                        if (!strcmp(reinterpret_cast<const char*>(node3->name), "conflictingpatch"))
                        {
                            for (xmlAttr* attr = node3->properties; attr != NULL; attr = attr->next)
                            {
                                if (attr->type == XML_ATTRIBUTE_NODE)
                                {
                                    const char* value;
                                    if (attr->children != NULL && attr->children->type == XML_TEXT_NODE)
                                    {
                                        value = reinterpret_cast<const char*>(attr->children->content);
                                    }
                                    else
                                    {
                                        value = "";
                                    }
                                    if (!strcmp(reinterpret_cast<const char*>(attr->name), "uuid"))
                                    {
                                        if (*value)
                                        {
                                            record.conflicting.push_back(Glib::ustring(value));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (!strcmp(reinterpret_cast<const char*>(node2->name), "requiredpatches"))
            {
                for (xmlNode* node3 = node2->children; node3 != NULL; node3 = node3->next)
                {
                    if (node3->type == XML_ELEMENT_NODE)
                    {
                        if (!strcmp(reinterpret_cast<const char*>(node3->name), "requiredpatch"))
                        {
                            for (xmlAttr* attr = node3->properties; attr != NULL; attr = attr->next)
                            {
                                if (attr->type == XML_ATTRIBUTE_NODE)
                                {
                                    const char* value;
                                    if (attr->children != NULL && attr->children->type == XML_TEXT_NODE)
                                    {
                                        value = reinterpret_cast<const char*>(attr->children->content);
                                    }
                                    else
                                    {
                                        value = "";
                                    }
                                    if (!strcmp(reinterpret_cast<const char*>(attr->name), "uuid"))
                                    {
                                        if (*value)
                                        {
                                            record.required.push_back(Glib::ustring(value));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}


static bool ParseServerVersion(xmlNode* node, PatchBase::ServerRecord& serverRecord)
{
    for (xmlAttr* attr = node->properties; attr != NULL; attr = attr->next)
    {
        if (attr->type == XML_ATTRIBUTE_NODE)
        {
            const char* value;
            if (attr->children != NULL && attr->children->type == XML_TEXT_NODE)
            {
                value = reinterpret_cast<const char*>(attr->children->content);
            }
            else
            {
                value = "";
            }
            if (!strcmp(reinterpret_cast<const char*>(attr->name), "build-number"))
            {
                serverRecord.build = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "latest"))
            {
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "name"))
            {
                serverRecord.name = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "timestamp"))
            {
                ParseTimestamp(value, serverRecord.timestamp);
#if 0
                struct tm result = { 0 };
                gmtime_r(&serverRecord.timestamp, &result);
                g_print("\n%s\n%04d-%02d-%02d*%02d:%02d:%02d*\n", value, result.tm_year + 1900, result.tm_mon, result.tm_mday, result.tm_hour, result.tm_min, result.tm_sec);
#endif
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "url"))
            {
                serverRecord.url = value;
            }
            else if (!strcmp(reinterpret_cast<const char*>(attr->name), "value"))
            {
                serverRecord.version = value;
            }
        }
    }

    if (serverRecord.version.bytes() == 0)
    {
        return false;
    }

    for (xmlNode* node2 = node->children; node2 != NULL; node2 = node2->next)
    {
        if (node2->type == XML_ELEMENT_NODE)
        {
            if (!strcmp(reinterpret_cast<const char*>(node2->name), "patch"))
            {
                for (xmlAttr* attr = node2->properties; attr != NULL; attr = attr->next)
                {
                    if (attr->type == XML_ATTRIBUTE_NODE)
                    {
                        const char* value;
                        if (attr->children != NULL && attr->children->type == XML_TEXT_NODE)
                        {
                            value = reinterpret_cast<const char*>(attr->children->content);
                        }
                        else
                        {
                            value = "";
                        }
                        if (!strcmp(reinterpret_cast<const char*>(attr->name), "uuid"))
                        {
                            if (*value)
                            {
                                serverRecord.patches.push_back(Glib::ustring(value));
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}


bool PatchBase::load()
{
    TRACE(StringBuffer().format("PatchBase@%zx::load", this));

    bool downloadRequired = true;

    struct stat statinfo = { 0 };
    if (!stat(_path.c_str(), &statinfo))
    {
        const time_t duration = 24 * 60 * 60;
        time_t expTime = statinfo.st_ctime + duration;
        time_t curTime = time(NULL);
        if (curTime <= expTime)
        {
            downloadRequired = false;
        }
    }

    if (downloadRequired)
    {
        if (!download())
        {
            return false;
        }
    }

    xmlDocPtr doc = xmlReadFile(_path.c_str(), NULL, 0);
    if (!doc)
    {
        return false;
    }
    xmlNode* root = xmlDocGetRootElement(doc);
    if (root && root->type == XML_ELEMENT_NODE)
    {
        for (xmlNode* cur = root->children; cur != NULL; cur = cur->next)
        {
            if (cur->type == XML_ELEMENT_NODE)
            {
                if (!strcmp(reinterpret_cast<const char*>(cur->name), "patches"))
                {
                    for (xmlNode* node = cur->children; node != NULL; node = node->next)
                    {
                        if (node->type == XML_ELEMENT_NODE)
                        {
                            if (!strcmp(reinterpret_cast<const char*>(node->name), "patch"))
                            {
                                RefPtr<PatchRecord> record = PatchRecord::create();
                                if (ParsePatch(node, *record))
                                {
                                    _recordMap.put(record);
                                }
                            }
                        }
                    }
                }
                else if (!strcmp(reinterpret_cast<const char*>(cur->name), "serverversions"))
                {
                    for (xmlNode* node = cur->children; node != NULL; node = node->next)
                    {
                        if (node->type == XML_ELEMENT_NODE)
                        {
                            if (!strcmp(reinterpret_cast<const char*>(node->name), "version"))
                            {
                                RefPtr<ServerRecord> serverRecord = ServerRecord::create();
                                if (ParseServerVersion(node, *serverRecord))
                                {
                                    _serverRecordMap.put(serverRecord);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    xmlFreeDoc(doc);

    Logger& log = Logger::instance();
    if (log.getLevel() >= LogLevel::DEBUG)
    {
        for (std::map<Glib::ustring, RefPtr<ServerRecord> >::const_iterator i = _serverRecordMap.begin(); i != _serverRecordMap.end(); i++)
        {
            RefPtr<ServerRecord> serverRecord = i->second;
            log.debug("%s: name=\"%s\" build=\"%s\"", serverRecord->version.c_str(), serverRecord->name.c_str(), serverRecord->build.c_str());
            for (std::list<Glib::ustring>::const_iterator j = serverRecord->patches.begin(); j != serverRecord->patches.end(); j++)
            {
                RefPtr<PatchRecord> record = _recordMap.get(*j);
                if (!record)
                {
                    log.debug("\t%s: MISSING!", j->c_str());
                    continue;
                }
                log.debug("\t%s: label=\"%s\" desc=\"%s\" url=\"%s\"", record->uuid.c_str(), record->label.c_str(), record->description.c_str(), record->url.c_str());
                for (std::list<Glib::ustring>::const_iterator jj = record->conflicting.begin(); jj != record->conflicting.end(); jj++)
                {
                    RefPtr<PatchRecord> record2 = _recordMap.get(*jj);
                    if (!record2)
                    {
                        log.debug("\t\tconflicting: %s: MISSING!", jj->c_str());
                        continue;
                    }
                    log.debug("\t\tconflicting: %s: label=\"%s\" desc=\"%s\"\n", record2->uuid.c_str(), record2->label.c_str(), record2->description.c_str());
                }
                for (std::list<Glib::ustring>::const_iterator jj = record->required.begin(); jj != record->required.end(); jj++)
                {
                    RefPtr<PatchRecord> record2 = _recordMap.get(*jj);
                    if (!record2)
                    {
                        log.debug("\t\trequired: %s: MISSING!", jj->c_str());
                        continue;
                    }
                    log.debug("\t\trequired: %s: label=\"%s\" desc=\"%s\"", record2->uuid.c_str(), record2->label.c_str(), record2->description.c_str());
                }
            }
        }
    }

    return true;
}


static size_t receive(void* ptr, size_t size, size_t nmemb, PatchBase* pThis)
{
    size_t len = size * nmemb;
    size_t ret = pThis->parse(ptr, len) ? len : 0;
    return ret;
}


bool PatchBase::download()
{
    TRACE(StringBuffer().format("PatchBase@%zx::download", this));

    bool retval = false;

    CURL* curl = NULL;

    try
    {
        Glib::ustring::size_type i = _path.rfind("/");
        if (i == Glib::ustring::npos)
        {
            throw "malformed path";
        }
        Glib::ustring dir(_path, 0, i);
        struct stat statinfo = { 0 };
        if (stat(dir.c_str(), &statinfo))
        {
            if (!mkdir(dir.c_str(), 0777))
            {
                TRACEPUT("dir={%s} created.", dir.c_str());
            }
            else
            {
                TRACEPUT("dir={%s} mkdir failed. error=%d", dir.c_str(), errno);
                throw "unable to create dir";
            }
        }
        else if (S_ISDIR(statinfo.st_mode))
        {
            TRACEPUT("dir={%s} exists.", dir.c_str());
        }
        else
        {
            TRACEPUT("dir={%s} is not a directory.", dir.c_str());
            throw "no such directory";
        }

        Glib::ustring tmp1 = Glib::ustring::compose("%1.tmp1", _path);
        Glib::ustring tmp2 = Glib::ustring::compose("%1.tmp2", _path);

        unlink(tmp1.c_str());
        unlink(tmp2.c_str());

        _fp = fopen(tmp1.c_str(), "w");
        _size = 0;

        if (!_fp)
        {
            throw "fopen";
        }

        curl = curl_easy_init();
        if (!curl)
        {
            throw "curl_easy_init";
        }

        Glib::ustring url("http://updates.xensource.com/XenServer/updates.xml");

        if (Logger::instance().getLevel() >= LogLevel::DEBUG)
        {
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            curl_easy_setopt(curl, CURLOPT_STDERR, stderr);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

        CURLcode result = curl_easy_perform(curl);

        fclose(_fp);
        _fp = 0;

        if (result != CURLE_OK)
        {
            Logger::instance().info("CURL: %d (%s)", (int)result, curl_easy_strerror(result));
            throw "curl_easy_perform";
        }

        if (rename(_path.c_str(), tmp2.c_str()))
        {
            if (errno != ENOENT)
            {
                Logger::instance().debug("PatchBase::download: rename(%s,%s) failed.", _path.c_str(), tmp2.c_str());
                throw "rename(1)";
            }
        }

        if (rename(tmp1.c_str(), _path.c_str()))
        {
            Logger::instance().debug("PatchBase::download: rename(%s,%s) failed.", tmp1.c_str(), _path.c_str());
            throw "rename(2)";
        }

        unlink(tmp2.c_str());

        Logger::instance().info("%s\t%'zu bytes downloaded.", _path.c_str(), _size);

        retval = true;
    }
    catch (...)
    {
    }

    if (curl)
    {
        curl_easy_cleanup(curl);
    }

    if (_fp)
    {
        fclose(_fp);
        _fp = 0;
    }

    return retval;
}


bool PatchBase::parse(void* ptr, size_t len)
{
    if (len)
    {
        size_t out = fwrite(ptr, len, 1, _fp);
        if (!out)
        {
            return false;
        }
        _size += len;
    }
    else
    {
        Logger::instance().warn("PatchBase::parse: ZERO byte!");
    }

    return true;
}


PatchBase::RecordIterator PatchBase::getRecordIterator(const char* v)
{
    PatchBase::RecordIterator retval;
    Glib::ustring version(v);
    retval.patchBase = this;
    retval.serverRecord = _serverRecordMap.get(version);
    if (retval.serverRecord)
    {
        retval.iterator = retval.serverRecord->patches.begin();
    }
    return retval;
}


RefPtr<PatchRecord> PatchBase::getRecord(const Glib::ustring& uuid) const
{
    RefPtr<PatchRecord> record = _recordMap.get(uuid);
    return record;
}


PatchBase::ServerRecordMap::ServerRecordMap()
{
}


PatchBase::ServerRecordMap::~ServerRecordMap()
{
}


void PatchBase::ServerRecordMap::put(const RefPtr<ServerRecord>& serverRecord)
{
    std::map<Glib::ustring, RefPtr<ServerRecord> >::iterator i = find(serverRecord->version);
    if (i != end())
    {
        i->second = serverRecord;
    }
    else
    {
        insert(std::pair<Glib::ustring, RefPtr<ServerRecord> >(serverRecord->version, serverRecord));
    }
}


RefPtr<PatchBase::ServerRecord> PatchBase::ServerRecordMap::get(const Glib::ustring& key) const
{
    RefPtr<ServerRecord> serverRecord;
    std::map<Glib::ustring, RefPtr<ServerRecord> >::const_iterator i = find(key);
    if (i != end())
    {
        serverRecord = i->second;
    }
    return serverRecord;
}


PatchBase::RecordMap::RecordMap()
{
}


PatchBase::RecordMap::~RecordMap()
{
}


void PatchBase::RecordMap::put(const RefPtr<PatchRecord>& record)
{
    std::map<Glib::ustring, RefPtr<PatchRecord> >::iterator i = find(record->uuid);
    if (i != end())
    {
        i->second = record;
    }
    else
    {
        insert(std::pair<Glib::ustring, RefPtr<PatchRecord> >(record->uuid, record));
    }
}


RefPtr<PatchRecord> PatchBase::RecordMap::get(const Glib::ustring& key) const
{
    RefPtr<PatchRecord> record;
    std::map<Glib::ustring, RefPtr<PatchRecord> >::const_iterator i = find(key);
    if (i != end())
    {
        record = i->second;
    }
    return record;
}


PatchBase::RecordIterator::RecordIterator()
    : patchBase(NULL)
    , serverRecord()
    , iterator()
{
}


PatchBase::RecordIterator::RecordIterator(const RecordIterator& src)
    : patchBase(src.patchBase)
    , serverRecord(src.serverRecord)
    , iterator(src.iterator)
{
}


void PatchBase::RecordIterator::operator =(const RecordIterator& rhs)
{
    patchBase = rhs.patchBase;
    serverRecord = rhs.serverRecord;
    iterator = rhs.iterator;
}


RefPtr<PatchRecord> PatchBase::RecordIterator::next()
{
    RefPtr<PatchRecord> record;
    if (serverRecord && iterator != serverRecord->patches.end())
    {
        Glib::ustring uuid = *iterator;
        iterator++;
        record = patchBase->_recordMap.get(uuid);
    }
    return record;
}
