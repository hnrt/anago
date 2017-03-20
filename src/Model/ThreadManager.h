// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_THREADNAMEMAP_H
#define HNRT_THREADNAMEMAP_H


#include <glibmm.h>
#include <map>


namespace hnrt
{
    class ThreadManager
    {
    public:

        static void init();
        static void fini();
        static ThreadManager& instance();

        virtual ~ThreadManager();
        int count();
        bool isMain() const { return Glib::Thread::self() == _mainThread; }
        Glib::ustring add(const char*);
        Glib::ustring add(const Glib::ustring&);
        void remove();
        Glib::ustring find();

    private:

        ThreadManager();
        ThreadManager(const ThreadManager&);
        void operator =(const ThreadManager&);

        Glib::Mutex _mutex;
        Glib::Thread* _mainThread;
        std::map<Glib::Thread*, Glib::ustring> _nameMap;
        std::map<Glib::ustring, int> _countMap;
    };
}


#endif //!HNRT_THREADNAMEMAP_H
