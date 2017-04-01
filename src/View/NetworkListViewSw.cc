// Copyright (C) 2012-2017 Hideaki Narita


#include "NetworkListViewSw.h"


using namespace hnrt;


NetworkListViewSw::NetworkListViewSw(Session& session)
    : _listView(session)
{
    add(_listView);
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    set_shadow_type(Gtk::SHADOW_IN);
}
