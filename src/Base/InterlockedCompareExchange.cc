// Copyright (C) 1999-2018 Hideaki Narita


#ifdef __GNUC__
#ifdef __x86_64__


#include <stdint.h>


int8_t InterlockedCompareExchange(volatile int8_t* destination, int8_t value, int8_t comparand)
{
    int8_t oldValue;

    __asm__ __volatile__ ("lock; cmpxchgb %2,(%3)"
                          : "=a" (oldValue)
                          : "a" (comparand), "c" (value), "d" (destination));

    return oldValue;
}


int16_t InterlockedCompareExchange(volatile int16_t* destination, int16_t value, int16_t comparand)
{
    int16_t oldValue;

    __asm__ __volatile__ ("lock; cmpxchg %2,(%3)"
                          : "=a" (oldValue)
                          : "a" (comparand), "c" (value), "d" (destination));

    return oldValue;
}


int32_t InterlockedCompareExchange(volatile int32_t* destination, int32_t value, int32_t comparand)
{
    int32_t oldValue;

    __asm__ __volatile__ ("lock; cmpxchgl %2,(%3)"
                          : "=a" (oldValue)
                          : "a" (comparand), "c" (value), "d" (destination));

    return oldValue;
}


int64_t InterlockedCompareExchange(volatile int64_t* destination, int64_t value, int64_t comparand)
{
    int64_t oldValue;

    __asm__ __volatile__ ("lock; cmpxchgq %2,(%3)"
                          : "=a" (oldValue)
                          : "a" (comparand), "c" (value), "d" (destination));

    return oldValue;
}


#endif //__x86_64__
#endif //__GNUC__
