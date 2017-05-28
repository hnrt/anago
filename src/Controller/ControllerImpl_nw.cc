// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "View/View.h"
#include "XenServer/Network.h"
#include "XenServer/Session.h"
#include "ControllerImpl.h"


using namespace hnrt;


void ControllerImpl::changeNetworkName(Network& network)
{
    XenPtr<xen_network_record> record = network.getRecord();
    Glib::ustring label(record->name_label);
    Glib::ustring description(record->name_description);
    if (!View::instance().getName(gettext("Change network label/description"), label, description))
    {
        return;
    }
    Session& session = network.getSession();
    Session::Lock lock(session);
    network.setName(label.c_str(), description.c_str());
}
