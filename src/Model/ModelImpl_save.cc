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
            RefPtr<Json::Object> object1(new Json::Object());
            object1->add("version", 1L);

            RefPtr<Json::Object> object2(new Json::Object());
            object2->add("width", (long)getWidth());
            object2->add("height", (long)getHeight());
            object2->add("pane1_width", (long)getPane1Width());
            object1->add("UI", object2);

            Json::Array array3;
            std::list<RefPtr<Host> > hosts;
            get(hosts);
            for (std::list<RefPtr<Host> >::const_iterator iter = hosts.begin(); iter != hosts.end(); iter++)
            {
                const Session& session = (*iter)->getSession();
                const ConnectSpec& cs = session.getConnectSpec();
                RefPtr<Json::Object> object3(new Json::Object());
                object3->add("uuid", cs.uuid.c_str());
                object3->add("display_name", cs.displayname.c_str());
                object3->add("host", cs.hostname.c_str());
                object3->add("user", cs.username.c_str());
                object3->add("password", cs.password.c_str());
                object3->add("last_access", cs.lastAccess);
                object3->add("auto_connect", cs.autoConnect);
                object3->add("mac", cs.mac.toString().c_str());
                object3->add("display_order", (long)cs.displayOrder);
                array3.push_back(RefPtr<Json::Value>(new Json::Value(object3)));
            }
            object1->add("servers", array3);

            Json::Array array4;
            for (ConsoleMap::const_iterator iter = _consoleMap.begin(); iter != _consoleMap.end(); iter++)
            {
                const ConsoleInfo& info = iter->second;
                RefPtr<Json::Object> object4(new Json::Object());
                object4->add("uuid", info.uuid.c_str());
                object4->add("enabled", info.enabled);
                object4->add("scale", info.scale);
                array4.push_back(RefPtr<Json::Value>(new Json::Value(object4)));
            }
            object1->add("consoles", array4);

            RefPtr<Json::Object> object5(new Json::Object());
            if (!_exportVmPath.empty())
            {
                object5->add("path", _exportVmPath.c_str());
                object5->add("verify", _exportVmVerify);
            }
            object1->add("export", object5);

            RefPtr<Json::Value> root(new Json::Value(object1));

            Json json;
            json.set(root);
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
