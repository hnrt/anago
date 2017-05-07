// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CHANGECDDIALOG_H
#define HNRT_CHANGECDDIALOG_H


#include "CdDeviceComboBox.h"
#include "CdImageListView.h"


namespace hnrt
{
    class VirtualBlockDevice;
    class VirtualMachine;

    class ChangeCdDialog
        : public Gtk::Dialog
    {
    public:

        ChangeCdDialog(Gtk::Window&, const VirtualMachine&);
        void select(const Glib::ustring&);
        Glib::ustring getDevice() const { return _devCombo.getSelected(); }
        Glib::ustring getImage() const { return _imgListView.getSelected(); }

    private:

        ChangeCdDialog(const ChangeCdDialog&);
        void operator =(const ChangeCdDialog&);
        void onDeviceChanged();
        void onImageChanged();

        Gtk::Table _table;
        Gtk::Label _devLabel;
        Gtk::HBox _devBox;
        CdDeviceComboBox _devCombo;
        Gtk::Label _imgLabel;
        CdImageListView _imgListView;
        Gtk::Button* _pApplyButton;
    };
}


#endif //!HNRT_CHANGECDDIALOG_H
