// Copyright (C) 2018 Hideaki Narita


#include <stdexcept>
#include "Base/StringBuffer.h"
#include "Registrant.h"
#include "ThreadManagerImpl.h"


using namespace hnrt;


ThreadManagerImpl::ThreadManagerImpl()
    : _mainThread(Glib::Thread::self())
{
}


ThreadManagerImpl::~ThreadManagerImpl()
{
}


int ThreadManagerImpl::count() const
{
    Glib::Mutex::Lock lock(const_cast<ThreadManagerImpl*>(this)->_mutex);
    return static_cast<int>(_nameMap.size());
}


Glib::ustring ThreadManagerImpl::add(const Glib::ustring& basename)
{
    Glib::Thread* thread = Glib::Thread::self();
    Glib::Mutex::Lock lock(_mutex);
    std::map<Glib::Thread*, Glib::ustring>::const_iterator iter = _nameMap.find(thread);
    if (iter != _nameMap.end())
    {
        return iter->second;
    }
    std::map<Glib::ustring, int>::iterator iter2 = _countMap.find(basename);
    if (iter2 == _countMap.end())
    {
        _countMap.insert(std::pair<Glib::ustring, int>(basename, 0));
        iter2 = _countMap.find(basename);
        if (iter2 == _countMap.end())
        {
            g_printerr("ERROR: ThreadManagerImpl::add(%zx,%s): Entry just inserted not found.\n", (size_t)thread, basename.c_str());
            return basename;
        }
    }
    int& count = iter2->second;
    count++;
    Glib::ustring name(StringBuffer().format("%s-%d", basename.c_str(), count));
    _nameMap.insert(std::pair<Glib::Thread*, Glib::ustring>(thread, name));
    return name;
}


void ThreadManagerImpl::remove()
{
    Glib::Thread* thread = Glib::Thread::self();
    Glib::Mutex::Lock lock(_mutex);
    std::map<Glib::Thread*, Glib::ustring>::iterator iter = _nameMap.find(thread);
    if (iter != _nameMap.end())
    {
        _nameMap.erase(iter);
    }
}


const char* ThreadManagerImpl::getName() const
{
    Glib::Thread* thread = Glib::Thread::self();
    if (thread == _mainThread)
    {
        return "Main";
    }
    Glib::Mutex::Lock lock(const_cast<ThreadManagerImpl*>(this)->_mutex);
    std::map<Glib::Thread*, Glib::ustring>::const_iterator iter = _nameMap.find(thread);
    if (iter != _nameMap.end())
    {
        return iter->second.c_str();
    }
    else
    {
        StringBuffer msg;
        msg.format("ThreadManagerImpl::getName: Thread[%zx] not found.", thread);
        g_printerr("ERROR: %s\n", msg.str());
        throw std::runtime_error(msg.str());
    }
}


Glib::Thread* ThreadManagerImpl::create(const sigc::slot<void>& slot, bool joinable, const char* name)
{
    return Glib::Thread::create(sigc::bind<sigc::slot<void>, Glib::ustring>(sigc::mem_fun(*this, &ThreadManagerImpl::run), slot, Glib::ustring(name)), joinable);
}


void ThreadManagerImpl::run(sigc::slot<void> slot, Glib::ustring name)
{
    Registrant registrant(*this, name);
    try
    {
        slot();
    }
    catch (...)
    {
        throw;
    }
}
