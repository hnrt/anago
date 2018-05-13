// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_ATTACHHDDDIALOG_H
#define HNRT_ATTACHHDDDIALOG_H


#include "Base/RefPtr.h"
#include "BlockDeviceNumberComboBox.h"
#include "StorageRepositoryComboBox.h"
#include "VirtualDiskImageListView.h"


namespace hnrt
{
    class StorageRepository;
    class VirtualMachine;
    class XenObject;

    class AttachHddDialog
        : public Gtk::Dialog
    {
    public:

        AttachHddDialog(Gtk::Window&, const VirtualMachine&);
        virtual ~AttachHddDialog();
        Glib::ustring getUserDevice();
        Glib::ustring getVdi();

    private:


        AttachHddDialog(const AttachHddDialog&);
        void operator =(const AttachHddDialog&);
        void onSrChanged();
        void onVdiChanged();
        void onAdd();
        void onResize();
        void onRemove();
        void onObjectUpdated(RefPtr<XenObject>, int);
        void update(const StorageRepository&);

        const VirtualMachine& _vm;
        Gtk::Button* _pApplyButton;
        Gtk::Table _table;
        Gtk::Label _devLabel;
        BlockDeviceNumberComboBox _devCombo;
        Gtk::HBox _devBox;
        Gtk::Label _srLabel;
        StorageRepositoryComboBox _srCombo;
        Gtk::HBox _srBox;
        Gtk::Label _vdiLabel;
        VirtualDiskImageListView _vdiList;
        Gtk::Button _addButton;
        Gtk::Button _resizeButton;
        Gtk::Button _removeButton;
        Gtk::VBox _buttonBox;
        sigc::connection _srUpdated;
    };
}


#endif //!ANAGO_ATTACHHDDDIALOG_H
