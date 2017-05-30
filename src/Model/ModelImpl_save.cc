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
            RefPtr<Json> json = Json::create(Json::OBJECT);
            json->set("version", 1);
            json->set("UI.width", _width);
            json->set("UI.height", _height);
            json->set("UI.pane1_width", _pane1Width);
            Json::Array& array1 = json->setArray("servers");
            std::list<RefPtr<Host> > hosts;
            get(hosts);
            for (std::list<RefPtr<Host> >::const_iterator iter = hosts.begin(); iter != hosts.end(); iter++)
            {
                const Session& session = (*iter)->getSession();
                const ConnectSpec& cs = session.getConnectSpec();
                array1.push_back(cs.toJson());
            }
            Json::Array& array2 = json->setArray("consoles");
            for (ConsoleMap::const_iterator iter = _consoleMap.begin(); iter != _consoleMap.end(); iter++)
            {
                const ConsoleInfo& info = iter->second;
                array2.push_back(info.toJson());
            }
            json->set("export.path", _exportVmPath);
            json->set("export.verify", _exportVmVerify);
            json->set("import.path", _importVmPath);
            json->set("verify.path", _verifyVmPath);
            json->set("web_browser.path", _webBrowserPath);
            json->save(fp);
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
