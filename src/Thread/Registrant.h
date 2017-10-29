// Copyright (C) 2017 Hideaki Narita


#ifndef HNRT_REGISTRANT_H
#define HNRT_REGISTRANT_H


#include <glibmm/ustring.h>
#include "ThreadManagerImpl.h"


namespace hnrt
{
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
}


#endif //!HNRT_REGISTRANT_H
