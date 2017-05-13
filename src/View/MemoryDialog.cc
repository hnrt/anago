// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "MemoryDialog.h"


using namespace hnrt;


MemoryDialog::MemoryDialog(Gtk::Window& parent)
    : Gtk::Dialog(gettext("Change memory settings"), parent)
    , _smaxLabel(gettext("Static maximum:"))
    , _dmaxLabel(gettext("Dynamic maximum:"))
    , _dminLabel(gettext("Dynamic minimum:"))
    , _sminLabel(gettext("Static minimum:"))
{
    set_default_size(300, -1);
    set_border_width(6);

    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    add_button(Gtk::Stock::APPLY, Gtk::RESPONSE_APPLY);

    Gtk::VBox* box = get_vbox();

    _table.set_spacings(6);
    _table.set_border_width(6);
    box->pack_start(_table, Gtk::PACK_EXPAND_WIDGET);

    _table.attach(_smaxLabel, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
    _table.attach(_smaxBox, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_dmaxLabel, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
    _table.attach(_dmaxBox, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_dminLabel, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
    _table.attach(_dminBox, 1, 2, 2, 3, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
    _table.attach(_sminLabel, 0, 1, 3, 4, Gtk::FILL, Gtk::FILL);
    _table.attach(_sminBox, 1, 2, 3, 4, Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
 
    _smaxLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _dmaxLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _dminLabel.set_alignment(1.0, 0.5); // h=right, v=center
    _sminLabel.set_alignment(1.0, 0.5); // h=right, v=center

    show_all_children();
}


int64_t MemoryDialog::getStaticMin()
{
    int64_t smin = _sminBox.getValue();
    int64_t smax = _smaxBox.getValue();
    if (smin > smax)
    {
        smin = smax;
        _sminBox.setValue(smin);
    }
    return smin;
}


void MemoryDialog::setStaticMin(int64_t value)
{
    _sminBox.setValue(value);
}


int64_t MemoryDialog::getStaticMax()
{
    return _smaxBox.getValue();
}


void MemoryDialog::setStaticMax(int64_t value)
{
    _smaxBox.setValue(value);
}


int64_t MemoryDialog::getDynamicMin()
{
    int64_t smin = getStaticMin();
    int64_t dmax = getDynamicMax();
    int64_t dmin = _dminBox.getValue();
    if (dmin > dmax)
    {
        dmin = dmax;
        _dminBox.setValue(dmin);
    }
    else if (dmin < smin)
    {
        dmin = smin;
        _dminBox.setValue(dmin);
    }
    return dmin;
}


void MemoryDialog::setDynamicMin(int64_t value)
{
    _dminBox.setValue(value);
}


int64_t MemoryDialog::getDynamicMax()
{
    int64_t smax = getStaticMax();
    int64_t smin = getStaticMin();
    int64_t dmax = _dmaxBox.getValue();
    if (dmax > smax)
    {
        dmax = smax;
        _dmaxBox.setValue(dmax);
    }
    else if (dmax < smin)
    {
        dmax = smin;
        _dmaxBox.setValue(dmax);
    }
    return dmax;
}


void MemoryDialog::setDynamicMax(int64_t value)
{
    _dmaxBox.setValue(value);
}
