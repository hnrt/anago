// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIEWIMPL_H
#define HNRT_VIEWIMPL_H


#include "MainWindow.h"
#include "View.h"
#include "VirtualMachineStatusWindow.h"


namespace hnrt
{
    class ViewImpl
        : public View
    {
    public:

        ViewImpl();
        ~ViewImpl();
        virtual const Glib::ustring& getDisplayName() { return _displayName; }
        virtual Gtk::Window& getWindow() { return _mainWindow; }
        virtual Gtk::Window& getStatusWindow() { return _statusWindow; }
        virtual void load();
        virtual void save();
        virtual void clear();
        virtual void about();
        virtual void showInfo(const Glib::ustring&);
        virtual void showWarning(const Glib::ustring&);
        virtual void showError(const Glib::ustring&);
        virtual bool askYesNo(const Glib::ustring&);
        virtual bool getConnectSpec(ConnectSpec&);
        virtual bool confirmServerToRemove(const char*);
        virtual void showBusyServers(const std::list<Glib::ustring>&);
        virtual bool confirmServersToShutdown(const std::list<Glib::ustring>&, bool);
        virtual bool getName(const char*, Glib::ustring&, Glib::ustring&);
        virtual bool getCpuSettings(int64_t&, int64_t&, int&);
        virtual bool getMemorySettings(int64_t&, int64_t&, int64_t&, int64_t&);
        virtual bool getShadowMemorySettings(double&);
        virtual bool getVgaSettings(bool&, int&);
        virtual bool selectCd(const VirtualMachine&, Glib::ustring&, Glib::ustring&);
        virtual bool getVirtualMachineSpec(const Session&, VirtualMachineSpec&);
        virtual bool getVirtualMachineToCopy(const Session&, Glib::ustring&, Glib::ustring&);
        virtual bool getDisksToDelete(const VirtualMachine&, std::list<Glib::ustring>&);
        virtual bool getExportVmPath(Glib::ustring&, bool&);
        virtual bool getImportVmPath(Glib::ustring&);

    private:

        ViewImpl(const ViewImpl&);
        void operator =(const ViewImpl&);
        void showMessageDialog(const Glib::ustring&, Gtk::MessageType);

        Glib::ustring _displayName;
        MainWindow _mainWindow;
        VirtualMachineStatusWindow _statusWindow;
    };
}


#endif //!HNRT_VIEWIMPL_H
