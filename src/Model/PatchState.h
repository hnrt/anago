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
            CLEANED,
        };

        inline PatchState(Value = UNDEFINED);
        inline PatchState(const PatchState&);
        inline PatchState& operator =(const PatchState&);
        inline Value value() const;
        inline operator Value() const;

    protected:

        Value _value;
    };

    inline PatchState::PatchState(Value value)
        : _value(value)
    {
    }

    inline PatchState::PatchState(const PatchState& src)
        : _value(src._value)
    {
    }

    inline PatchState& PatchState::operator =(const PatchState& src)
    {
        _value = src._value;
        return *this;
    }

    inline PatchState::Value PatchState::value() const
    {
        return _value;
    }

    inline PatchState::operator PatchState::Value() const
    {
        return _value;
    }
}


#endif //!HNRT_PATCHSTATE_H
