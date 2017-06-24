// Copyright (C) 2017 Hideaki Narita


#include "File/File.h"
#include "Logger/Trace.h"
#include "Model/PatchRecord.h"
#include "Protocol/ThinClientInterface.h"
#include "Patch.h"
#include "Session.h"


using namespace hnrt;


RefPtr<Patch> Patch::create(Session& session, const RefPtr<PatchRecord>& record)
{
    return RefPtr<Patch>(new Patch(session, record));
}


Patch::Patch(Session& session, const RefPtr<PatchRecord>& record)
    : XenObject(XenObject::PATCH, session, NULL, record->uuid.c_str(), record->label.c_str())
    , _record(record)
{
    TRACEFUN(this, "Patch::ctor");
}


Patch::~Patch()
{
    TRACEFUN(this, "Patch::dtor");
}


void Patch::init()
{
    TRACEFUN(this, "Patch::init");
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
    _output.clear();
    _errorOutput.clear();
    _exitCode = -1;
    emit(XenObject::CREATED);
}


void Patch::fini()
{
    TRACEFUN(this, "Patch::fini");
    _cli->fini();
    _cli.reset();
    emit(XenObject::DESTROYED);
}


RefPtr<PatchRecord> Patch::getRecord() const
{
    return _record;
}


bool Patch::upload()
{
    TRACEFUN(this, "Patch::upload");

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
    TRACEFUN(this, "Patch@%zx::apply");

    _record->state = PatchState::APPLY_INPROGRESS;

    emit(XenObject::PATCH_APPLY_PENDING);

    bool retval = _cli->run("update-pool-apply",
                            Glib::ustring::compose("uuid=%1", _uuid).c_str(),
                            NULL);

    _record->state = retval ? PatchState::APPLIED : PatchState::APPLY_FAILURE;

    TRACEPUT("return=%s", retval ? "true" : "false");

    return retval;
}


Glib::ustring Patch::getOutput()
{
    Glib::Mutex::Lock lock(_mutex);
    return _output;
}


Glib::ustring Patch::getErrorOutput()
{
    Glib::Mutex::Lock lock(_mutex);
    return _errorOutput;
}


int Patch::getExitCode()
{
    Glib::Mutex::Lock lock(_mutex);
    return _exitCode;
}


void Patch::print(ThinClientInterface& cli)
{
    {
        Glib::Mutex::Lock lock(_mutex);
        if (!_output.empty())
        {
            _output.append("\n");
        }
        _output.append(cli.getOutput());
    }
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
    {
        Glib::Mutex::Lock lock(_mutex);
        if (!_errorOutput.empty())
        {
            _errorOutput.append("\n");
        }
        _errorOutput.append(cli.getErrorOutput());
    }
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
    {
        Glib::Mutex::Lock lock(_mutex);
        _exitCode = cli.getExitCode();
    }
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
