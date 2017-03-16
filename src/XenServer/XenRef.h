// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENREF_H
#define HNRT_XENREF_H


#include <string.h>
#include <glibmm.h>
#include "XenServer/Api.h"
#include "XenServer/Constants.h"


namespace hnrt
{
    template<typename T, typename F>
    class XenRef
    {
    public:

        XenRef(T x = 0)
            : _x(x)
        {
        }

        ~XenRef()
        {
            F::free(_x);
        }

        operator T() const { return _x ? _x : (T)NULLREFSTRING; }

        T* operator &() { F::free(_x); _x = 0; return &_x; }

        bool isNull() const { return !_x || !strcmp((const char*)_x, NULLREFSTRING); }

        Glib::ustring toString() const { return Glib::ustring((const char*)_x); }

    private:

        XenRef(const XenRef<T,F>&);
        void operator =(const XenRef<T,F>&);

        T _x;
    };

#define XenRefFree(type) struct type##_free_t { static void free(type p) { if (p) type##_free(p); } }

    XenRefFree(xen_host);
    XenRefFree(xen_host_metrics);
    XenRefFree(xen_network);
    XenRefFree(xen_pool);
    XenRefFree(xen_pool_patch);
    XenRefFree(xen_pbd);
    XenRefFree(xen_pif);
    XenRefFree(xen_secret);
    XenRefFree(xen_sr);
    XenRefFree(xen_task);
    XenRefFree(xen_vbd);
    XenRefFree(xen_vdi);
    XenRefFree(xen_vif);
    XenRefFree(xen_vm);
    XenRefFree(xen_vm_guest_metrics);
    XenRefFree(xen_vm_metrics);
}


#endif //!HNRT_XENREF_H
