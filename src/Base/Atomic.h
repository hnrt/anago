// Copyright (C) 1999-2017 Hideaki Narita


#ifndef HNRT_ATOMIC_H
#define HNRT_ATOMIC_H
#ifdef __GNUC__
#ifdef __x86_64__


#include <stddef.h>
#include <stdint.h>


int8_t InterlockedCompareExchange(volatile int8_t* destination, int8_t value, int8_t comparand);
int16_t InterlockedCompareExchange(volatile int16_t* destination, int16_t value, int16_t comparand);
int32_t InterlockedCompareExchange(volatile int32_t* destination, int32_t value, int32_t comparand);
int64_t InterlockedCompareExchange(volatile int64_t* destination, int64_t value, int64_t comparand);
int8_t InterlockedExchange(volatile int8_t* destination, int8_t value);
int16_t InterlockedExchange(volatile int16_t* destination, int16_t value);
int32_t InterlockedExchange(volatile int32_t* destination, int32_t value);
int64_t InterlockedExchange(volatile int64_t* destination, int64_t value);
int8_t InterlockedExchangeAdd(volatile int8_t* destination, int8_t value);
int16_t InterlockedExchangeAdd(volatile int16_t* destination, int16_t value);
int32_t InterlockedExchangeAdd(volatile int32_t* destination, int32_t value);
int64_t InterlockedExchangeAdd(volatile int64_t* destination, int64_t value);


// Note that InterlockedIncrement returns the resulting value of the increment, not the previous value.
// Likewise, InterlockedDecrement returns the resulting value of the decrement, not the previous value.
#define InterlockedIncrement(destination) (InterlockedExchangeAdd(destination,1)+1)
#define InterlockedDecrement(destination) (InterlockedExchangeAdd(destination,-1)-1)


template<typename T> T* InterlockedCompareExchangePointer(T** d, T* v, T* c)
{
#if __SIZEOF_POINTER__ == 8
    return reinterpret_cast<T*>(InterlockedCompareExchange(reinterpret_cast<volatile int64_t*>(d),reinterpret_cast<int64_t>(v),reinterpret_cast<int64_t>(c)));
#elif __SIZEOF_POINTER__ == 4
    return reinterpret_cast<T*>(InterlockedCompareExchange(reinterpret_cast<volatile int32_t*>(d),reinterpret_cast<int32_t>(v),reinterpret_cast<int32_t>(c)));
#else
#error "__SIZEOF_POINTER__ is not what I expect."
#endif
}


template<typename T> T* InterlockedExchangePointer(T** d, T* v)
{
#if __SIZEOF_POINTER__ == 8
    return reinterpret_cast<T*>(InterlockedExchange(reinterpret_cast<volatile int64_t*>(d),reinterpret_cast<int64_t>(v)));
#elif __SIZEOF_POINTER__ == 4
    return reinterpret_cast<T*>(InterlockedExchange(reinterpret_cast<volatile int32_t*>(d),reinterpret_cast<int32_t>(v)));
#else
#error "__SIZEOF_POINTER__ is not what I expect."
#endif
}


#endif //__x86_64__
#endif //__GNUC__
#endif //!HNRT_ATOMIC_H
