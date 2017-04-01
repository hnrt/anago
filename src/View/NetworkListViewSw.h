// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_NETWORKLISTVIEWSW_H
#define HNRT_NETWORKLISTVIEWSW_H


#include <gtkmm.h>
#include "NetworkListView.h"


namespace hnrt
{
    class NetworkListViewSw : public Gtk::ScrolledWindow
    {
    public:

        NetworkListViewSw(Session& session);
        const NetworkListView& listView() const { return _listView; }
        NetworkListView& listView() { return _listView; }

    private:

        NetworkListViewSw(const NetworkListViewSw&);
        void operator =(const NetworkListViewSw&);

        NetworkListView _listView;
    };
}


#endif //!HNRT_NETWORKLISTVIEWSW_H
