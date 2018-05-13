// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_ADDVMDIALOG_H
#define HNRT_ADDVMDIALOG_H


#include <gtkmm.h>
#include "CdImageListView.h"
#include "HardDiskDriveListBox.h"
#include "NetworkListView.h"
#include "VirtualMachineTemplateListView.h"


namespace hnrt
{
    struct VirtualMachineSpec;

    class AddVmDialog
        : public Gtk::Dialog
    {
    public:

        AddVmDialog(Gtk::Window&, const Session&);
        void getSpec(VirtualMachineSpec&);

    private:

        AddVmDialog(const AddVmDialog&);
        void operator =(const AddVmDialog&);
        void onTemplateChanged();
        void onHardDiskDriveChanged();
        void onCdChanged();
        void onNetworkChanged();
        void validate();
        Glib::ustring getTemplate() const { return _templateLv.getSelected(); }
        Glib::ustring getName() const { return _nameEntry.get_text(); }
        Glib::ustring getDesc() const { return _descEntry.get_text(); }
        int getHardDiskDriveList(std::list<HardDiskDriveSpec>& list) const;
        Glib::ustring getCd() const { return _cdLv.getSelected(); }
        int getNetworks(std::list<Glib::ustring>& list) const { return _nwLv.getSelected(list); }

        Gtk::HPaned _hpaned;
        Gtk::VBox _box1;
        Gtk::VBox _box2;
        Gtk::Label _templateLabel;
        VirtualMachineTemplateListView _templateLv;
        Gtk::ScrolledWindow& _teSw;
        Gtk::Label _nameLabel;
        Gtk::Entry _nameEntry;
        Gtk::Label _descLabel;
        Gtk::Entry _descEntry;
        Gtk::Label _hddLabel;
        HardDiskDriveListBox _hddBox;
        Gtk::Label _cdLabel;
        CdImageListView _cdLv;
        Gtk::ScrolledWindow& _cdSw;
        Gtk::Label _nicLabel;
        NetworkListView _nwLv;
        Gtk::ScrolledWindow& _nwSw;

        const Session& _session;

        Gtk::Button* _pCancelButton;
        Gtk::Button* _pApplyButton;
    };
}


#endif //!HNRT_ADDVMDIALOG_H
