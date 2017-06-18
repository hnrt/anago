// Copyright (C) 2012-2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "Protocol/ThinClientInterface.h"
#include "XenServer/Session.h"
#include "PatchUploader.h"


using namespace hnrt;


RefPtr<PatchUploader> PatchUploader::create()
{
    return RefPtr<PatchUploader>(new PatchUploader);
}


PatchUploader::PatchUploader()
{
    TRACE("PatchUploader::ctor");
}


PatchUploader::~PatchUploader()
{
    TRACE("PatchUploader::dtor");
}


bool PatchUploader::run(Session& session, const char* path)
{
    TRACE("PatchUploader::run", "path=\"%s\"", path);

    const ConnectSpec& cs = session.getConnectSpec();
    Glib::ustring pw = cs.descramblePassword();

    RefPtr<ThinClientInterface> cli = ThinClientInterface::create();

    cli->init();
    cli->setHostname(cs.hostname.c_str());
    cli->setUsername(cs.username.c_str());
    cli->setPassword(pw.c_str());
    cli->setTimeout(600000L);

    bool retval = cli->run("update-upload",
                           StringBuffer().format("file-name=%s", path).str(),
                           NULL);

    pw.clear();

    TRACEPUT("return=%s", retval ? "true" : "false");

    return retval;
}
