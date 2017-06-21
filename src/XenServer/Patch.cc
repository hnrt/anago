// Copyright (C) 2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "File/File.h"
#include "Logger/Trace.h"
#include "Model/PatchRecord.h"
#include "Protocol/ThinClientInterface.h"
#include "Patch.h"
#include "Session.h"
#include "XenObjectStore.h"


using namespace hnrt;


RefPtr<Patch> Patch::create(Session& session, const RefPtr<PatchRecord>& record)
{
    return RefPtr<Patch>(new Patch(session, record));
}


Patch::Patch(Session& session, const RefPtr<PatchRecord>& record)
    : XenObject(XenObject::PATCH, session, NULL, record->uuid.c_str(), record->label.c_str())
    , _record(record)
{
    TRACE(StringBuffer().format("Patch@%zx::ctor", this));
}


Patch::~Patch()
{
    TRACE(StringBuffer().format("Patch@%zx::dtor", this));
}


void Patch::init()
{
    const ConnectSpec& cs = _session.getConnectSpec();
    Glib::ustring pw = cs.descramblePassword();
    _cli = ThinClientInterface::create();
    _cli->init();
    _cli->setHostname(cs.hostname.c_str());
    _cli->setUsername(cs.username.c_str());
    _cli->setPassword(pw.c_str());
    _cli->setTimeout(600000L);
    _cli->setPrintCallback(sigc::mem_fun(*this, &Patch::print));
    _cli->setPrintErrorCallback(sigc::mem_fun(*this, &Patch::printError));
    _cli->setExitCallback(sigc::mem_fun(*this, &Patch::exit));
    pw.clear();
    RefPtr<Patch> object(this, 1);
    _session.getStore().add(object);
}


void Patch::fini()
{
    RefPtr<Patch> object(this, 1);
    _session.getStore().remove(_uuid, _type);
    _cli->fini();
    _cli.reset();
}


RefPtr<PatchRecord> Patch::getRecord() const
{
    return _record;
}


bool Patch::upload()
{
    TRACE(StringBuffer().format("Patch@%zx::upload", this));

    RefPtr<File> file = _record->getFile();
    if (!file)
    {
        Logger::instance().warn("%s: No patch file was found. Treated as upload failure.", _record->label.c_str());
        _record->state = PatchState::UPLOAD_FAILURE;
        emit(XenObject::PATCH_UPLOAD_FILE_ERROR);
        return false;
    }

    _path = file->path();

    TRACEPUT("path=\"%s\"", file->path());

    _record->state = PatchState::UPLOAD_INPROGRESS;

    emit(XenObject::PATCH_UPLOAD_PENDING);

    bool retval = _cli->run("update-upload",
                            Glib::ustring::compose("file-name=%1", _path).c_str(),
                            NULL);

    _record->state = retval ? PatchState::UPLOADED : PatchState::UPLOAD_FAILURE;

    TRACEPUT("return=%s", retval ? "true" : "false");

    return retval;
}


bool Patch::apply()
{
    TRACE(StringBuffer().format("Patch@%zx::apply", this));

    _record->state = PatchState::APPLY_INPROGRESS;

    emit(XenObject::PATCH_APPLY_PENDING);

    bool retval = _cli->run("update-pool-apply",
                            Glib::ustring::compose("uuid=%1", _uuid).c_str(),
                            NULL);

    _record->state = retval ? PatchState::APPLIED : PatchState::APPLY_FAILURE;

    TRACEPUT("return=%s", retval ? "true" : "false");

    return retval;
}


const Glib::ustring& Patch::getOutput() const
{
    return _cli->getOutput();
}


const Glib::ustring& Patch::getErrorOutput() const
{
    return _cli->getErrorOutput();
}


int Patch::getExitCode() const
{
    return _cli->getExitCode();
}


void Patch::print(ThinClientInterface& cli)
{
    switch (_record->state)
    {
    case PatchState::UPLOAD_INPROGRESS:
        emit(XenObject::PATCH_UPLOAD_PRINT);
        break;
    case PatchState::APPLY_INPROGRESS:
        emit(XenObject::PATCH_APPLY_PRINT);
        break;
    default:
        break;
    }
}


void Patch::printError(ThinClientInterface& cli)
{
    switch (_record->state)
    {
    case PatchState::UPLOAD_INPROGRESS:
        emit(XenObject::PATCH_UPLOAD_PRINT_ERROR);
        break;
    case PatchState::APPLY_INPROGRESS:
        emit(XenObject::PATCH_APPLY_PRINT_ERROR);
        break;
    default:
        break;
    }
}


void Patch::exit(ThinClientInterface& cli)
{
    switch (_record->state)
    {
    case PatchState::UPLOAD_INPROGRESS:
        emit(XenObject::PATCH_UPLOAD_EXIT);
        break;
    case PatchState::APPLY_INPROGRESS:
        emit(XenObject::PATCH_APPLY_EXIT);
        break;
    default:
        break;
    }
}
