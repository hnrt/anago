// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHLISTVIEWSW_H
#define HNRT_PATCHLISTVIEWSW_H


#include <gtkmm.h>
#include "PatchListView.h"


namespace hnrt
{
    class PatchListViewSw
        : public Gtk::ScrolledWindow
    {
    public:

        PatchListViewSw();
        const PatchListView& listView() const { return _listView; }
        PatchListView& listView() { return _listView; }

    private:

        PatchListViewSw(const PatchListViewSw&);
        void operator =(const PatchListViewSw&);

        PatchListView _listView;
    };
}


#endif //!HNRT_PATCHLISTVIEWSW_H
