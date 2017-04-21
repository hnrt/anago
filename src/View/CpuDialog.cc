// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "CpuDialog.h"


using namespace hnrt;


CpuDialog::CpuDialog(Gtk::Window& parent)
    : Gtk::Dialog(gettext("Change CPU settings"), parent)
    , _maxLabel(gettext("VCPUs maximum:"))
    , _startupLabel(gettext("VCPUs at startup:"))
    , _cpsLabel(gettext("Cores per socket:"))
{
    set_default_size(300, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();
 
    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_maxLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_maxEntry, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
    _table.attach(_startupLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _table.attach(_startupEntry, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
    _table.attach(_cpsLabel, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
    _table.attach(_cpsEntry, 1, 2, 2, 3, Gtk::SHRINK, Gtk::SHRINK);

    _maxLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _startupLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _cpsLabel.set_alignment(1.0, 0.5); // h=right, v=center

    _maxEntry.set_digits(0);
    _maxEntry.set_increments(1.0, 4.0);
    _maxEntry.set_range(1.0, 64.0);
 
    _startupEntry.set_digits(0);
    _startupEntry.set_increments(1.0, 4.0);
    _startupEntry.set_range(1.0, 64.0);
 
    _cpsEntry.set_digits(0);
    _cpsEntry.set_increments(1.0, 4.0);
    _cpsEntry.set_range(1.0, 64.0);

    show_all_children();
}


int64_t CpuDialog::getVcpusMax()
{
    int64_t max = _maxEntry.get_value_as_int();
    if (max < 1)
    {
        max = 1;
        _maxEntry.set_value(max);
    }
    return max;
}


void CpuDialog::setVcpusMax(int64_t value)
{
    _maxEntry.set_value(value);
}


int64_t CpuDialog::getVcpusAtStartup()
{
    int64_t startup = _startupEntry.get_value_as_int();
    int64_t max = _maxEntry.get_value_as_int();
    if (startup > max)
    {
        startup = max;
        _startupEntry.set_value(startup);
    }
    return startup;
}


void CpuDialog::setVcpusAtStartup(int64_t value)
{
    _startupEntry.set_value(value);
}


int CpuDialog::getCoresPerSocket()
{
    return _cpsEntry.get_value_as_int();
}


void CpuDialog::setCoresPerSocket(int value)
{
    _cpsEntry.set_value(value);
}
