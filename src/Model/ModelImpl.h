// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_MODELIMPL_H
#define HNRT_MODELIMPL_H


#include <glibmm.h>
#include "Logger/Logger.h"
#include "File/Json.h"
#include "ConsoleInfo.h"
#include "Model.h"


namespace hnrt
{
    class Host;
    struct ConnectSpec;

    class ModelImpl
        : public Model
    {
    public:

        ModelImpl();
        ~ModelImpl();
        void init();
        void fini();
        virtual void load();
        virtual void save();
        virtual void clear();
        virtual int get(std::list<RefPtr<Host> >&);
        virtual void add(const ConnectSpec&);
        virtual void remove(Session&);
        virtual void removeAllSessions();

        virtual void deselectAll();
        virtual void select(const RefPtr<XenObject>&);
        virtual int getSelected(std::list<RefPtr<Host> >&);
        virtual int getSelected(std::list<RefPtr<VirtualMachine> >&);
        virtual int getSelected(std::list<RefPtr<StorageRepository> >&);
        virtual RefPtr<Host> getSelectedHost();
        virtual RefPtr<VirtualMachine> getSelectedVm();
        virtual RefPtr<StorageRepository> getSelectedSr();

        virtual void selectSnapshot(const RefPtr<VirtualMachine>&);
        virtual void deselectSnapshot();

        virtual RefPtr<PatchBase> getPatchBase();

        virtual void addConsole(const Glib::ustring&, const RefPtr<Console>&);
        virtual void removeConsole(const Glib::ustring&);
        virtual RefPtr<Console> getConsole(const Glib::ustring&);

        virtual const char* getAppDir() const { return _appDir.c_str(); }

        virtual int getWidth();
        virtual void setWidth(int);
        virtual int getHeight();
        virtual void setHeight(int);
        virtual int getPane1Width();
        virtual void setPane1Width(int);
        virtual bool getConsoleEnabled(const Glib::ustring&);
        virtual void setConsoleEnabled(const Glib::ustring&, bool);
        virtual bool getConsoleScale(const Glib::ustring&);
        virtual void setConsoleScale(const Glib::ustring&, bool);

        virtual Glib::ustring getExportVmPath(const VirtualMachine&);
        virtual void setExportVmPath(const Glib::ustring&);
        virtual bool getExportVmVerify();
        virtual void setExportVmVerify(bool);
        virtual Glib::ustring getImportVmPath();
        virtual void setImportVmPath(const Glib::ustring&);
        virtual Glib::ustring getVerifyVmPath();
        virtual void setVerifyVmPath(const Glib::ustring&);

    private:

        ModelImpl(const ModelImpl&);
        void operator =(const ModelImpl&);
        void loadV1(const RefPtr<Json>&);
        void loadV1Server(const RefPtr<Json>&);
        void loadV1Console(const RefPtr<Json>&);
        ConsoleInfo& getConsoleInfo(const Glib::ustring&);

        Glib::ustring _path;
        Glib::ustring _appDir;
        Glib::RecMutex _mutex;
        std::list<RefPtr<Host> > _hosts;
        std::list<RefPtr<XenObject> > _selected;
        RefPtr<VirtualMachine> _selectedSnapshot;
        RefPtr<PatchBase> _patchBase;
        ConsoleMap _consoleMap;
        int _width;
        int _height;
        int _pane1Width;
        Glib::ustring _exportVmPath;
        bool _exportVmVerify;
        Glib::ustring _importVmPath;
        Glib::ustring _verifyVmPath;
    };
}


#endif //!HNRT_MODELIMPL_H
