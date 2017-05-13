// Copyright (C) 2012-2017 Hideaki Narita


#ifndef HNRT_VIRTUALMACHINEOPSTATE_H
#define HNRT_VIRTUALMACHINEOPSTATE_H


namespace hnrt
{
    class VirtualMachineOperationState
    {
    public:

        enum Value
        {
            UNDEFINED,
            IMPORT_PENDING,
            IMPORT_INPROGRESS,
            IMPORT_SUCCESS,
            IMPORT_FAILURE,
            IMPORT_CANCELING,
            IMPORT_CANCELED,
            EXPORT_PENDING,
            EXPORT_INPROGRESS,
            EXPORT_SUCCESS,
            EXPORT_FAILURE,
            EXPORT_CANCELING,
            EXPORT_CANCELED,
            EXPORT_VERIFY_PENDING,
            EXPORT_VERIFY_INPROGRESS,
            EXPORT_VERIFY_SUCCESS,
            EXPORT_VERIFY_FAILURE,
            EXPORT_VERIFY_CANCELING,
            EXPORT_VERIFY_CANCELED,
            VERIFY_PENDING,
            VERIFY_INPROGRESS,
            VERIFY_SUCCESS,
            VERIFY_FAILURE,
            VERIFY_CANCELING,
            VERIFY_CANCELED,
        };

        inline VirtualMachineOperationState(Value = UNDEFINED);
        inline VirtualMachineOperationState(const VirtualMachineOperationState&);
        inline VirtualMachineOperationState& operator =(const VirtualMachineOperationState&);
        inline Value value() const;
        inline operator Value() const;
        inline bool isActive() const;
        inline bool isInactive() const;
        inline bool isPending() const;
        inline bool isCanceling() const;
        inline Value canceling() const;

    protected:

        Value _value;
    };

    inline VirtualMachineOperationState::VirtualMachineOperationState(Value value)
        : _value(value)
    {
    }

    inline VirtualMachineOperationState::VirtualMachineOperationState(const VirtualMachineOperationState& src)
        : _value(src._value)
    {
    }

    inline VirtualMachineOperationState& VirtualMachineOperationState::operator =(const VirtualMachineOperationState& src)
    {
        _value = src._value;
        return *this;
    }

    inline VirtualMachineOperationState::Value VirtualMachineOperationState::value() const
    {
        return _value;
    }

    inline VirtualMachineOperationState::operator Value() const
    {
        return _value;
    }

    inline bool VirtualMachineOperationState::isActive() const
    {
        switch (_value)
        {
        case IMPORT_PENDING:
        case IMPORT_INPROGRESS:
        case IMPORT_CANCELING:
        case EXPORT_PENDING:
        case EXPORT_INPROGRESS:
        case EXPORT_CANCELING:
        case EXPORT_VERIFY_PENDING:
        case EXPORT_VERIFY_INPROGRESS:
        case EXPORT_VERIFY_CANCELING:
        case VERIFY_PENDING:
        case VERIFY_INPROGRESS:
        case VERIFY_CANCELING:
            return true;
        default:
            return false;
        }
    }

    inline bool VirtualMachineOperationState::isInactive() const
    {
        return !isActive();
    }

    inline bool VirtualMachineOperationState::isPending() const
    {
        switch (_value)
        {
        case IMPORT_PENDING:
        case EXPORT_PENDING:
        case EXPORT_VERIFY_PENDING:
        case VERIFY_PENDING:
            return true;
        default:
            return false;
        }
    }

    inline bool VirtualMachineOperationState::isCanceling() const
    {
        switch (_value)
        {
        case IMPORT_CANCELING:
        case EXPORT_CANCELING:
        case EXPORT_VERIFY_CANCELING:
        case VERIFY_CANCELING:
            return true;
        default:
            return false;
        }
    }

    inline VirtualMachineOperationState::Value VirtualMachineOperationState::canceling() const
    {
        switch (_value)
        {
        case IMPORT_PENDING:
        case IMPORT_INPROGRESS:
            return IMPORT_CANCELING;
        case EXPORT_PENDING:
        case EXPORT_INPROGRESS:
            return EXPORT_CANCELING;
        case EXPORT_VERIFY_PENDING:
        case EXPORT_VERIFY_INPROGRESS:
            return EXPORT_VERIFY_CANCELING;
        case VERIFY_PENDING:
        case VERIFY_INPROGRESS:
            return VERIFY_CANCELING;
        default:
            return _value;
        }
    }
}


#endif //!HNRT_VIRTUALMACHINEOPSTATE_H
