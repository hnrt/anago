// Copyright (C) 2017 Hideaki Narita


#include "Base/StringBuffer.h"
#include "ThreadNameMap.h"


using namespace hnrt;


static ThreadNameMap* _singleton;


void ThreadNameMap::init()
{
    _singleton = new ThreadNameMap();
}


void ThreadNameMap::fini()
{
    delete _singleton;
}


ThreadNameMap& ThreadNameMap::instance()
{
    return *_singleton;
}


ThreadNameMap::ThreadNameMap()
    : _mainThread(Glib::Thread::self())
{
}


ThreadNameMap::~ThreadNameMap()
{
}


int ThreadNameMap::count()
{
    Glib::Mutex::Lock lock(_mutex);
    return static_cast<int>(_nameMap.size());
}


Glib::ustring ThreadNameMap::add(const char* basename)
{
    Glib::ustring key(basename);
    return add(key);
}


Glib::ustring ThreadNameMap::add(const Glib::ustring& basename)
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
            g_printerr("ERROR: ThreadNameMap::add(%zx,%s): Entry just inserted not found.\n", (size_t)thread, basename.c_str());
            return basename;
        }
    }
    int& count = iter2->second;
    count++;
    Glib::ustring name(StringBuffer().format("%s-%d", basename.c_str(), count));
    _nameMap.insert(std::pair<Glib::Thread*, Glib::ustring>(thread, name));
    return name;
}


void ThreadNameMap::remove()
{
    Glib::Thread* thread = Glib::Thread::self();
    if (thread == _mainThread)
    {
        return;
    }
    Glib::Mutex::Lock lock(_mutex);
    std::map<Glib::Thread*, Glib::ustring>::iterator iter = _nameMap.find(thread);
    if (iter != _nameMap.end())
    {
        _nameMap.erase(iter);
    }
}


Glib::ustring ThreadNameMap::find()
{
    Glib::Thread* thread = Glib::Thread::self();
    if (thread == _mainThread)
    {
        return Glib::ustring("Main");
    }
    Glib::Mutex::Lock lock(_mutex);
    std::map<Glib::Thread*, Glib::ustring>::const_iterator iter = _nameMap.find(thread);
    if (iter != _nameMap.end())
    {
        return iter->second;
    }
    else
    {
        return Glib::ustring(StringBuffer().format("%zx", thread));
    }
}
