// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SIZEINBYTESBOX_H
#define HNRT_SIZEINBYTESBOX_H


#include <stdint.h>
#include <gtkmm.h>


namespace hnrt
{
    class SizeInBytesBox
        : public Gtk::HBox
    {
    public:

        SizeInBytesBox();
        virtual ~SizeInBytesBox();
        void setValue(int64_t value, bool round = false);
        int64_t getValue();

    protected:

        struct Record
            : public Gtk::TreeModel::ColumnRecord
        {
            Gtk::TreeModelColumn<Glib::ustring> colName;
            Gtk::TreeModelColumn<int64_t> colValue;

            Record()
            {
                add(colName);
                add(colValue);
            }

        private:

            Record(const Record&);
            void operator =(const Record&);
        };

        SizeInBytesBox(const SizeInBytesBox&);
        void operator =(const SizeInBytesBox&);
        void initStore();
        void onUnitChanged();
        void onNumChanged();

        Gtk::SpinButton _spinButton;
        Record _record;
        Glib::RefPtr<Gtk::ListStore> _store;
        Gtk::ComboBox _unitCombo;
        Gtk::Entry _numEntry;

        int64_t _unit;
        Glib::ustring _text;
    };
}


#endif //!HNRT_SIZEINBYTESBOX_H
