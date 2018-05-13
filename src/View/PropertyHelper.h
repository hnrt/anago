// Copyright (C) 2012-2018 Hideaki Narita


#ifndef HNRT_PROPERTYHELPER_H
#define HNRT_PROPERTYHELPER_H


struct xen_host_record;
struct xen_host_metrics_record;
struct xen_network_record;
struct xen_pbd_record;
struct xen_pif_record;
struct xen_sr_record;
struct xen_vbd_record;
struct xen_vdi_record;
struct xen_vif_record;
struct xen_vm_record;
struct xen_vm_guest_metrics_record;
struct xen_vm_metrics_record;


namespace hnrt
{
    class NameValueListView;
    class PatchListView;
    struct PatchRecord;

    void SetHostProperties(NameValueListView&, const xen_host_record*);
    void SetHostCpuProperties(NameValueListView&, const xen_host_record*);
    void SetHostMemoryProperties(NameValueListView&, const xen_host_metrics_record*);
    void SetHostSoftwareProperties(NameValueListView&, const xen_host_record*);
    void SetNetworkProperties(NameValueListView&, const xen_network_record*);
    void SetPbdProperties(NameValueListView&, const xen_pbd_record*);
    void SetPifProperties(NameValueListView&, const xen_pif_record*);
    void SetSrProperties(NameValueListView&, const xen_sr_record*);
    void SetVbdProperties(NameValueListView&, const xen_vbd_record*);
    void SetVdiProperties(NameValueListView&, const xen_vdi_record*);
    void SetVifProperties(NameValueListView&, const xen_vif_record*, const xen_vm_guest_metrics_record*);
    void SetVmProperties(NameValueListView&, const xen_vm_record*, const xen_vm_guest_metrics_record*);
    void SetVmMemoryProperties(NameValueListView&, const xen_vm_record*, const xen_vm_metrics_record*);
}


#endif //!HNRT_PROPERTYHELPER_H
