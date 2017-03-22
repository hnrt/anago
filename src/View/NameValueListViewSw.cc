// Copyright (C) 2012-2017 Hideaki Narita


#include "NameValueListViewSw.h"


using namespace hnrt;


NameValueListViewSw::NameValueListViewSw()
{
    add(_listView);
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    set_shadow_type(Gtk::SHADOW_IN);
}
