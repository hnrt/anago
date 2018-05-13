// Copyright (C) 2017-2018 Hideaki Narita


#include <libintl.h>
#include "File/File.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchRecord.h"
#include "Protocol/HttpClient.h"
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
    _cli->setPrintFunction(sigc::mem_fun(*this, &Patch::print));
    _cli->setPrintErrorFunction(sigc::mem_fun(*this, &Patch::printError));
    _cli->setExitFunction(sigc::mem_fun(*this, &Patch::exit));
    _cli->setProgressFunction(sigc::mem_fun(*this, &Patch::reportProgress));
    pw.clear();
    _output.clear();
    _errorOutput.clear();
    _exitCode = -1;
    emit(XenObject::CREATED);
}


void Patch::initDownload()
{
    TRACEFUN(this, "Patch::initDownload");
    Glib::ustring dir = Model::instance().getAppDir();
    Glib::ustring::size_type pos1 = _record->patchUrl.rfind('/');
    if (pos1 != Glib::ustring::npos)
    {
        Glib::ustring::size_type pos2 = _record->patchUrl.find('&', pos1 + 1);
        Glib::ustring::size_type len = (pos2 != Glib::ustring::npos) ? (pos2 - (pos1 + 1)) : Glib::ustring::npos;
        _path = Glib::ustring::compose("%1%2", dir, _record->patchUrl.substr(pos1 + 1, len));
    }
    else
    {
        _path = Glib::ustring::compose("%1%2.zip", dir, _record->label);
    }
    _httpClient = HttpClient::create();
    _httpClient->init();
    _output.clear();
    _errorOutput.clear();
    _exitCode = -1;
    emit(XenObject::CREATED);
}


void Patch::fini()
{
    TRACEFUN(this, "Patch::fini");
    if (_httpClient)
    {
        _httpClient->fini();
        _httpClient.reset();
    }
    if (_cli)
    {
        _cli->fini();
        _cli.reset();
    }
    emit(XenObject::DESTROYED);
}


RefPtr<PatchRecord> Patch::getRecord() const
{
    return _record;
}


bool Patch::download()
{
    TRACEFUN(this, "Patch::download");

    _expected = -1;
    _actual = 0;
    _updateAfter.now().addMilliseconds(UPDATE_INTERVAL);

    TRACEPUT("url=%s", _record->patchUrl.c_str());
    TRACEPUT("path=%s", _path.c_str());

    _record->state = PatchState::DOWNLOAD_INPROGRESS;

    emit(XenObject::PATCH_DOWNLOAD_PENDING);

    RefPtr<File> file = File::create(_path.c_str(), "w");

    if (!file->open())
    {
        Logger::instance().error("%s: %s", file->path(), strerror(file->error()));
        _record->state = PatchState::DOWNLOAD_FAILURE;
        _errorOutput = Glib::ustring::compose(gettext("Unable to open file.\n\n%1\n\n%2"), file->path(), strerror(file->error()));
        emit(XenObject::PATCH_DOWNLOAD_FAILED);
        return false;
    }

    _httpClient->setUrl(_record->patchUrl.c_str());
    _httpClient->setMethod(HttpClient::GET);
    _httpClient->followLocation();
    _httpClient->setWriteFunction(sigc::bind<RefPtr<File> >(sigc::mem_fun(*this, &Patch::write), file));

    bool result = _httpClient->run();

    file->close();

    if (!result)
    {
        _errorOutput.assign(gettext("Download failed."));
        emit(XenObject::PATCH_DOWNLOAD_FAILED);
        return false;
    }

    if (!file->exists())
    {
        Logger::instance().warn("%s: Not found. Download failed.", file->path());
        _record->state = PatchState::DOWNLOAD_FAILURE;
        _errorOutput = Glib::ustring::compose(gettext("File not exist.\n\n%1"), file->path());
        emit(XenObject::PATCH_DOWNLOAD_FAILED);
        return false;
    }
    else if (!file->size())
    {
        Logger::instance().warn("%s: Empty. Download failed.", file->path());
        _record->state = PatchState::DOWNLOAD_FAILURE;
        _errorOutput = Glib::ustring::compose(gettext("File is empty.\n\n%1"), file->path());
        emit(XenObject::PATCH_DOWNLOAD_FAILED);
        return false;
    }

    RefPtr<File> file2 = _record->unzipFile(_path);
    if (!file2)
    {
        Logger::instance().warn("%s: No patch file was found. Treated as download failure.", _path.c_str());
        _record->state = PatchState::DOWNLOAD_FAILURE;
        _errorOutput = Glib::ustring::compose(gettext("File not found.\n\n%1"), file2->path());
        emit(XenObject::PATCH_DOWNLOAD_FAILED);
        return false;
    }
    else if (!file2->size())
    {
        Logger::instance().warn("%s: Empty. Download failed.", file2->path());
        _record->state = PatchState::DOWNLOAD_FAILURE;
        _errorOutput = Glib::ustring::compose(gettext("File is empty.\n\n%1"), file2->path());
        emit(XenObject::PATCH_DOWNLOAD_FAILED);
        return false;
    }

    Logger::instance().info("%s: %'zu bytes extracted.", file2->path(), file2->size());

    _record->state = PatchState::DOWNLOADED;

    _output = Glib::ustring::compose(gettext("Downloaded successfully.\n\n%1"), file2->path());

    emit(XenObject::PATCH_DOWNLOADED);

    return true;
}


bool Patch::write(const void* ptr, size_t len, RefPtr<File> file)
{
    if (len)
    {
        if (_expected < 0)
        {
            _expected = static_cast<ssize_t>(_httpClient->getContentLength());
        }
        if (file->write(ptr, len))
        {
            Glib::Mutex::Lock lock(_mutex);
            _actual += len;
            TRACE("Patch@%zx::write(%zu,%s): total=%zu", this, len, file->path(), _actual);
        }
        else
        {
            TRACE("Patch@%zx::write(%zu,%s): Failed. total=%zu", this, len, file->path(), _actual);
            return false;
        }
        if (_updateAfter < Time().now())
        {
            _updateAfter.now().addMilliseconds(UPDATE_INTERVAL);
            emit(XenObject::PATCH_DOWNLOADING);
        }
    }
    return true;
}


bool Patch::upload()
{
    TRACEFUN(this, "Patch::upload");

    RefPtr<File> file = _record->getFile();
    if (!file)
    {
        Logger::instance().warn("%s: No patch file was found. Treated as upload failure.", _record->label.c_str());
        _record->state = PatchState::UPLOAD_FAILURE;
        _errorOutput.assign(gettext("No patch file was found."));
        emit(XenObject::PATCH_UPLOAD_FAILED);
        return false;
    }

    _path = file->path();

    TRACEPUT("path=\"%s\"", file->path());

    _expected = file->size();
    _actual = 0;
    _updateAfter.now().addMilliseconds(UPDATE_INTERVAL);

    _record->state = PatchState::UPLOAD_INPROGRESS;

    emit(XenObject::PATCH_UPLOAD_PENDING);

    bool retval = _cli->run("update-upload",
                            Glib::ustring::compose("file-name=%1", _path).c_str(),
                            NULL);

    _record->state = retval ? PatchState::UPLOADED : PatchState::UPLOAD_FAILURE;

    emit(retval ? XenObject::PATCH_UPLOADED : XenObject::PATCH_UPLOAD_FAILED);

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

    emit(retval ? XenObject::PATCH_APPLIED : XenObject::PATCH_APPLY_FAILED);

    TRACEPUT("return=%s", retval ? "true" : "false");

    return retval;
}


void Patch::print(const char* message)
{
    _output.append(message);
}


void Patch::printError(const char* message)
{
    _errorOutput.append(message);
}


void Patch::exit(int exitCode)
{
    _exitCode = exitCode;
}


void Patch::reportProgress(size_t nbytes)
{
    TRACEFUN(this, "Patch::reportProgress(%zu)", nbytes);
    switch (_record->state)
    {
    case PatchState::UPLOAD_INPROGRESS:
        _actual = nbytes;
        if (_updateAfter < Time().now())
        {
            _updateAfter.now().addMilliseconds(UPDATE_INTERVAL);
            emit(XenObject::PATCH_UPLOADING);
        }
        break;
    default:
        break;
    }
}
