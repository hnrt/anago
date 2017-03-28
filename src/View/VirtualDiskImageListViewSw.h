// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALDISKIMAGELISTVIEWSW_H
#define HNRT_VIRTUALDISKIMAGELISTVIEWSW_H


#include "VirtualDiskImageListView.h"


namespace hnrt
{
    class VirtualDiskImageListViewSw 
        : public Gtk::ScrolledWindow
    {
    public:

        VirtualDiskImageListViewSw();
        const VirtualDiskImageListView& listView() const { return _listView; }
        VirtualDiskImageListView& listView() { return _listView; }

    protected:

        VirtualDiskImageListViewSw(const VirtualDiskImageListViewSw&);
        void operator =(const VirtualDiskImageListViewSw&);

        VirtualDiskImageListView _listView;
    };
}


#endif //!HNRT_VIRTUALDISKIMAGELISTVIEWSW_H
