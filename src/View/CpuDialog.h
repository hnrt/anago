// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_CPUDIALOG_H
#define HNRT_CPUDIALOG_H


#include <stdint.h>
#include <gtkmm.h>


namespace hnrt
{
    class CpuDialog
        : public Gtk::Dialog
    {
    public:

        CpuDialog(Gtk::Window&);
        int64_t getVcpusMax();
        void setVcpusMax(int64_t);
        int64_t getVcpusAtStartup();
        void setVcpusAtStartup(int64_t);
        int getCoresPerSocket();
        void setCoresPerSocket(int);

    private:

        CpuDialog(const CpuDialog&);
        void operator =(const CpuDialog&);

        Gtk::Table _table;
        Gtk::Label _maxLabel;
        Gtk::SpinButton _maxEntry;
        Gtk::Label _startupLabel;
        Gtk::SpinButton _startupEntry;
        Gtk::Label _cpsLabel;
        Gtk::SpinButton _cpsEntry;
    };
}


#endif //!HNRT_CPUDIALOG_H
