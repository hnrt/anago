// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODEL_H
#define HNRT_MODEL_H


#include <glibmm/ustring.h>
#include <list>
#include "Base/RefPtr.h"


namespace hnrt
{
    class Console;
    class Host;
    class PatchBase;
    class Session;
    class StorageRepository;
    class VirtualMachine;
    class XenObject;
    struct ConnectSpec;

    class Model
    {
    public:

        static void init();
        static void fini();
        static Model& instance();

        virtual void load() = 0;
        virtual void save() = 0;
        virtual void clear() = 0;
        virtual int get(std::list<RefPtr<Host> >&) = 0;
        virtual void add(const ConnectSpec&) = 0;
        virtual void remove(Session&) = 0;
        virtual void removeAllSessions() = 0;

        virtual void deselectAll() = 0;
        virtual void select(const RefPtr<XenObject>&) = 0;
        virtual int getSelected(std::list<RefPtr<Host> >&) = 0;
        virtual int getSelected(std::list<RefPtr<VirtualMachine> >&) = 0;
        virtual int getSelected(std::list<RefPtr<StorageRepository> >&) = 0;
        virtual RefPtr<Host> getSelectedHost() = 0;
        virtual RefPtr<VirtualMachine> getSelectedVm() = 0;
        virtual RefPtr<StorageRepository> getSelectedSr() = 0;

        virtual void selectSnapshot(const RefPtr<VirtualMachine>&) = 0;
        virtual void deselectSnapshot() = 0;

        virtual RefPtr<PatchBase> getPatchBase() = 0;

        virtual void addConsole(const Glib::ustring&, const RefPtr<Console>&) = 0;
        virtual void removeConsole(const Glib::ustring&) = 0;
        virtual RefPtr<Console> getConsole(const Glib::ustring&) = 0;

        virtual const char* getAppDir() const = 0;

        virtual int getWidth() = 0;
        virtual void setWidth(int) = 0;
        virtual int getHeight() = 0;
        virtual void setHeight(int) = 0;
        virtual int getPane1Width() = 0;
        virtual void setPane1Width(int) = 0;
        virtual bool getConsoleEnabled(const Glib::ustring&) = 0;
        virtual void setConsoleEnabled(const Glib::ustring&, bool) = 0;
        virtual bool getConsoleScale(const Glib::ustring&) = 0;
        virtual void setConsoleScale(const Glib::ustring&, bool) = 0;

        virtual Glib::ustring getExportVmPath(const VirtualMachine&) = 0;
        virtual void setExportVmPath(const Glib::ustring&) = 0;
        virtual bool getExportVmVerify() = 0;
        virtual void setExportVmVerify(bool) = 0;
        virtual Glib::ustring getImportVmPath() = 0;
        virtual void setImportVmPath(const Glib::ustring&) = 0;
    };
}


#endif //!HNRT_MODEL_H
