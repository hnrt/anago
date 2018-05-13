// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_VGADIALOG_H
#define HNRT_VGADIALOG_H


#include <gtkmm.h>


namespace hnrt
{
    class VgaDialog
        : public Gtk::Dialog
    {
    public:

        VgaDialog(Gtk::Window&);
        bool isStdVga();
        void setStdVga(bool);
        int getVideoRam();
        void setVideoRam(int);

    private:

        VgaDialog(const VgaDialog&);
        void operator =(const VgaDialog&);
        void onToggled();

        Gtk::Table _table;
        Gtk::Label _vgaLabel;
        Gtk::RadioButtonGroup _vgaGroup;
        Gtk::RadioButton _cirrusButton;
        Gtk::RadioButton _standardButton;
        Gtk::Label _videoramLabel;
        Gtk::HBox _videoramBox;
        Gtk::SpinButton _videoramEntry;
        Gtk::Label _unitLabel;
    };
}


#endif //!HNRT_VGADIALOG_H
