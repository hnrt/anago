// Copyright (C) 1999-2017 Hideaki Narita


#ifdef __GNUC__
#ifdef __x86_64__


#include <stdint.h>


int8_t InterlockedExchangeAdd(volatile int8_t* destination, int8_t value)
{
    int8_t oldValue;

    __asm__ __volatile__ ("lock; xaddb %1,(%2)"
                          : "=a" (oldValue)
                          : "a" (value), "d" (destination));

    return oldValue;
}


int16_t InterlockedExchangeAdd(volatile int16_t* destination, int16_t value)
{
    int16_t oldValue;

    __asm__ __volatile__ ("lock; xadd %1,(%2)"
                          : "=a" (oldValue)
                          : "a" (value), "d" (destination));

    return oldValue;
}


int32_t InterlockedExchangeAdd(volatile int32_t* destination, int32_t value)
{
    int32_t oldValue;

    __asm__ __volatile__ ("lock; xaddl %1,(%2)"
                          : "=a" (oldValue)
                          : "a" (value), "d" (destination));

    return oldValue;
}


int64_t InterlockedExchangeAdd(volatile int64_t* destination, int64_t value)
{
    int64_t oldValue;

    __asm__ __volatile__ ("lock; xaddq %1,(%2)"
                          : "=a" (oldValue)
                          : "a" (value), "d" (destination));

    return oldValue;
}


#endif //__x86_64__
#endif //__GNUC__
