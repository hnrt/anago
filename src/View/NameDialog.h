// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NAMEDIALOG_H
#define HNRT_NAMEDIALOG_H


#include <gtkmm.h>


namespace hnrt
{
    class NameDialog
        : public Gtk::Dialog
    {
    public:

        NameDialog(Gtk::Window&, const char*);
        Glib::ustring getLabel() const;
        void setLabel(const char*);
        Glib::ustring getDescription() const;
        void setDescription(const char*);

    private:

        NameDialog(const NameDialog&);
        void operator =(const NameDialog&);

        Gtk::Table _table;
        Gtk::Label _labelLabel;
        Gtk::Entry _labelEntry;
        Gtk::Label _descriptionLabel;
        Gtk::Entry _descriptionEntry;
    };
}


#endif //!HNRT_NAMEDIALOG_H
