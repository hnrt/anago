// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_REFOBJ_H
#define HNRT_REFOBJ_H


namespace hnrt
{
    class RefObj
    {
    public:

        RefObj();
        virtual ~RefObj();
        virtual void incRef() const;
        virtual void decRef() const;
        int refCount() const { return _refCount; }

    private:

        RefObj(const RefObj&);
        void operator =(const RefObj&);

        volatile int _refCount;
    };
}


#endif //!HNRT_REFOBJ_H
