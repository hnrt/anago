// Copyright (C) 2012-2017 Hideaki Narita


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "Logger/Trace.h"
#include "XenServer/Session.h"
#include "ConnectSpec.h"
#include "Json.h"
#include "ModelImpl.h"


using namespace hnrt;


void ModelImpl::save()
{
    Trace trace(__PRETTY_FUNCTION__);

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
            RefPtr<Json::Value> root(new Json::Value());
            RefPtr<Json::Object> object1(new Json::Object());
            object1->add("version", 1L);
            RefPtr<Json::Object> object2(new Json::Object());
            object2->add("width", (long)getWidth());
            object2->add("height", (long)getHeight());
            object2->add("pane1_width", (long)getPane1Width());
            object1->add("UI", object2);
            Json::Array array3;
            std::list<Session*> sessions;
            get(sessions);
            for (std::list<Session*>::const_iterator iter = sessions.begin(); iter != sessions.end(); iter++)
            {
                const Session* pSession = *iter;
                const ConnectSpec& cs = pSession->getConnectSpec();
                RefPtr<Json::Object> object4(new Json::Object());
                object4->add("uuid", cs.uuid.c_str());
                object4->add("display_name", cs.displayname.c_str());
                object4->add("host", cs.hostname.c_str());
                object4->add("user", cs.username.c_str());
                object4->add("password", cs.password.c_str());
                object4->add("last_access", cs.lastAccess);
                object4->add("auto_connect", cs.autoConnect);
                object4->add("mac", cs.mac.toString().c_str());
                object4->add("display_order", (long)cs.displayOrder);
                RefPtr<Json::Value> value4(new Json::Value());
                value4->set(object4);
                array3.push_back(value4);
            }
            object1->add("servers", array3);
            root->set(object1);
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
