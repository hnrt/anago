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
                    _ptr->reference();
                }
                while (--delta);
            }
        }

        RefPtr(const RefPtr<T>& src)
            : _ptr(src._ptr)
        {
            if (_ptr)
            {
                _ptr->reference();
            }
        }

        ~RefPtr()
        {
            if (_ptr)
            {
                _ptr->unreference();
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
                _ptr->unreference();
            }
            _ptr = src._ptr;
            if (_ptr)
            {
                _ptr->reference();
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
            RefPtr<T> instance(static_cast<T*>(src._ptr));
            if (instance._ptr)
            {
                instance._ptr->reference();
            }
            return instance;
        }

    private:

        T* _ptr;
    };
}


#endif //!HNRT_REFPTR_H
