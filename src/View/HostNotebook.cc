// Copyright (C) 2012-2017 Hideaki Narita


#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Controller/SignalManager.h"
#include "Logger/Trace.h"
#include "Model/Model.h"
#include "Model/PatchBase.h"
#include "XenServer/Host.h"
#include "XenServer/PerformanceMonitor.h"
#include "XenServer/Session.h"
#include "XenServer/XenObjectStore.h"
#include "PropertyHelper.h"
#include "HostNotebook.h"


using namespace hnrt;


RefPtr<Notebook> HostNotebook::create(const RefPtr<Host>& host)
{
    return RefPtr<Notebook>(new HostNotebook(host));
}


HostNotebook::HostNotebook(const RefPtr<Host>& host)
    : _genLv(_genLvSw.listView())
    , _genLvMenu(HostMenu::NAME_VALUE)
    , _cpuLv(_cpuLvSw.listView())
    , _memLv(_memLvSw.listView())
    , _swvLv(_swvLvSw.listView())
    , _patLv(_patLvSw.listView())
    , _host(host)
{
    Trace trace(StringBuffer().format("HostNotebook@%zx::ctor(%s)", this, _host->getSession().getConnectSpec().hostname.c_str()));

    _genBox.pack_start(_genLvSw);
    _genLv.setMenu(&_genLvMenu);

    _cpuBox.pack_start(_cpuLvSw);

    _memBox.pack_start(_memLvSw);

    _swvBox.pack_start(_swvLvSw);

    _patBox.pack_start(_patLvSw);

    _pfmSw.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    _pfmSw.add(_pfmBox2);
    _cpuLabel.set_label(gettext("CPU Load History"));
    _cpuLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _pfmBox2.pack_start(_cpuLabel, Gtk::PACK_SHRINK);
    _pfmBox2.pack_start(_cpuGraph);
    _memLabel.set_label(gettext("Memory Usage History"));
    _memLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _pfmBox2.pack_start(_memLabel, Gtk::PACK_SHRINK);
    _pfmBox2.pack_start(_memGraph);
    _netLabel.set_label(gettext("Network I/O History"));
    _netLabel.set_alignment(0.0, 0.5); // h=left, v=center
    _pfmBox2.pack_start(_netLabel, Gtk::PACK_SHRINK);
    _pfmBox2.pack_start(_netGraph);
    _pfmBox.pack_start(_pfmSw);

    _autoConnect.set_label(gettext("Automatically connect to this host at start time"));
    _autoConnect.set_active(_host->getSession().getConnectSpec().autoConnect);
    _autoConnect.signal_toggled().connect(sigc::mem_fun(*this, &HostNotebook::onAutoConnectChanged));
    _optBox.pack_start(_autoConnect, Gtk::PACK_SHRINK);
    append_page(_optBox, Glib::ustring(gettext("Options")));

    show_all_children();

    SignalManager& sm = SignalManager::instance();
    _connectionSession = sm.xenObjectSignal(_host->getSession()).connect(sigc::mem_fun(*this, &HostNotebook::onSessionUpdated));
    _connectionHost = sm.xenObjectSignal(*host).connect(sigc::mem_fun(*this, &HostNotebook::onHostUpdated));
}


HostNotebook::~HostNotebook()
{
    Trace trace(StringBuffer().format("HostNotebook@%zx::dtor(%s)", this, _host->getSession().getConnectSpec().hostname.c_str()));
    trace.put("host.ref=%d", _host->refCount());

    _connectionSession.disconnect();
    _connectionHost.disconnect();
}


void HostNotebook::onAutoConnectChanged()
{
    _host->getSession().getConnectSpec().autoConnect = _autoConnect.get_active();
}


void HostNotebook::onSessionUpdated(RefPtr<XenObject> object, int what)
{
    Trace trace(StringBuffer().format("HostNotebook@%zx::onSessionUpdated(%zx,%d)", this, object.ptr(), what));

    switch (what)
    {
    case XenObject::CONNECTED:
    case XenObject::DISCONNECTED:
        update();
        break;
    case XenObject::PERFORMANCE_STATS_UPDATED:
        updatePerformaceStats();
        break;
    default:
        break;
    }
}


void HostNotebook::onHostUpdated(RefPtr<XenObject> object, int what)
{
    Trace trace(StringBuffer().format("HostNotebook@%zx::onHostUpdated(%zx,%d)", this, object.ptr(), what));

    switch (what)
    {
    case XenObject::CONNECTED:
    case XenObject::CONNECT_FAILED:
    case XenObject::DISCONNECTED:
    case XenObject::RECORD_UPDATED:
    case XenObject::NAME_UPDATED:
    case XenObject::STATUS_UPDATED:
    case XenObject::SESSION_UPDATED:
        update();
        break;
    default:
        break;
    }
}


void HostNotebook::update()
{
    Trace trace(StringBuffer().format("HostNotebook@%zx::update", this));

    bool tabs = page_num(_optBox) > 0;

    Session& session = _host->getSession();
    if (session && session.isConnected())
    {
        if (!tabs)
        {
            insert_page(_genBox, Glib::ustring(gettext("Host")), 0);
            insert_page(_cpuBox, Glib::ustring(gettext("CPU")), 1);
            insert_page(_memBox, Glib::ustring(gettext("Memory")), 2);
            insert_page(_swvBox, Glib::ustring(gettext("Software")), 3);
            insert_page(_patBox, Glib::ustring(gettext("Patches")), 4);
            insert_page(_pfmBox, Glib::ustring(gettext("Performance")), 5);
            show_all_children();
        }
    }
    else
    {
        if (tabs)
        {
            remove_page(_genBox);
            remove_page(_cpuBox);
            remove_page(_memBox);
            remove_page(_swvBox);
            remove_page(_patBox);
            remove_page(_pfmBox);
        }
        return;
    }

    XenPtr<xen_host_record> record = _host->getRecord();
    if (record)
    {
        SetHostProperties(_genLv, record);

        if (record->cpu_info && record->cpu_info->size)
        {
            SetHostCpuProperties(_cpuLv, record);
        }
        else
        {
            _cpuLv.clear();
        }

        if (record->software_version && record->software_version->size)
        {
            SetHostSoftwareProperties(_swvLv, record);
        }
        else
        {
            _swvLv.clear();
        }

        _patLv.clear();
        std::list<RefPtr<PatchRecord> > patchList;
        if (_host->getPatchList(patchList))
        {
            for (std::list<RefPtr<PatchRecord> >::const_iterator i = patchList.begin(); i != patchList.end(); i++)
            {
                _patLv.set(**i);
            }
        }
    }
    else
    {
        _genLv.clear();
        _cpuLv.clear();
        _swvLv.clear();
        _patLv.clear();
    }

    XenPtr<xen_host_metrics_record> metricsRecord = _host->getMetricsRecord();
    if (metricsRecord)
    {
        SetHostMemoryProperties(_memLv, metricsRecord);
    }
    else
    {
        _memLv.clear();
    }
}


void HostNotebook::updatePerformaceStats()
{
    RefPtr<PerformanceMonitor> pm = _host->getSession().getStore().getPerformanceMonitor();

    for (;;)
    {
        PerformanceMonitor::ListEntry entry = pm->getEntry(_host->getUUID());
        if (!entry.first)
        {
            break;
        }
        StringBuffer key;
        PerformanceMonitor::Map::const_iterator i;

        _cpuGraph.addTime(entry.first);
        for (int j = 0; j < CpuGraph::kMaxElementCount; j++)
        {
            key.format("cpu%u", j);
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    double v = strtod(i->second.c_str(), NULL);
                    _cpuGraph.addValue(j, round(v * 100.0));
                }
                else
                {
                    _cpuGraph.addValue(j, 0);
                }
            }
            else
            {
                break;
            }
        }
        _cpuGraph.update();

        _memGraph.addTime(entry.first);
        double memory_total_kib, memory_free_kib;
        key.format("memory_total_kib");
        i = entry.second->find(key.str());
        if (i != entry.second->end())
        {
            if (i->second != "NaN")
            {
                memory_total_kib = strtod(i->second.c_str(), NULL);
            }
            else
            {
                memory_total_kib = 0.0;
            }
        }
        else
        {
            memory_total_kib = 0.0;
        }
        key.format("memory_free_kib");
        i = entry.second->find(key.str());
        if (i != entry.second->end())
        {
            if (i->second != "NaN")
            {
                memory_free_kib = strtod(i->second.c_str(), NULL);
            }
            else
            {
                memory_free_kib = 0.0;
            }
        }
        else
        {
            memory_free_kib = 0.0;
        }
        _memGraph.addValue(0, round(memory_total_kib / 1024.0));
        _memGraph.addValue(1, round((memory_total_kib - memory_free_kib) / 1024.0));
        _memGraph.update();

        _netGraph.addTime(entry.first);
        for (int j = 0; j < 10; j++)
        {
            double rx, tx;

            key.format("pif_eth%d_rx", j);
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    rx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    rx = 0.0;
                }
            }
            else
            {
                continue;
            }
            
            key.format("pif_eth%d_tx", j);
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    tx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    tx = 0.0;
                }
            }
            else
            {
                continue;
            }

            StringBuffer rxLabel, txLabel;
            rxLabel.format(gettext("eth%d receive"), j);
            txLabel.format(gettext("eth%d send"), j);
            _netGraph.addValue(j, rx, tx, rxLabel, txLabel);
        }
#if 0
        {
            double rx, tx;

            key.format("pif_lo_rx");
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    rx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    rx = 0.0;
                }
            }
            else
            {
                continue;
            }
            
            key.format("pif_lo_tx");
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    tx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    tx = 0.0;
                }
            }
            else
            {
                continue;
            }

            StringBuffer rxLabel, txLabel;
            rxLabel.format(gettext("Loopback receive"));
            txLabel.format(gettext("Loopback send"));
            _netGraph.addValue(10, rx, tx, rxLabel, txLabel);
        }
#endif
#if 0
        {
            double rx, tx;

            key.format("pif_aggr_rx");
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    rx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    rx = 0.0;
                }
            }
            else
            {
                continue;
            }
            
            key.format("pif_aggr_tx");
            i = entry.second->find(key.str());
            if (i != entry.second->end())
            {
                if (i->second != "NaN")
                {
                    tx = strtod(i->second.c_str(), NULL);
                }
                else
                {
                    tx = 0.0;
                }
            }
            else
            {
                continue;
            }

            StringBuffer rxLabel, txLabel;
            rxLabel.format(gettext("Total receive"));
            txLabel.format(gettext("Total send"));
            _netGraph.addValue(11, rx, tx, rxLabel, txLabel);
        }
#endif
        _netGraph.update();

        delete entry.second;
    }
}
