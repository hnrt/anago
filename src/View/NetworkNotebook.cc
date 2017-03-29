// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "XenServer/Network.h"
#include "XenServer/PhysicalInterface.h"
#include "XenServer/Session.h"
#include "XenServer/XenObjectStore.h"
#include "XenServer/XenPtr.h"
#include "PropertyHelper.h"
#include "NetworkNotebook.h"


using namespace hnrt;


RefPtr<Notebook> NetworkNotebook::create(Network& network)
{
    return RefPtr<Notebook>(new NetworkNotebook(network));
}


NetworkNotebook::NetworkNotebook(Network& network)
    : _networkLv(_networkLvSw.listView())
    , _networkMenu(network)
    , _pifLv(_pifLvSw.listView())
    , _network(network)
{
    Trace trace(StringBuffer().format("NetworkNotebook@%zx::ctor", this));

    _networkLv.setMenu(&_networkMenu);

    _networkLabel.set_label(gettext("Network:"));
    _networkLabel.set_alignment(0, 0.5); // h=left, v=center
    _networkBox.pack_start(_networkLabel, Gtk::PACK_SHRINK);
    _networkBox.pack_start(_networkLvSw);

    _pifLabel.set_label(gettext("Physical interface:"));
    _pifLabel.set_alignment(0, 0.5); // h=left, v=center
    _pifBox.pack_start(_pifLabel, Gtk::PACK_SHRINK);
    _pifBox.pack_start(_pifLvSw);

    _genBox.pack1(_networkBox, true, true);
    _genBox.pack2(_pifBox, true, true);
    append_page(_genBox, Glib::ustring(gettext("Properties")));

    show_all_children();

    onNetworkUpdated(RefPtr<XenObject>(&_network, 1), XenObject::RECORD_UPDATED);
 
    _connection = SignalManager::instance().xenObjectSignal(_network).connect(sigc::mem_fun(*this, &NetworkNotebook::onNetworkUpdated));
}


NetworkNotebook::~NetworkNotebook()
{
    Trace trace(StringBuffer().format("NetworkNotebook@%zx::dtor", this));

    _connection.disconnect();
}


void NetworkNotebook::onNetworkUpdated(RefPtr<XenObject> object, int what)
{
    Trace trace(StringBuffer().format("NetworkNotebook@%zx::onNetworkUpdated", this));

    if (what == XenObject::RECORD_UPDATED)
    {
        XenPtr<xen_network_record> nwRecord = _network.getRecord();
        SetNetworkProperties(_networkLv, nwRecord);

        XenPtr<xen_pif_record> pifRecord;

        if (nwRecord->pifs && nwRecord->pifs->size)
        {
            Session& session = _network.getSession();
            RefPtr<PhysicalInterface> pif = session.getStore().getPif(nwRecord->pifs->contents[0]);
            if (pif)
            {
                pifRecord = pif->getRecord();
            }
        }

        if (pifRecord)
        {
            SetPifProperties(_pifLv, pifRecord);
        }
        else
        {
            _pifLv.clear();
        }
    }
}
