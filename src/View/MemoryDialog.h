// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_MEMORYDIALOG_H
#define HNRT_MEMORYDIALOG_H


#include <gtkmm.h>
#include "SizeInBytesBox.h"


namespace hnrt
{
    class MemoryDialog
        : public Gtk::Dialog
    {
    public:

        MemoryDialog(Gtk::Window&);
        int64_t getStaticMin();
        void setStaticMin(int64_t);
        int64_t getStaticMax();
        void setStaticMax(int64_t);
        int64_t getDynamicMin();
        void setDynamicMin(int64_t);
        int64_t getDynamicMax();
        void setDynamicMax(int64_t);

    private:

        MemoryDialog(const MemoryDialog&);
        void operator =(const MemoryDialog&);

        Gtk::Table _table;
        Gtk::Label _smaxLabel;
        SizeInBytesBox _smaxBox;
        Gtk::Label _dmaxLabel;
        SizeInBytesBox _dmaxBox;
        Gtk::Label _dminLabel;
        SizeInBytesBox _dminBox;
        Gtk::Label _sminLabel;
        SizeInBytesBox _sminBox;
    };
}


#endif //!HNRT_MEMORYDIALOG_H
