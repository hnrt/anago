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
    Trace trace(NULL, "ModelImpl::load");

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
            RefPtr<Json> json = Json::load(fp);
            int version = 0;
            if (json->get("version", version))
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


void ModelImpl::loadV1(const RefPtr<Json>& json)
{
    Trace trace(NULL, "ModelImpl::loadV1");

    json->get("UI.width", _width);
    json->get("UI.height", _height);
    json->get("UI.pane1_width", _pane1Width);

    json->get("servers", sigc::mem_fun(*this, &ModelImpl::loadV1Server));

    json->get("consoles", sigc::mem_fun(*this, &ModelImpl::loadV1Console));

    json->get("export.path", _exportVmPath);
    json->get("export.verify", _exportVmVerify);
    json->get("import.path", _importVmPath);
    json->get("verify.path", _verifyVmPath);

    json->get("web_browser.path", _webBrowserPath);
}


void ModelImpl::loadV1Server(const RefPtr<Json>& value)
{
    Trace trace(NULL, "ModelImpl::loadV1Server");

    ConnectSpec cs;
    if (cs.fromJson(value))
    {
        add(cs);
    }
}


void ModelImpl::loadV1Console(const RefPtr<Json>& value)
{
    Trace trace(NULL, "ModelImpl::loadV1Console");

    ConsoleInfo info;
    if (info.fromJson(value))
    {
        _consoleMap.insert(ConsoleEntry(info.uuid, info));
    }
}
