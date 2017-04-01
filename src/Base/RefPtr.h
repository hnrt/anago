// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_REFPTR_H
#define HNRT_REFPTR_H


namespace hnrt
{
    template<typename T>
    class RefPtr
    {
    public:

        RefPtr()
            : _ptr(0)
        {
        }

        RefPtr(T* ptr)
            : _ptr(ptr)
        {
        }

        RefPtr(T* ptr, int delta)
            : _ptr(ptr)
        {
            if (_ptr && delta > 0)
            {
                do
                {
                    _ptr->incRef();
                }
                while (--delta);
            }
        }

        RefPtr(const RefPtr<T>& src)
            : _ptr(src._ptr)
        {
            if (_ptr)
            {
                _ptr->incRef();
            }
        }

        ~RefPtr()
        {
            if (_ptr)
            {
                _ptr->decRef();
            }
        }

        const T* ptr() const
        {
            return _ptr;
        }

        T* ptr()
        {
            return _ptr;
        }

        operator const T*() const
        {
            return _ptr;
        }

        operator T*()
        {
            return _ptr;
        }

        const T* operator ->() const
        {
            return _ptr;
        }

        T* operator ->()
        {
            return _ptr;
        }

        const T& operator *() const
        {
            return *_ptr;
        }

        T& operator *()
        {
            return *_ptr;
        }

        RefPtr<T>& operator =(const RefPtr<T>& src)
        {
            if (_ptr)
            {
                _ptr->decRef();
            }
            _ptr = src._ptr;
            if (_ptr)
            {
                _ptr->incRef();
            }
            return *this;
        }

        bool operator ==(const RefPtr<T>& rhs) const
        {
            return _ptr == rhs._ptr;
        }

        bool operator !=(const RefPtr<T>& rhs) const
        {
            return _ptr != rhs._ptr;
        }

        template<typename U> static RefPtr<T> castStatic(const RefPtr<U>& src)
        {
            RefPtr<T> instance(static_cast<T*>(const_cast<U*>(src.ptr())));
            if (instance._ptr)
            {
                instance._ptr->incRef();
            }
            return instance;
        }

        void reset()
        {
            if (_ptr)
            {
                _ptr->decRef();
                _ptr = 0;
            }
        }

    private:

        T* _ptr;
    };
}


#endif //!HNRT_REFPTR_H
