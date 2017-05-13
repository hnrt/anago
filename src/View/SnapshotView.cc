// Copyright (C) 2012-2017 Hideaki Narita


#include "XenServer/VirtualMachine.h"
#include "SnapshotView.h"


using namespace hnrt;


SnapshotView::SnapshotView()
{
    Gtk::ScrolledWindow::add(_treeView);
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    set_shadow_type(Gtk::SHADOW_IN);
}
