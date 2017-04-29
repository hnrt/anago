// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <map>
#include <glibmm/ustring.h>
#include <openssl/sha.h>
#include "Base/StringBuffer.h"
#include "Logger/Logger.h"
#include "VirtualMachineArchive.h"
#include "XenObject.h"


using namespace hnrt;


struct Header
{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
};


static bool IsZeroBlock(const void* ptr)
{
    const char* cur = (const char*)ptr;
    const char* end = cur + 512;
    while (cur < end)
    {
        if (*cur)
        {
            return false;
        }
        cur++;
    }
    return true;
}


RefPtr<File> VirtualMachineArchive::create(const char* path, const char* mode, XenObject& owner)
{
    return RefPtr<File>(new VirtualMachineArchive(path, mode, owner));
}


VirtualMachineArchive::VirtualMachineArchive(const char* path, const char* mode, XenObject& owner)
    : FileImpl(path, mode)
    , _owner(owner)
{
}


VirtualMachineArchive::~VirtualMachineArchive()
{
}


bool VirtualMachineArchive::validate(int& percent, bool& abortFlag)
{
    percent = 0;

    time_t lastUpdated = 0;

    if (!_fp)
    {
        _error = EBADF;
        return false;
    }

    size_t nbytesExpected = size();

    char buf[512 * 16];
    size_t len;

    std::map<Glib::ustring, Glib::ustring> csMap;

    static const char csSuffix[] = { ".checksum" };
    const size_t csSuffixLen = strlen(csSuffix);

    for (;;)
    {
        if (abortFlag)
        {
            _error = ECANCELED;
            return false;
        }

        time_t now = time(NULL);
        if (lastUpdated < now)
        {
            lastUpdated = now;
            _owner.emit(XenObject::VERIFYING);
        }

        len = read(buf, 512);
        percent = 100 * _nbytes / nbytesExpected;
        if (len != 512)
        {
            if (!ferror(_fp))
            {
                _error = EPROTO;
            }
            return false;
        }

        if (IsZeroBlock(buf))
        {
            break;
        }

        Header header;
        memcpy(&header, buf, sizeof(header));

        size_t nameLen = strlen(header.name);
        size_t dataLen = strtoul(header.size, NULL, 8);

        if (nameLen > csSuffixLen && !strcmp(header.name + nameLen - csSuffixLen, csSuffix))
        {
            // checksum data

            Glib::ustring basename(header.name, nameLen - csSuffixLen);
            std::map<Glib::ustring, Glib::ustring>::iterator iter = csMap.find(basename);
            if (iter == csMap.end())
            {
                _error = EPROTO;
                return false;
            }

            Glib::ustring digest = iter->second;
            size_t digestLen = digest.bytes();

            if (dataLen < digestLen)
            {
                _error = EPROTO;
                return false;
            }

            if (dataLen > 512)
            {
                _error = EPROTO;
                return false;
            }

            if (abortFlag)
            {
                _error = ECANCELED;
                return false;
            }

            len = read(buf, 512);
            percent = 100 * _nbytes / nbytesExpected;
            if (len != 512)
            {
                if (!ferror(_fp))
                {
                    _error = EPROTO;
                }
                return false;
            }

            if (strncasecmp(buf, digest.c_str(), digestLen))
            {
                _error = EPROTO;
                return false;
            }

            csMap.erase(iter);

            continue;
        }

        SHA_CTX ctx;
        if (!SHA1_Init(&ctx))
        {
            Logger::instance().error("SHA1_Init failed.");
            _error = EINVAL;
            return false;
        }

        while (dataLen >= 512)
        {
            if (abortFlag)
            {
                _error = ECANCELED;
                return false;
            }

            size_t readLen = dataLen >= sizeof(buf) ? sizeof(buf) : (dataLen / 512) * 512;
            len = read(buf, readLen);
            percent = 100 * _nbytes / nbytesExpected;
            if (len != readLen)
            {
                if (!ferror(_fp))
                {
                    _error = EPROTO;
                }
                return false;
            }

            if (!SHA1_Update(&ctx, buf, len))
            {
                Logger::instance().error("SHA1_Update failed.");
                _error = EINVAL;
                return false;
            }

            dataLen -= len;
        }

        if (dataLen)
        {
            if (abortFlag)
            {
                _error = ECANCELED;
                return false;
            }

            len = read(buf, 512);
            percent = 100 * _nbytes / nbytesExpected;
            if (len != 512)
            {
                if (!ferror(_fp))
                {
                    _error = EPROTO;
                }
                return false;
            }

            if (!SHA1_Update(&ctx, buf, dataLen))
            {
                Logger::instance().error("SHA1_Update failed.");
                _error = EINVAL;
                return false;
            }

            dataLen = 0;
        }

        unsigned char hash[SHA_DIGEST_LENGTH] = { 0 };
        if (!SHA1_Final(hash, &ctx))
        {
            Logger::instance().error("SHA1_Final failed.");
            _error = EINVAL;
            return false;
        }

        if (!strcmp(header.name, "ova.xml"))
        {
            continue;
        }

        StringBuffer sb;
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        {
            sb.appendFormat("%02x", hash[i]);
        }

        csMap.insert(std::pair<Glib::ustring, Glib::ustring>(Glib::ustring(header.name), Glib::ustring(sb.str())));
    }

    if (abortFlag)
    {
        _error = ECANCELED;
        return false;
    }

    len = read(buf, 512);
    percent = 100 * _nbytes / nbytesExpected;
    if (len != 512)
    {
        if (!ferror(_fp))
        {
            _error = EPROTO;
        }
        return false;
    }

    if (!IsZeroBlock(buf))
    {
        _error = EPROTO;
        return false;
    }

    if (csMap.size() != 0)
    {
        _error = EPROTO;
        return false;
    }

    return true;
}
