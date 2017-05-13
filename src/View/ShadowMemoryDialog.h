// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SHADOWMEMORYDIALOGBOX_H
#define HNRT_SHADOWMEMORYDIALOGBOX_H


#include <gtkmm.h>


namespace hnrt
{
    class ShadowMemoryDialog
        : public Gtk::Dialog
    {
    public:

        ShadowMemoryDialog(Gtk::Window&);
        double getMultiplier();
        void setMultiplier(double value);

    private:

        ShadowMemoryDialog(const ShadowMemoryDialog&);
        void operator =(const ShadowMemoryDialog&);

        Gtk::Table _table;
        Gtk::Label _mulLabel;
        Gtk::SpinButton _mulSpinButton;
        Gtk::HBox _mulBox;
    };
}


#endif //!HNRT_SHADOWMEMORYDIALOGBOX_H
