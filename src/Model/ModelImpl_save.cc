// Copyright (C) 2012-2017 Hideaki Narita


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "Logger/Trace.h"
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
            RefPtr<Json::Object> toplevel(new Json::Object());
            json.set(toplevel);
            toplevel->add("version", 1L);
            toplevel->add("foo", 1L);
            toplevel->add("bar", true);
            toplevel->add("baz", "quux");
            RefPtr<Json::Object> sub1(new Json::Object());
            sub1->add("width", 100L);
            sub1->add("height", 100L);
            toplevel->add("ui", sub1);
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
