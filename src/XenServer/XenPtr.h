// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_XENPTR_H
#define HNRT_XENPTR_H


#include "Base/RefObj.h"
#include "XenServer/XenServer.h"


namespace hnrt
{
    template<typename T>
    class XenPtr
    {
    private:

        struct Container
            : public RefObj
        {
            T* _ptr;

            Container(T* p = 0)
                : _ptr(p)
            {
            }

            ~Container()
            {
                XenServer::free(_ptr);
            }
        };

        Container* _ctr;

    public:

        XenPtr(T* p = 0)
            : _ctr(0)
        {
            if (p)
            {
                _ctr = new Container(p);
            }
        }

        XenPtr(Container* ctr)
            : _ctr(ctr)
        {
            if (_ctr)
            {
                _ctr->reference();
            }
        }

        XenPtr(const XenPtr<T>& src)
            : _ctr(src._ctr)
        {
            if (_ctr)
            {
                _ctr->reference();
            }
        }

        ~XenPtr()
        {
            if (_ctr)
            {
                _ctr->unreference();
            }
        }

        operator const T*() const { return _ctr ? _ctr->_ptr : 0; }

        operator T*() { return _ctr ? _ctr->_ptr : 0; }

        const T* operator ->() const { return _ctr ? _ctr->_ptr : 0; }

        T* operator ->() { return _ctr ? _ctr->_ptr : 0; }

        T** address()
        // operator & doesn't work with std::vector
        {
            if (_ctr)
            {
                _ctr->unreference();
            }
            _ctr = new Container;
            return &_ctr->_ptr;
        }

        XenPtr<T>& operator =(const XenPtr<T>& src)
        {
            if (_ctr)
            {
                _ctr->unreference();
            }
            _ctr = src._ctr;
            if (_ctr)
            {
                _ctr->reference();
            }
            return *this;
        }

        Container* container() const
        {
            return _ctr;
        }
    };
}


#endif //!HNRT_XENPTR_H
