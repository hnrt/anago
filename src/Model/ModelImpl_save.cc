// Copyright (C) 2012-2017 Hideaki Narita


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "File/Json.h"
#include "Logger/Trace.h"
#include "XenServer/Host.h"
#include "XenServer/Session.h"
#include "ConnectSpec.h"
#include "ModelImpl.h"


using namespace hnrt;


void ModelImpl::save()
{
    Trace trace("ModelImpl::save");

    FILE* fp = NULL;

    try
    {
        Glib::ustring tmp = _path + ".tmp";
        fp = fopen(tmp.c_str(), "w");
        if (!fp)
        {
            throw Glib::ustring::compose("Unable to open %1: %2", tmp, strerror(errno));
        }
        try
        {
            Json json;
            json.setInt("version", 1);
            json.setInt("UI.width", _width);
            json.setInt("UI.height", _height);
            json.setInt("UI.pane1_width", _pane1Width);
            Json::Array& array1 = json.addArray("servers");
            std::list<RefPtr<Host> > hosts;
            get(hosts);
            for (std::list<RefPtr<Host> >::const_iterator iter = hosts.begin(); iter != hosts.end(); iter++)
            {
                const Session& session = (*iter)->getSession();
                const ConnectSpec& cs = session.getConnectSpec();
                RefPtr<Json::Object> object(new Json::Object());
                object->add("uuid", cs.uuid.c_str());
                object->add("display_name", cs.displayname.c_str());
                object->add("host", cs.hostname.c_str());
                object->add("user", cs.username.c_str());
                object->add("password", cs.password.c_str());
                object->add("last_access", cs.lastAccess);
                object->add("auto_connect", cs.autoConnect);
                object->add("mac", cs.mac.toString().c_str());
                object->add("display_order", (long)cs.displayOrder);
                array1.push_back(RefPtr<Json::Value>(new Json::Value(object)));
            }
            Json::Array& array2 = json.addArray("consoles");
            for (ConsoleMap::const_iterator iter = _consoleMap.begin(); iter != _consoleMap.end(); iter++)
            {
                const ConsoleInfo& info = iter->second;
                RefPtr<Json::Object> object(new Json::Object());
                object->add("uuid", info.uuid.c_str());
                object->add("enabled", info.enabled);
                object->add("scale", info.scale);
                array2.push_back(RefPtr<Json::Value>(new Json::Value(object)));
            }
            json.setString("export.path", _exportVmPath);
            json.setBoolean("export.verify", _exportVmVerify);
            json.setString("import.path", _importVmPath);
            json.save(fp);
        }
        catch (Glib::ustring msg)
        {
            throw Glib::ustring::compose("%1: %2", tmp, msg);
        }
        fclose(fp);
        fp = NULL;
        if (rename(tmp.c_str(), _path.c_str()))
        {
            throw Glib::ustring::compose("Unable to rename %1 to %2: %3", tmp, _path, strerror(errno));
        }
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
