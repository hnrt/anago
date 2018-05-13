// Copyright (C) 2012-2018 Hideaki Narita


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

        XenRef(T x = 0);
        ~XenRef();
        operator T() const;
        T* operator &();
        bool isNull() const;
        Glib::ustring toString() const;

    private:

        XenRef(const XenRef<T,F>&);
        void operator =(const XenRef<T,F>&);

        T _x;
    };

    template<typename T, typename F>
    XenRef<T, F>::XenRef(T x)
        : _x(x)
    {
    }

    template<typename T, typename F>
    XenRef<T, F>::~XenRef()
    {
        F::free(_x);
    }

    template<typename T, typename F>
    XenRef<T, F>::operator T() const
    {
        return _x ? _x : (T)NULLREFSTRING;
    }

    template<typename T, typename F>
    T* XenRef<T, F>::operator &()
    {
        F::free(_x);
        _x = 0;
        return &_x;
    }

    template<typename T, typename F>
    bool XenRef<T, F>::isNull() const
    {
        return !_x || !*(const char*)_x || !strcmp((const char*)_x, NULLREFSTRING);
    }

    template<typename T, typename F>
    Glib::ustring XenRef<T, F>::toString() const
    {
        return Glib::ustring((const char*)_x);
    }

#define XenRefFree(type) struct type##_free_t { static void free(type p) { if (p) type##_free(p); } }

    XenRefFree(xen_host);
    XenRefFree(xen_host_metrics);
    XenRefFree(xen_network);
    XenRefFree(xen_pool);
    XenRefFree(xen_pool_patch);
    XenRefFree(xen_pool_update);
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
