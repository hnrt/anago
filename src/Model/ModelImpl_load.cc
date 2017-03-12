// Copyright (C) 2012-2017 Hideaki Narita


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "Base/RefObj.h"
#include "Base/RefPtr.h"
#include "Logger/Trace.h"
#include "View/View.h"
#include "Json.h"
#include "ModelImpl.h"


using namespace hnrt;


void ModelImpl::load()
{
    Trace trace(__PRETTY_FUNCTION__);

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
        }
        catch (Glib::ustring msg)
        {
            throw Glib::ustring::compose("%1: %2", _path, msg);
        }
        fclose(fp);
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
