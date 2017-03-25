// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_THREADMANAGERIMPL_H
#define HNRT_THREADMANAGERIMPL_H


#include <glibmm.h>
#include <map>
#include "ThreadManager.h"


namespace hnrt
{
    class ThreadManagerImpl
        : public ThreadManager
    {
    public:

        ThreadManagerImpl();
        ~ThreadManagerImpl();
        virtual int count() const;
        virtual bool isMain() const { return Glib::Thread::self() == _mainThread; }
        virtual const char* getName() const;
        virtual Glib::Thread* create(const sigc::slot<void>&, bool, const char*);

    private:

        ThreadManagerImpl(const ThreadManagerImpl&);
        void operator =(const ThreadManagerImpl&);
        Glib::ustring add(const Glib::ustring&);
        void remove();
        void run(sigc::slot<void>, Glib::ustring);

        class Registrant
        {
        public:

            Registrant(ThreadManagerImpl& threadManager, const Glib::ustring& name)
                : _threadManager(threadManager)
            {
                _threadManager.add(name);
            }

            ~Registrant()
            {
                _threadManager.remove();
            }

        private:

            ThreadManagerImpl& _threadManager;
        };

        Glib::Thread* _mainThread;
        Glib::Mutex _mutex;
        std::map<Glib::Thread*, Glib::ustring> _nameMap;
        std::map<Glib::ustring, int> _countMap;
    };
}


#endif //!HNRT_THREADMANAGERIMPL_H
