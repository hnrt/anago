// Copyright (C) 2012-2018 Hideaki Narita


#define NO_TRACE


#include <math.h>
#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Logger/Trace.h"
#include "SizeInBytesBox.h"


using namespace hnrt;


#define SPIN_MIN 1.0
#define SPIN_MAX 9999.0
#define SPIN_STEP 1.0
#define SPIN_PAGE 16.0


SizeInBytesBox::SizeInBytesBox()
    : _unit(1)
    , _text("1")
{
    TRACEPUT(this, "SizeInBytesBox::ctor");

    _spinButton.set_digits(0);
    _spinButton.set_increments(SPIN_STEP, SPIN_PAGE);
    _spinButton.set_range(SPIN_MIN, SPIN_MAX);

    _store = Gtk::ListStore::create(_record);
    initStore();

    _unitCombo.set_model(_store);
    _unitCombo.pack_start(_record.colName, true);
    _unitCombo.set_active(0);

    _spinButton.set_value(strtol(_text.c_str(), NULL, 10));
    _numEntry.set_text(_text);
    _numEntry.set_alignment(1.0); // right

    pack_start(_numEntry, Gtk::PACK_SHRINK);
    pack_start(_unitCombo, Gtk::PACK_SHRINK);

    _unitCombo.signal_changed().connect(sigc::mem_fun(*this, &SizeInBytesBox::onUnitChanged));
    _numEntry.signal_changed().connect(sigc::mem_fun(*this, &SizeInBytesBox::onNumChanged));
}


SizeInBytesBox::~SizeInBytesBox()
{
    TRACEPUT(this, "SizeInBytesBox::dtor");
}


void SizeInBytesBox::initStore()
{
    TRACEPUT(this, "SizeInBytesBox::initStore");
    Gtk::TreeModel::Row row = *_store->append();
    row[_record.colName] = Glib::ustring(gettext("Bytes"));
    row[_record.colValue] = 1L;
    row = *_store->append();
    row[_record.colName] = Glib::ustring("KiB");
    row[_record.colValue] = 1024L;
    row = *_store->append();
    row[_record.colName] = Glib::ustring("MiB");
    row[_record.colValue] = 1024L * 1024L;
    row = *_store->append();
    row[_record.colName] = Glib::ustring("GiB");
    row[_record.colValue] = 1024L * 1024L * 1024L;
    row = *_store->append();
    row[_record.colName] = Glib::ustring("TiB");
    row[_record.colValue] = 1024L * 1024L * 1024L * 1024L;
}


void SizeInBytesBox::onUnitChanged()
{
    TRACEPUT(this, "SizeInBytesBox::onUnitChanged");
    TRACEPUT("unit=%zd", _unit);
    Gtk::TreeIter iter = _unitCombo.get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        int64_t unitNext = row[_record.colValue];
        if (unitNext == 1)
        {
            if (_unit != 1)
            {
                _spinButton.hide();
                remove(_spinButton);
                remove(_unitCombo);
                pack_start(_numEntry, Gtk::PACK_SHRINK);
                pack_start(_unitCombo, Gtk::PACK_SHRINK);
                _numEntry.show();
                _unitCombo.show();
                _numEntry.set_text(StringBuffer().format("%zd", static_cast<int64_t>(_spinButton.get_value() * _unit)).str());
            }
        }
        else
        {
            double value;
            if (_unit == 1)
            {
                _numEntry.hide();
                remove(_numEntry);
                remove(_unitCombo);
                pack_start(_spinButton, Gtk::PACK_SHRINK);
                pack_start(_unitCombo, Gtk::PACK_SHRINK);
                _spinButton.show();
                _unitCombo.show();
                value = strtod(_numEntry.get_text().c_str(), NULL);
            }
            else
            {
                value = _spinButton.get_value() * _unit;
            }
            value = ceil(value / unitNext);
            if (value < SPIN_MIN)
            {
                value = SPIN_MIN;
            }
            else if (value > SPIN_MAX)
            {
                value = SPIN_MAX;
            }
            _spinButton.set_value(value);
        }
        _unit = unitNext;
    }
    TRACEPUT("unit=%zd", _unit);
}


void SizeInBytesBox::setValue(int64_t value, bool round)
{
    TRACEPUT(this, "SizeInBytesBox::setValue(%zd,%d)", value, round);
    if (round)
    {
        int64_t u = 1;
        Gtk::TreeIter iter = _store->get_iter("0"); // point to first item
        while (1)
        {
            Gtk::TreeModel::Row row = *iter;
            u = row[_record.colValue];
            if (value < u * 1024L)
            {
                _unitCombo.set_active(iter);
                break;
            }
            Gtk::TreeIter iterLast = iter;
            iter++;
            if (!iter)
            {
                _unitCombo.set_active(iterLast);
                break;
            }
        }
        double v = ceil((double)value / u);
        if (v < SPIN_MIN)
        {
            v = SPIN_MIN;
        }
        else if (v > SPIN_MAX)
        {
            v = SPIN_MAX;
        }
        _spinButton.set_value(v);
    }
    else
    {
        _unitCombo.set_active(0);
        _text.assign(StringBuffer().format("%zd", value > 1 ? value : 1));
        _numEntry.set_text(_text);
    }
}


int64_t SizeInBytesBox::getValue()
{
    TRACEPUT(this, "SizeInBytesBox::getValue");
    int64_t value;
    if (_unit == 1)
    {
        char* stop = NULL;
        value = strtoul(_numEntry.get_text().c_str(), &stop, 10);
        if (!*stop)
        {
        }
        else if (!strcasecmp(stop, "K"))
        {
            value *= 1024L;
        }
        else if (!strcasecmp(stop, "M"))
        {
            value *= 1024L * 1024L;
        }
        else if (!strcasecmp(stop, "G"))
        {
            value *= 1024L * 1024L * 1024L;
        }
        else if (!strcasecmp(stop, "T"))
        {
            value *= 1024L * 1024L * 1024L * 1024L;
        }
        else
        {
        }
    }
    else
    {
        double spinValue = _spinButton.get_value();
        TRACEPUT("spin=%g unit=%zu", spinValue, _unit);
        value = static_cast<int64_t>(spinValue * _unit);
    }
    TRACEPUT("value=%zu", value);
    return value;
}


void SizeInBytesBox::onNumChanged()
{
    TRACEPUT(this, "SizeInBytesBox::onNumChanged");
    Glib::ustring num = _numEntry.get_text();
    TRACEPUT("num=%s", num.c_str());
    char* stop = NULL;
    int64_t value = strtoul(num.c_str(), &stop, 10);
    if (!*stop
        || !strcasecmp(stop, "K")
        || !strcasecmp(stop, "M")
        || !strcasecmp(stop, "G")
        || !strcasecmp(stop, "T"))
    {
        _text = num;
    }
    else
    {
        _numEntry.set_text(_text);
        TRACEPUT("num=%s", _numEntry.get_text().c_str());
    }
    (void)value;
}
