// Copyright (C) 2012-2017 Hideaki Narita


#include "XenServer/VirtualDiskImage.h"
#include "VirtualDiskImageListViewSw.h"


using namespace hnrt;


VirtualDiskImageListViewSw::VirtualDiskImageListViewSw()
    : _listView()
{
    add(_listView);
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    set_shadow_type(Gtk::SHADOW_IN);
}
