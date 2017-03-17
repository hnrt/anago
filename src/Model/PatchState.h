// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_PATCHSTATE_H
#define HNRT_PATCHSTATE_H


namespace hnrt
{
    class PatchState
    {
    public:

        enum Value
        {
            UNDEFINED,
            AVAILABLE,
            DOWNLOAD_PENDING,
            DOWNLOAD_INPROGRESS,
            DOWNLOAD_FAILURE,
            DOWNLOADED,
            UPLOAD_PENDING,
            UPLOAD_INPROGRESS,
            UPLOAD_FAILURE,
            UPLOADED,
            APPLY_INPROGRESS,
            APPLY_FAILURE,
            APPLIED,
            CLEAN_INPROGRESS,
            CLEAN_FAILURE,
        };

        PatchState(Value value = UNDEFINED)
            : _value(value)
        {
        }

        PatchState(const PatchState& src)
            : _value(src._value)
        {
        }

        PatchState& operator =(const PatchState& src)
        {
            _value = src._value;
            return *this;
        }

        Value value() const
        {
            return _value;
        }

        operator Value() const
        {
            return _value;
        }

    protected:

        Value _value;
    };
}


#endif //!HNRT_PATCHSTATE_H
