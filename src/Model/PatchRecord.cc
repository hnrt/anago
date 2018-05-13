// Copyright (C) 2012-2018 Hideaki Narita


#include <stdlib.h>
#include "File/File.h"
#include "Logger/Logger.h"
#include "Model/Model.h"
#include "PatchRecord.h"


using namespace hnrt;


static const char* s_extensions[2] =
{
    ".iso",
    ".xsupdate"
};


RefPtr<File> PatchRecord::unzipFile(const Glib::ustring& zipPath) const
{
    RefPtr<File> file;
    const char* dir = Model::instance().getAppDir();
    for (unsigned i = 0; i < sizeof(s_extensions) / sizeof(s_extensions[0]); i++)
    {
        Glib::ustring filename = Glib::ustring::compose("%1%2", label, s_extensions[i]);
        Glib::ustring cmd = Glib::ustring::compose("unzip \"%1\" \"%2\" -d \"%3\"", zipPath, filename, dir);
        Logger::instance().info("%s", cmd.c_str());
        system(cmd.c_str());
        Glib::ustring path = Glib::ustring::compose("%1%2%3", dir, label, s_extensions[i]);
        file = File::create(path.c_str());
        if (file->exists())
        {
            return file;
        }
        Logger::instance().info("%s: Not found.", file->path());
    }
    file.reset();
    return file;
}


RefPtr<File> PatchRecord::getFile() const
{
    RefPtr<File> file;
    const char* dir = Model::instance().getAppDir();
    for (unsigned i = 0; i < sizeof(s_extensions) / sizeof(s_extensions[0]); i++)
    {
        Glib::ustring path = Glib::ustring::compose("%1%2%3", dir, label, s_extensions[i]);
        file = File::create(path.c_str());
        if (file->exists())
        {
            return file;
        }
    }
    file.reset();
    return file;
}
