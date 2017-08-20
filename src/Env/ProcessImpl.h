// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PROCESSIMPL_H
#define HNRT_PROCESSIMPL_H


#include "Process.h"


namespace hnrt
{
    class ProcessImpl
        : public Process
    {
    public:

        ProcessImpl();
        virtual ~ProcessImpl();
        virtual Glib::ustring getExecutablePath() const { return _executablePath; }
        virtual Glib::ustring getExecutableDirectory() const;

    private:

        ProcessImpl(const ProcessImpl&);
        void operator =(const ProcessImpl&);

        Glib::ustring _executablePath;
    };
}


#endif //!HNRT_PROCESSIMPL_H
