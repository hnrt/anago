// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NAMEVALUELISTVIEWSW_H
#define HNRT_NAMEVALUELISTVIEWSW_H


#include <gtkmm.h>
#include "NameValueListView.h"


namespace hnrt
{
    class NameValueListViewSw
        : public Gtk::ScrolledWindow
    {
    public:

        NameValueListViewSw();
        const NameValueListView& listView() const { return _listView; }
        NameValueListView& listView() { return _listView; }

    private:

        NameValueListViewSw(const NameValueListViewSw&);
        void operator =(const NameValueListViewSw&);

        NameValueListView _listView;
    };
}


#endif //!HNRT_NAMEVALUELISTVIEWSW_H
