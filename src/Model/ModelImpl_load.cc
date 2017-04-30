// Copyright (C) 2012-2017 Hideaki Narita


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "File/Json.h"
#include "Logger/Trace.h"
#include "Net/Console.h"
#include "View/View.h"
#include "ConnectSpec.h"
#include "ModelImpl.h"
#include "PatchBase.h"


using namespace hnrt;


void ModelImpl::load()
{
    Trace trace("ModelImpl::load");

    FILE* fp = NULL;

    try
    {
        fp = fopen(_path.c_str(), "r");
        if (!fp)
        {
            if (errno == ENOENT)
            {
                Logger::instance().warn(Glib::ustring::compose("%1: %2", _path, strerror(ENOENT)).c_str());
                View::instance().showWarning(Glib::ustring::compose("%1:\n\n%2", _path, strerror(ENOENT)));
                return;
            }
            else
            {
                throw Glib::ustring::compose("Unable to open %1: %2", _path, strerror(errno));
            }
        }
        try
        {
            Json json;
            json.load(fp);
            int version = 0;
            if (json.get("version", version))
            {
                trace.put("version=%d", version);
                if (version == 1)
                {
                    loadV1(json);
                }
            }
        }
        catch (Glib::ustring msg)
        {
            throw Glib::ustring::compose("%1: %2", _path, msg);
        }
        fclose(fp);
        fp = NULL;
        _patchBase->load();
    }
    catch (Glib::ustring msg)
    {
        if (fp)
        {
            fclose(fp);
        }
        throw;
    }
}


void ModelImpl::loadV1(const Json& json)
{
    Trace trace("ModelImpl::loadV1");

    json.get("UI.width", _width);
    json.get("UI.height", _height);
    json.get("UI.pane1_width", _pane1Width);

    json.get("servers", sigc::mem_fun(*this, &ModelImpl::loadV1Server));

    json.get("consoles", sigc::mem_fun(*this, &ModelImpl::loadV1Console));

    json.get("export.path", _exportVmPath);
    json.get("export.verify", _exportVmVerify);
    json.get("import.path", _importVmPath);
}


void ModelImpl::loadV1Server(const RefPtr<Json::Value>& value)
{
    Trace trace("ModelImpl::loadV1Server");

    ConnectSpec cs;
    Glib::ustring mac;
    if (value->get("uuid", cs.uuid) &&
        value->get("display_name", cs.displayname) &&
        value->get("host", cs.hostname) &&
        value->get("user", cs.username) &&
        value->get("password", cs.password) &&
        value->get("last_access", cs.lastAccess) &&
        value->get("auto_connect", cs.autoConnect) &&
        value->get("mac", mac) &&
        value->get("display_order", cs.displayOrder))
    {
        cs.mac.parse(mac.c_str());
        add(cs);
    }
}


void ModelImpl::loadV1Console(const RefPtr<Json::Value>& value)
{
    Trace trace("ModelImpl::loadV1Console");

    Glib::ustring uuid;
    if (value->get("uuid", uuid))
    {
        ConsoleInfo info(uuid);
        value->get("enabled", info.enabled);
        value->get("scale", info.scale);
        _consoleMap.insert(ConsoleEntry(info.uuid, info));
    }
}
