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
            RefPtr<Json::Value> root = json.root();
            if (root.ptr() && root->type() == Json::OBJECT)
            {
                RefPtr<Json::Object> object1 = root->object();
                RefPtr<Json::Value> valueVersion = object1->get("version");
                if (valueVersion.ptr() && valueVersion->type() == Json::NUMBER)
                {
                    trace.put("version=%ld", valueVersion->integer());
                    if (valueVersion->integer() == 1L)
                    {
                        loadV1(json);
                    }
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

    RefPtr<Json::Object> object1 = json.root()->object();

    RefPtr<Json::Value> value1 = object1->get("UI");
    if (value1.ptr() && value1->type() == Json::OBJECT)
    {
        RefPtr<Json::Object> object2 = value1->object();
        RefPtr<Json::Value> value2 = object2->get("width");
        if (value2.ptr() && value2->type() == Json::NUMBER)
        {
            trace.put("width=%ld", value2->integer());
            setWidth((int)value2->integer());
        }
        value2 = object2->get("height");
        if (value2.ptr() && value2->type() == Json::NUMBER)
        {
            trace.put("height=%ld", value2->integer());
            setHeight((int)value2->integer());
        }
        value2 = object2->get("pane1_width");
        if (value2.ptr() && value2->type() == Json::NUMBER)
        {
            trace.put("pane1_width=%ld", value2->integer());
            setPane1Width((int)value2->integer());
        }
    }

    value1 = object1->get("servers");
    if (value1.ptr() && value1->type() == Json::ARRAY)
    {
        const Json::Array& array = value1->array();
        for (Json::Array::size_type index = 0; index < array.size(); index++)
        {
            RefPtr<Json::Value> value2 = array[index];
            if (value2.ptr() && value2->type() == Json::OBJECT)
            {
                RefPtr<Json::Object> object3 = value2->object();
                RefPtr<Json::Value> valueUuid = object3->get("uuid");
                RefPtr<Json::Value> valueDisp = object3->get("display_name");
                RefPtr<Json::Value> valueHost = object3->get("host");
                RefPtr<Json::Value> valueUser = object3->get("user");
                RefPtr<Json::Value> valuePass = object3->get("password");
                RefPtr<Json::Value> valueLast = object3->get("last_access");
                RefPtr<Json::Value> valueAuto = object3->get("auto_connect");
                RefPtr<Json::Value> valueMac = object3->get("mac");
                RefPtr<Json::Value> valueOrder = object3->get("display_order");
                if (valueUuid.ptr() && valueUuid->type() == Json::STRING
                    && valueDisp.ptr() && valueDisp->type() == Json::STRING
                    && valueHost.ptr() && valueHost->type() == Json::STRING
                    && valueUser.ptr() && valueUser->type() == Json::STRING
                    && valuePass.ptr() && valuePass->type() == Json::STRING
                    && valueLast.ptr() && valueLast->type() == Json::NUMBER
                    && valueAuto.ptr() && valueAuto->type() == Json::BOOLEAN
                    && valueMac.ptr() && valueMac->type() == Json::STRING
                    && valueOrder.ptr() && valueOrder->type() == Json::NUMBER)
                {
                    ConnectSpec cs;
                    cs.uuid = valueUuid->string();
                    cs.displayname = valueDisp->string();
                    cs.hostname = valueHost->string();
                    cs.username = valueUser->string();
                    cs.password = valuePass->string();
                    cs.lastAccess = valueLast->integer();
                    cs.autoConnect = valueAuto->boolean();
                    cs.mac.parse(valueMac->string().c_str());
                    cs.displayOrder = (int)valueOrder->integer();
                    add(cs);
                }
            }
        }
    }

    value1 = object1->get("consoles");
    if (value1.ptr() && value1->type() == Json::ARRAY)
    {
        const Json::Array& array = value1->array();
        for (Json::Array::size_type index = 0; index < array.size(); index++)
        {
            RefPtr<Json::Value> value2 = array[index];
            if (value2.ptr() && value2->type() == Json::OBJECT)
            {
                RefPtr<Json::Object> object3 = value2->object();
                RefPtr<Json::Value> valueUuid = object3->get("uuid");
                RefPtr<Json::Value> valueEnabled = object3->get("enabled");
                RefPtr<Json::Value> valueScale = object3->get("scale");
                if (valueUuid.ptr() && valueUuid->type() == Json::STRING)
                {
                    ConsoleInfo info(valueUuid->string());
                    if (valueEnabled.ptr() && valueEnabled->type() == Json::BOOLEAN)
                    {
                        info.enabled = valueEnabled->boolean();
                    }
                    if (valueScale.ptr() && valueScale->type() == Json::BOOLEAN)
                    {
                        info.scale = valueScale->boolean();
                    }
                    _consoleMap.insert(ConsoleEntry(info.uuid, info));
                }
            }
        }
    }
}
