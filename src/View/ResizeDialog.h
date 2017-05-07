// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_RESIZEDIALOG_H
#define HNRT_RESIZEDIALOG_H


#include "SizeInBytesBox.h"


namespace hnrt
{
    class ResizeDialog
        : public Gtk::Dialog
    {
    public:

        ResizeDialog(Gtk::Window&);
        int64_t getSize();
        void setSize(int64_t);

    private:

        ResizeDialog(const ResizeDialog&);
        void operator =(const ResizeDialog&);

        Gtk::Table _table;
        Gtk::Label _sizeLabel;
        SizeInBytesBox _sizeBox;
    };
}


#endif //!HNRT_RESIZEDIALOG_H
