// Copyright (C) 2012-2017 Hideaki Narita


#include <stdlib.h>
#include <libintl.h>
#include "Base/StringBuffer.h"
#include "Model/Model.h"
#include "Model/PatchRecord.h"
#include "Model/PatchState.h"
#include "XenServer/XenServer.h"
#include "NameValueListView.h"
#include "PatchListView.h"
#include "PropertyHelper.h"


using namespace hnrt;


void hnrt::SetHostProperties(NameValueListView& listView, const xen_host_record* record)
{
    listView.set("UUID", record->uuid);
    listView.set("Label", record->name_label);
    listView.set("Description", record->name_description);
    listView.set("Host name", record->hostname);
    listView.set("Address", record->address);
    listView.set("Enabled", record->enabled);
    listView.set("API version (major)", record->api_version_major);
    listView.set("API version (minor)", record->api_version_minor);
    listView.set("API version (vendor)", record->api_version_vendor);
    listView.set("other-config", record->other_config);
    listView.set("cpu-config", record->cpu_configuration);
    listView.set("chipset-info", record->chipset_info, true);
}


void hnrt::SetHostCpuProperties(NameValueListView& listView, const xen_host_record* record)
{
    if (record->cpu_info && record->cpu_info->size)
    {
        bool endOfList = false;
        for (size_t i = 0; !endOfList; i++)
        {
            endOfList = i == record->cpu_info->size - 1 ? true : false;
            listView.set(record->cpu_info->contents[i].key, record->cpu_info->contents[i].val, endOfList);
        }
    }
}


void hnrt::SetHostMemoryProperties(NameValueListView& listView, const xen_host_metrics_record* record)
{
    listView.set("Total", record->memory_total);
    listView.set("Free", record->memory_free);
}


void hnrt::SetHostSoftwareProperties(NameValueListView& listView, const xen_host_record* record)
{
    if (record->software_version && record->software_version->size)
    {
        bool endOfList = false;
        for (size_t i = 0; !endOfList; i++)
        {
            endOfList = i == record->software_version->size - 1 ? true : false;
            listView.set(record->software_version->contents[i].key, record->software_version->contents[i].val, endOfList);
        }
    }
}


void hnrt::SetNetworkProperties(NameValueListView& listView, const xen_network_record* record)
{
    listView.set("UUID", record->uuid);
    listView.set("Label", record->name_label);
    listView.set("Description", record->name_description);
    listView.set("MTU", record->mtu);
    listView.set("bridge", record->bridge);
    listView.set("other-config", record->other_config, true);
    listView.set("default_locking_mode", xen_network_default_locking_mode_to_string(record->default_locking_mode));
}


void hnrt::SetPbdProperties(NameValueListView& listView, const xen_pbd_record* record)
{
    listView.set("UUID", record->uuid);
    listView.set("device-config", record->device_config);
    listView.set("currently-attached", record->currently_attached, true);
}


void hnrt::SetPifProperties(NameValueListView& listView, const xen_pif_record* record)
{
    listView.set("UUID", record->uuid);
    listView.set("Device", record->device);
    listView.set("MAC", record->mac);
    listView.set("MTU", record->mtu);
    listView.set("VLAN", record->vlan);
    listView.set("physical", record->physical);
    listView.set("currently-attached", record->currently_attached);
    listView.set("IP", record->ip);
    listView.set("Netmask", record->netmask);
    listView.set("Gateway", record->gateway);
    listView.set("DNS", record->dns);
    listView.set("management", record->management);
    listView.set("other-config", record->other_config);
    listView.set("disallow_unplug", record->disallow_unplug);
    listView.set("IPv6-configuration-mode", xen_ipv6_configuration_mode_to_string(record->ipv6_configuration_mode));
    listView.set("IPv6", record->ipv6);
    listView.set("IPv6-gateway", record->ipv6_gateway);
    listView.set("primary-address-type", xen_primary_address_type_to_string(record->primary_address_type), true);
}


void hnrt::SetSrProperties(NameValueListView& listView, const xen_sr_record* record)
{
    listView.set("UUID", record->uuid);
    listView.set("Label", record->name_label);
    listView.set("Description", record->name_description);
    listView.set("virtual_allocation", record->virtual_allocation);
    listView.set("physical_utilisation", record->physical_utilisation);
    listView.set("physical_size", record->physical_size);
    listView.set("type", record->type);
    listView.set("content_type", record->content_type);
    listView.set("shared", record->shared);
    listView.set("other-config", record->other_config);
    listView.set("sm-config", record->sm_config);
    listView.set("local_cache_enabled", record->local_cache_enabled, true);
}


void hnrt::SetVbdProperties(NameValueListView& listView, const xen_vbd_record* record)
{
    listView.set("UUID", record->uuid);
    listView.set("Type", xen_vbd_type_to_string(record->type));
    listView.set("Device", record->device);
    listView.set("Userdevice", record->userdevice);
    listView.set("bootable", record->bootable);
    listView.set("mode", xen_vbd_mode_to_string(record->mode));
    listView.set("unpluggable", record->unpluggable);
    listView.set("storage-lock", record->storage_lock);
    listView.set("empty", record->empty);
    listView.set("currently-attached", record->currently_attached);
    listView.set("other-config", record->other_config);
    listView.set("status-code", record->status_code);
    listView.set("status-detail", record->status_detail, true);
}


void hnrt::SetVdiProperties(NameValueListView& listView, const xen_vdi_record* record)
{
    listView.set("UUID", record->uuid);
    listView.set("Label", record->name_label);
    listView.set("Description", record->name_description);
    listView.set("Type", xen_vdi_type_to_string(record->type));
    listView.set("virtual-size", record->virtual_size);
    listView.set("physical-utilisation", record->physical_utilisation);
    listView.set("location", record->location);
    listView.set("sharable", record->sharable);
    listView.set("read-only", record->read_only);
    listView.set("storage-lock", record->storage_lock);
    listView.set("managed", record->managed);
    listView.set("missing", record->missing);
    listView.set("other-config", record->other_config);
    listView.set("xenstore-data", record->xenstore_data);
    listView.set("sm-config", record->sm_config);
    listView.set("allow_caching", record->allow_caching, true);
}


void hnrt::SetVifProperties(NameValueListView& listView, const xen_vif_record* vifRecord, const xen_vm_guest_metrics_record* guestMetricsRecord)
{
    listView.set("UUID", vifRecord->uuid);
    listView.set("Device", vifRecord->device);
    listView.set("MAC", vifRecord->mac);
    listView.set("IPv4", XenServer::getIp(guestMetricsRecord, atoi(vifRecord->device)));
    listView.set("IPv6", XenServer::getIpv6(guestMetricsRecord, atoi(vifRecord->device)));
    listView.set("MTU", vifRecord->mtu);
    listView.set("currently-attached", vifRecord->currently_attached);
    listView.set("status_code", vifRecord->status_code);
    if (vifRecord->status_detail && *vifRecord->status_detail)
    {
        listView.set("status_detail", vifRecord->status_detail);
    }
    listView.set("mac_autogenerated", vifRecord->mac_autogenerated);
    listView.set("other-config", vifRecord->other_config);
    listView.set("runtime-properties", vifRecord->runtime_properties);
    if (vifRecord->qos_algorithm_type && *vifRecord->qos_algorithm_type)
    {
        listView.set("qos_algorithm_type", vifRecord->qos_algorithm_type);
    }
    listView.set("qos_algorithm_params", vifRecord->qos_algorithm_params);
    listView.set("locking_mode", xen_vif_locking_mode_to_string(vifRecord->locking_mode));
}


void hnrt::SetVmProperties(NameValueListView& listView, const xen_vm_record* record, const xen_vm_guest_metrics_record* guestMetricsRecord)
{
    listView.set("UUID", record->uuid);
    listView.set("Label", record->name_label);
    listView.set("Description", record->name_description);
    listView.set("power_state", xen_vm_power_state_to_string(record->power_state));
    listView.set("vcpus_max", record->vcpus_max);
    listView.set("vcpus_at_startup", record->vcpus_at_startup);
    listView.set("hvm_shadow_multiplier", record->hvm_shadow_multiplier);
    listView.set("pv_bootloader", record->pv_bootloader);
    listView.set("pv_kernel", record->pv_kernel);
    listView.set("pv_ramdisk", record->pv_ramdisk);
    listView.set("pv_args", record->pv_args);
    listView.set("pv_bootloader_args", record->pv_bootloader_args);
    listView.set("pv_legacy_args", record->pv_legacy_args);
    listView.set("hvm_boot_policy", record->hvm_boot_policy);
    listView.set("platform", record->platform);
    listView.set("other-config", record->other_config);
    listView.set("recommendations", record->recommendations, true);
    if (guestMetricsRecord)
    {
        listView.set("OS", XenServer::getOs(guestMetricsRecord));
    }
    else
    {
        listView.set("OS", gettext("(unknown)"));
    }
}


void hnrt::SetVmMemoryProperties(NameValueListView& listView, const xen_vm_record* record, const xen_vm_metrics_record* metricsRecord)
{
    listView.set("memory_static_max", record->memory_static_max);
    listView.set("memory_dynamic_max", record->memory_dynamic_max);
    listView.set("memory_dynamic_min", record->memory_dynamic_min);
    listView.set("memory_static_min", record->memory_static_min);
    listView.set("memory_actual", metricsRecord ? metricsRecord->memory_actual : 0);
}
