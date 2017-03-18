// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_THREADNAMEMAP_H
#define HNRT_THREADNAMEMAP_H


#include <glibmm.h>
#include <map>


namespace hnrt
{
    class ThreadNameMap
    {
    public:

        static void init();
        static void fini();
        static ThreadNameMap& instance();

        virtual ~ThreadNameMap();
        int count();
        Glib::ustring add(const char*);
        Glib::ustring add(const Glib::ustring&);
        void remove();
        Glib::ustring find();

    private:

        ThreadNameMap();
        ThreadNameMap(const ThreadNameMap&);
        void operator =(const ThreadNameMap&);

        Glib::Mutex _mutex;
        Glib::Thread* _mainThread;
        std::map<Glib::Thread*, Glib::ustring> _nameMap;
        std::map<Glib::ustring, int> _countMap;
    };
}


#endif //!HNRT_THREADNAMEMAP_H
