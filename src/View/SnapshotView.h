// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_SNAPSHOTVIEW_H
#define HNRT_SNAPSHOTVIEW_H


#include "SnapshotTreeView.h"


namespace hnrt
{
    class SnapshotView
        : public Gtk::ScrolledWindow
    {
    public:

        SnapshotView();
        SnapshotTreeView& getTreeView() { return _treeView; }

    protected:

        SnapshotView(const SnapshotView&);
        void operator =(const SnapshotView&);

        SnapshotTreeView _treeView;
    };
}


#endif //!HNRT_SNAPSHOTVIEW_H
