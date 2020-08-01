/********************************************************************/
/*                                                                  */
/*  int_rtl.c     Primitive actions for the integer type.           */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/int_rtl.c                                       */
/*  Changes: 1992, 1993, 1994, 2000, 2005, 2010  Thomas Mertes      */
/*  Content: Primitive actions for the integer type.                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "tim_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "int_rtl.h"


#define UINT_BITS(A)                     ((A) & UINTTYPE_MAX)
#define UINT_BITS_WITHOUT_HIGHEST_BIT(A) ((A) & (UINTTYPE_MAX >> 1))
#define UINT_HIGHEST_BIT(A)              ((A) >> (INTTYPE_SIZE - 1))
#define UINT_LOWER_HALVE_BITS_SET        (UINTTYPE_MAX >> (INTTYPE_SIZE >> 1))
#define LOWER_HALVE_OF_UINT(A)           ((A) & UINT_LOWER_HALVE_BITS_SET)
#define UPPER_HALVE_OF_UINT(A)           ((A) >> (INTTYPE_SIZE >> 1))


#if   INTTYPE_SIZE == 32
#define DECIMAL_DIGITS(num)                    \
  (num < INT_SUFFIX(100000000) ?               \
      (num < INT_SUFFIX(10000) ?               \
        (num < INT_SUFFIX(100) ?               \
          (num < INT_SUFFIX(10) ? 1 : 2)       \
        :                                      \
          (num < INT_SUFFIX(1000) ? 3 : 4)     \
        )                                      \
      :                                        \
        (num < INT_SUFFIX(1000000) ?           \
          (num < INT_SUFFIX(100000) ? 5 : 6)   \
        :                                      \
          (num < INT_SUFFIX(10000000) ? 7 : 8) \
        )                                      \
      )                                        \
    :                                          \
      (num < INT_SUFFIX(1000000000) ? 9 : 10)  \
    )
#elif INTTYPE_SIZE == 64
#define DECIMAL_DIGITS(num) \
    (num < INT_SUFFIX(10000000000000000) ?                  \
      (num < INT_SUFFIX(100000000) ?                        \
        (num < INT_SUFFIX(10000) ?                          \
          (num < INT_SUFFIX(100) ?                          \
            (num < INT_SUFFIX(10) ? 1 : 2)                  \
          :                                                 \
            (num < INT_SUFFIX(1000) ? 3 : 4)                \
          )                                                 \
        :                                                   \
          (num < INT_SUFFIX(1000000) ?                      \
            (num < INT_SUFFIX(100000) ? 5 : 6)              \
          :                                                 \
            (num < INT_SUFFIX(10000000) ? 7 : 8)            \
          )                                                 \
        )                                                   \
      :                                                     \
        (num < INT_SUFFIX(1000000000000) ?                  \
          (num < INT_SUFFIX(10000000000) ?                  \
            (num < INT_SUFFIX(1000000000) ? 9 : 10)         \
          :                                                 \
            (num < INT_SUFFIX(100000000000) ? 11 : 12)      \
          )                                                 \
        :                                                   \
          (num < INT_SUFFIX(100000000000000) ?              \
            (num < INT_SUFFIX(10000000000000) ? 13 : 14)    \
          :                                                 \
            (num < INT_SUFFIX(1000000000000000) ? 15 : 16)  \
          )                                                 \
        )                                                   \
      )                                                     \
    :                                                       \
      (num < INT_SUFFIX(1000000000000000000) ?              \
        (num < INT_SUFFIX(100000000000000000) ? 17 : 18)    \
      :                                                     \
        (num < INT_SUFFIX(10000000000000000000U) ? 19 : 20) \
      )                                                     \
    )
#endif


static const int most_significant[] = {
   -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
  };

static const int least_significant[] = {
   -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
  };

static const char lcDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char ucDigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const const_cstritype digitTable[] = {lcDigits, ucDigits};



void uint_mult (uinttype a, uinttype b, uinttype *c_high, uinttype *c_low)

  {
    uinttype a1;
    uinttype a2;
    uinttype b1;
    uinttype b2;
    uinttype c1;
    uinttype c2;
    uinttype c3;
    uinttype c4;

  /* uint_mult */
#ifdef TRACE_RANDOM
    printf("BEGIN uint_mult(%08x, %08x%08x)\n",
        (unsigned int) a, (unsigned int) b);
#endif
    a1 = LOWER_HALVE_OF_UINT(a);
    a2 = UPPER_HALVE_OF_UINT(a);
    b1 = LOWER_HALVE_OF_UINT(b);
    b2 = UPPER_HALVE_OF_UINT(b);
    c1 = UPPER_HALVE_OF_UINT(a1 * b1);
    c2 = a1 * b2;
    c3 = a2 * b1;
    c4 = UPPER_HALVE_OF_UINT(c1 + LOWER_HALVE_OF_UINT(c2) + LOWER_HALVE_OF_UINT(c3)) +
        UPPER_HALVE_OF_UINT(c2) + UPPER_HALVE_OF_UINT(c3) +
        a2 * b2;
    *c_low = UINT_BITS(a * b);
    *c_high = UINT_BITS(c4);
#ifdef TRACE_RANDOM
    printf("END uint_mult ==> %08x%08x\n",
        (unsigned int) *c_high, (unsigned int) *c_low);
#endif
  } /* uint_mult */



static INLINE void uint2_mult (uinttype a_high, uinttype a_low,
    uinttype b_high, uinttype b_low,
    uinttype *c_high, uinttype *c_low)

  {
    uinttype a_low1;
    uinttype a_low2;
    uinttype b_low1;
    uinttype b_low2;
    uinttype c1;
    uinttype c2;
    uinttype c3;
    uinttype c4;
    uinttype c5;

  /* uint2_mult */
#ifdef TRACE_RANDOM
    printf("BEGIN uint2_mult(%08x%08x, %08x%08x)\n",
        (unsigned int) a_high, (unsigned int) a_low,
        (unsigned int) b_high, (unsigned int) b_low);
#endif
    a_low1 = LOWER_HALVE_OF_UINT(a_low);
    a_low2 = UPPER_HALVE_OF_UINT(a_low);
    b_low1 = LOWER_HALVE_OF_UINT(b_low);
    b_low2 = UPPER_HALVE_OF_UINT(b_low);
    c1 = a_low1 * b_low1;
    c2 = a_low1 * b_low2;
    c3 = a_low2 * b_low1;
    c4 = UPPER_HALVE_OF_UINT(c1) + LOWER_HALVE_OF_UINT(c2) + LOWER_HALVE_OF_UINT(c3);
    c5 = UPPER_HALVE_OF_UINT(c4) + UPPER_HALVE_OF_UINT(c2) + UPPER_HALVE_OF_UINT(c3) +
        a_low2 * b_low2;
    *c_low = UINT_BITS(a_low * b_low);
    *c_high = UINT_BITS(a_low * b_high + a_high * b_low + c5);
#ifdef TRACE_RANDOM
    printf("END uint2_mult ==> %08x%08x\n",
        (unsigned int) *c_high, (unsigned int) *c_low);
#endif
  } /* uint2_mult */



static INLINE void uint2_add (uinttype a_high, uinttype a_low,
    uinttype b_high, uinttype b_low,
    uinttype *c_high, uinttype *c_low)

  { /* uint2_add */
#ifdef TRACE_RANDOM
    printf("BEGIN uint2_add(%08x%08x, %08x%08x)\n",
        (unsigned int) a_high, (unsigned int) a_low,
        (unsigned int) b_high, (unsigned int) b_low);
#endif
    *c_low = UINT_BITS(a_low + b_low);
    if (UINT_HIGHEST_BIT(a_low) + UINT_HIGHEST_BIT(b_low) +
        UINT_HIGHEST_BIT(UINT_BITS_WITHOUT_HIGHEST_BIT(a_low) +
                         UINT_BITS_WITHOUT_HIGHEST_BIT(b_low)) >= 2) {
      *c_high = UINT_BITS(a_high + b_high + 1);
    } else {
      *c_high = UINT_BITS(a_high + b_high);
    } /* if */
#ifdef TRACE_RANDOM
    printf("END uint2_add ==> %08x%08x\n",
        (unsigned int) *c_high, (unsigned int) *c_low);
#endif
  } /* uint2_add */



uinttype uint_rand (void)

  {
    static booltype seed_necessary = TRUE;
    static uinttype low_seed;
    static uinttype high_seed;

  /* uint_rand */
#ifdef TRACE_RANDOM
    printf("BEGIN uint_rand\n");
#endif
    if (unlikely(seed_necessary)) {
      uinttype micro_sec = (uinttype) timMicroSec();

      high_seed = (uinttype) time(NULL);
      high_seed = high_seed ^ (high_seed << 16);
      low_seed = (uinttype) clock();
      low_seed = (low_seed ^ (low_seed << 16)) ^ high_seed;
      /* printf("%10lo %010lo seed\n", (long unsigned) high_seed, (long unsigned) low_seed); */
      high_seed ^= micro_sec ^ micro_sec << 8 ^ micro_sec << 16 ^ micro_sec << 24;
#if INTTYPE_SIZE >= 64
      high_seed ^= micro_sec << 32 ^ micro_sec << 40 ^ micro_sec << 48 ^ micro_sec << 56;
#endif
      low_seed ^= micro_sec ^ micro_sec << 8 ^ micro_sec << 16 ^ micro_sec << 24;
#if INTTYPE_SIZE >= 64
      low_seed ^= micro_sec << 32 ^ micro_sec << 40 ^ micro_sec << 48 ^ micro_sec << 56;
#endif
      /* printf("%10lo %010lo seed\n", (long unsigned) high_seed, (long unsigned) low_seed); */
      seed_necessary = FALSE;
    } /* if */
#if INTTYPE_SIZE == 32
    /* SEED = SEED * 1103515245 + 12345 */
    uint2_mult(high_seed, low_seed, (uinttype) INT_SUFFIX(0), (uinttype) INT_SUFFIX(1103515245),
        &high_seed, &low_seed);
    uint2_add(high_seed, low_seed, (uinttype) INT_SUFFIX(0), (uinttype) INT_SUFFIX(12345),
        &high_seed, &low_seed);
#elif INTTYPE_SIZE == 64
    /* SEED = SEED * 6364136223846793005 + 1442695040888963407 */
    uint2_mult(high_seed, low_seed, (uinttype) INT_SUFFIX(0), (uinttype) INT_SUFFIX(6364136223846793005),
        &high_seed, &low_seed);
    uint2_add(high_seed, low_seed, (uinttype) INT_SUFFIX(0), (uinttype) INT_SUFFIX(1442695040888963407),
        &high_seed, &low_seed);
#endif
#ifdef TRACE_RANDOM
    printf("END uint_rand ==> %08x\n", (unsigned int) high_seed);
#endif
    return high_seed;
  } /* uint_rand */



int uint8MostSignificantBit (uint8type number)

  {
    int result;

  /* uint8MostSignificantBit */
    result = most_significant[number];
    return result;
  } /* uint8MostSignificantBit */



int uint16MostSignificantBit (uint16type number)

  {
    int result;

  /* uint16MostSignificantBit */
    if (number & 0xff00) {
      result = 8 + most_significant[number >> 8];
    } else {
      result = most_significant[number];
    } /* if */
    return result;
  } /* uint16MostSignificantBit */



int uint32MostSignificantBit (uint32type number)

  {
    int result;

  /* uint32MostSignificantBit */
    result = 0;
    if (number & 0xffff0000) {
      number >>= 16;
      result += 16;
    } /* if */
    if (number & 0xff00) {
      number >>= 8;
      result += 8;
    } /* if */
    result += most_significant[number];
    return result;
  } /* uint32MostSignificantBit */



#ifdef INT64TYPE
int uint64MostSignificantBit (uint64type number)

  {
    int result;

  /* uint64MostSignificantBit */
    result = 0;
#ifdef INT64TYPE_SUFFIX_LL
    if (number & 0xffffffff00000000LL) {
#else
    if (number & 0xffffffff00000000) {
#endif
      number >>= 32;
      result = 32;
    } /* if */
    if (number & 0xffff0000) {
      number >>= 16;
      result += 16;
    } /* if */
    if (number & 0xff00) {
      number >>= 8;
      result += 8;
    } /* if */
    result += most_significant[number];
    return result;
  } /* uint64MostSignificantBit */
#endif



int uint8LeastSignificantBit (uint8type number)

  {
    int result;

  /* uint8LeastSignificantBit */
    result = least_significant[number & 0xff];
    return result;
  } /* uint8LeastSignificantBit */



int uint16LeastSignificantBit (uint16type number)

  {
    int result;

  /* uint16LeastSignificantBit */
    if ((number & 0xff) == 0) {
      result = 8 + least_significant[(number >> 8) & 0xff];
    } else {
      result = least_significant[number & 0xff];
    } /* if */
    return result;
  } /* uint16LeastSignificantBit */



int uint32LeastSignificantBit (uint32type number)

  {
    int result;

  /* uint32LeastSignificantBit */
    result = 0;
    if ((number & 0xffff) == 0) {
      number >>= 16;
      result += 16;
    } /* if */
    if ((number & 0xff) == 0) {
      number >>= 8;
      result += 8;
    } /* if */
    result += least_significant[number & 0xff];
    return result;
  } /* uint32LeastSignificantBit */



#ifdef INT64TYPE
int uint64LeastSignificantBit (uint64type number)

  {
    int result;

  /* uint64LeastSignificantBit */
    result = 0;
    if ((number & 0xffffffff) == 0) {
      number >>= 32;
      result = 32;
    } /* if */
    if ((number & 0xffff) == 0) {
      number >>= 16;
      result += 16;
    } /* if */
    if ((number & 0xff) == 0) {
      number >>= 8;
      result += 8;
    } /* if */
    result += least_significant[number & 0xff];
    return result;
  } /* uint64LeastSignificantBit */
#endif



inttype uintCmpGeneric (const rtlGenerictype value1, const rtlGenerictype value2)

  { /* uintCmpGeneric */
    if (value1 < value2) {
      return -1;
    } else if (value1 > value2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* uintCmpGeneric */



/**
 *  Binomial coefficient
 *  @return n over k
 */
inttype intBinom (inttype n_number, inttype k_number)

  {
    inttype number;
    uinttype unsigned_result;
    inttype result;

  /* intBinom */
    /* printf("(%ld ! %ld) ", k_number, n_number); */
    if (n_number > 0 && 2 * k_number > n_number) {
      k_number = n_number - k_number;
    } /* if */
    if (k_number < 0) {
      result = 0;
    } else if (k_number == 0) {
      result = 1;
    } else /* if (n_number <= 30 || k_number <= 7)  */{
      if (n_number < 0) {
        result = n_number;
        for (number = 2; number <= k_number; number++) {
          result *= (n_number - number + 1);
          result /= number;
        } /* for */
      } else {
        unsigned_result = (uinttype) n_number;
        for (number = 2; number <= k_number; number++) {
          unsigned_result *= (uinttype) (n_number - number + 1);
          unsigned_result /= (uinttype) number;
        } /* for */
        result = (inttype) unsigned_result;
      } /* if */
/*
    } else {
      result = binom(n_number - 1, k_number - 1) +
          binom(n_number - 1, k_number);
*/
    } /* if */
    /* printf("--> %ld\n", result); */
    return result;
  } /* intBinom */



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 */
inttype intBitLength (inttype number)

  {
    inttype result;

  /* intBitLength */
    if (number < 0) {
      number = ~number;
    } /* if */
    result = uintMostSignificantBit((uinttype) number) + 1;
    return result;
  } /* intBitLength */



/**
 *  Compare two integer numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
inttype intCmp (inttype number1, inttype number2)

  { /* intCmp */
    if (number1 < number2) {
      return -1;
    } else if (number1 > number2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* intCmp */



/**
 *  Reinterpret the generic parameters as inttype and call intCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(rtlGenerictype) != sizeof(inttype).
 */
inttype intCmpGeneric (const rtlGenerictype value1, const rtlGenerictype value2)

  { /* intCmpGeneric */
    return intCmp((inttype) value1, (inttype) value2);
  } /* intCmpGeneric */



void intCpy (inttype *dest, inttype source)

  { /* intCpy */
    *dest = source;
  } /* intCpy */



/**
 *  Generic Create function to be used via function pointers.
 *  This functions is used to create new values for all types
 *  where a binary copy without special funtionality can be used:
 *  E.g.: inttype, floattype, chartype, booltype.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. The function intCreateGeneric is
 *  used from hashtables where the keys and the data is stored in
 *  rtlGenerictype data elements. This assures correct behaviour
 *  even when sizeof(rtlGenerictype) != sizeof(inttype). Even for
 *  sizeof(rtlGenerictype) == sizeof(inttype) some things must
 *  be considered: On some architectures (linux with gcc)
 *  functions with float results seem to be returned in a
 *  different way (may be another register). Therefore
 *  intCreateGeneric (with rtlGenerictype) must be used even
 *  when sizeof(rtlGenerictype) == sizeof(floattype).
 */
rtlGenerictype intCreateGeneric (rtlGenerictype source)

  { /* intCreateGeneric */
    return source;
  } /* intCreateGeneric */



/**
 *  Generic Destr function to be used via function pointers.
 *  This functions is used to destroy old values for all types
 *  where a noop without special funtionality can be used:
 *  E.g.: inttype, floattype, chartype, booltype.
 *  In contrast to prcNoop the number of parameters (1) of
 *  intDestrGeneric is correct. This is important since some
 *  C compilers generate wrong code when prcNoop is used.
 */
void intDestrGeneric (rtlGenerictype old_value)

  { /* intDestrGeneric */
  } /* intDestrGeneric */



/**
 *  Compute the truncated base 2 logarithm of an integer number.
 *  The definition of intLog2 is extended by defining intLog2(0) = -1.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
inttype intLog2 (inttype number)

  {
    int result;

  /* intLog2 */
    if (unlikely(number < 0)) {
      raise_error(NUMERIC_ERROR);
      result = 0;
    } else {
      result = uintMostSignificantBit((uinttype) number);
    } /* if */
    return result;
  } /* intLog2 */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
inttype intLowestSetBit (inttype number)

  {
    inttype result;

  /* intLowestSetBit */
    result = 0;
    if ((number & 0xffff) == 0) {
      number >>= 16;
      result += 16;
    } /* if */
    if ((number & 0xff) == 0) {
      number >>= 8;
      result += 8;
    } /* if */
    result += least_significant[number & 0xff];
    return (inttype) result;
  } /* intLowestSetBit */



stritype intLpad0 (inttype number, const inttype pad_size)

  {
    uinttype unsigned_number;
    booltype negative;
    strelemtype *buffer;
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* intLpad0 */
    negative = (number < 0);
    if (negative) {
      unsigned_number = (uinttype) -number;
    } else {
      unsigned_number = (uinttype) number;
    } /* if */
    length = DECIMAL_DIGITS(unsigned_number);
    if (negative) {
      length++;
    } /* if */
    if (pad_size > (inttype) length) {
      if (unlikely((uinttype) pad_size > MAX_STRI_LEN)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result_size = (memsizetype) pad_size;
      } /* if */
    } else {
      result_size = length;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      buffer = &result->mem[result_size];
      do {
        *(--buffer) = (strelemtype) (unsigned_number % 10 + '0');
      } while ((unsigned_number /= 10) != 0);
      if (buffer != result->mem) {
        while (buffer != &result->mem[1]) {
          *(--buffer) = (strelemtype) '0';
        } /* while */
        if (negative) {
          result->mem[0] = (strelemtype) '-';
        } else {
          result->mem[0] = (strelemtype) '0';
        } /* if */
      } /* if */
      return result;
    } /* if */
  } /* intLpad0 */



/**
 *  Convert a string to an integer number.
 *  The string must contain an integer literal consisting of an
 *  optional + or - sign, followed by a sequence of digits. Other
 *  characters as well as leading or trailing whitespace characters
 *  are not allowed. The sequence of digits is taken to be decimal.
 *  @return the integer result of the conversion.
 *  @exception RANGE_ERROR When the string is empty or it does not contain
 *             an integer literal or when the integer literal is too big
 *             or too small to be represented as integer value.
 */
inttype intParse (const const_stritype stri)

  {
    booltype okay;
    booltype negative;
    memsizetype position;
    inttype digitval;
    inttype integer_value;

  /* intParse */
    okay = TRUE;
    position = 0;
    integer_value = 0;
    if (stri->size != 0) {
      if (stri->mem[0] == ((strelemtype) '-')) {
        negative = TRUE;
        position++;
      } else {
        if (stri->mem[0] == ((strelemtype) '+')) {
          position++;
        } /* if */
        negative = FALSE;
      } /* if */
      while (position < stri->size &&
          stri->mem[position] >= ((strelemtype) '0') &&
          stri->mem[position] <= ((strelemtype) '9')) {
        digitval = ((inttype) stri->mem[position]) - ((inttype) '0');
        if (integer_value < MAX_DIV_10 ||
            (integer_value == MAX_DIV_10 &&
            digitval <= MAX_REM_10)) {
          integer_value = ((inttype) 10) * integer_value + digitval;
        } else {
          okay = FALSE;
        } /* if */
        position++;
      } /* while */
    } /* if */
    if (position == 0 || position < stri->size) {
      okay = FALSE;
    } /* if */
    if (likely(okay)) {
      if (negative) {
        return -integer_value;
      } else {
        return integer_value;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
      return 0;
    } /* if */
  } /* intParse */



/**
 *  Compute the exponentiation of a integer base with an integer exponent.
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
inttype intPow (inttype base, inttype exponent)

  {
    inttype result;

  /* intPow */
    if (unlikely(exponent < 0)) {
      raise_error(NUMERIC_ERROR);
      result = 0;
    } else {
      if (exponent & 1) {
        result = base;
      } else {
        result = 1;
      } /* if */
      exponent >>= 1;
      while (exponent != 0) {
        base *= base;
        if (exponent & 1) {
          result *= base;
        } /* if */
        exponent >>= 1;
      } /* while */
    } /* if */
    return result;
  } /* intPow */



/**
 *  Convert an integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  The parameter upperCase decides about the letter case.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
stritype intRadix (inttype number, inttype base, booltype upperCase)

  {
    uinttype unsigned_number;
    booltype negative;
    const_cstritype digits;
    strelemtype buffer_1[75];
    strelemtype *buffer;
    memsizetype length;
    stritype result;

  /* intRadix */
    if (unlikely(base < 2 || base > 36)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      negative = (number < 0);
      if (negative) {
        /* The unsigned value is negated to avoid a signed integer */
        /* overflow when the smallest signed integer is negated.   */
        unsigned_number = -(uinttype) number;
      } else {
        unsigned_number = (uinttype) number;
      } /* if */
      digits = digitTable[upperCase];
      buffer = &buffer_1[75];
      do {
        *(--buffer) = (strelemtype) (digits[unsigned_number % (uinttype) base]);
      } while ((unsigned_number /= (uinttype) base) != 0);
      if (negative) {
        *(--buffer) = (strelemtype) '-';
      } /* if */
      length = (memsizetype) (&buffer_1[75] - buffer);
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = length;
        memcpy(result->mem, buffer, (size_t) (length * sizeof(strelemtype)));
      } /* if */
    } /* if */
    return result;
  } /* intRadix */



/**
 *  Convert an integer number to a string using a radix.
 *  The conversion uses the numeral system with the specified base.
 *  The base is a power of two and it is specified indirectly with
 *  shift and mask. Digit values from 10 upward are encoded with
 *  letters. The parameter upperCase decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
stritype intRadixPow2 (inttype number, int shift, int mask, booltype upperCase)

  {
    uinttype unsigned_number;
    booltype negative;
    const_cstritype digits;
    strelemtype buffer_1[50];
    strelemtype *buffer;
    memsizetype length;
    stritype result;

  /* intRadixPow2 */
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uinttype) number;
    } else {
      unsigned_number = (uinttype) number;
    } /* if */
      digits = digitTable[upperCase];
    buffer = &buffer_1[50];
    do {
      *(--buffer) = (strelemtype) (digits[unsigned_number & (uinttype) mask]);
    } while ((unsigned_number >>= shift) != 0);
    if (negative) {
      *(--buffer) = (strelemtype) '-';
    } /* if */
    length = (memsizetype) (&buffer_1[50] - buffer);
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = length;
      memcpy(result->mem, buffer, (size_t) (length * sizeof(strelemtype)));
      return result;
    } /* if */
  } /* intRadixPow2 */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
inttype intRand (inttype low, inttype high)

  {
    uinttype scale_limit;
    uinttype rand_max;
    uinttype rand_val;
    inttype result;

  /* intRand */
    if (unlikely(low >= high)) {
      if (low == high) {
        result = low;
      } else {
        raise_error(RANGE_ERROR);
        result = 0;
      } /* if */
    } else {
      scale_limit = (uinttype) high - (uinttype) low;
      if (scale_limit == UINTTYPE_MAX) {
        rand_val = uint_rand();
      } else {
        scale_limit++;
        rand_max = UINTTYPE_MAX - (UINTTYPE_MAX % scale_limit);
        do {
          rand_val = uint_rand();
        } while (rand_val > rand_max);
      } /* if */
      result = (inttype) ((uinttype) low + rand_val % scale_limit);
    } /* if */
    return result;
  } /* intRand */



/**
 *  Compute the integer square root of an integer number.
 *  @return the integer square root.
 *  @exception NUMERIC_ERROR When number is negative.
 */
inttype intSqrt (inttype number)

  {
    register uinttype result;
    register uinttype res2;

  /* intSqrt */
    if (unlikely(number < 0)) {
      raise_error(NUMERIC_ERROR);
      return 0;
    } else if (number == 0) {
      return (inttype) 0;
    } else {
      res2 = (uinttype) number;
      do {
        result = res2;
        res2 = (result + (uinttype) number / result) >> 1;
      } while (result > res2);
      return (inttype) result;
    } /* if */
  } /* intSqrt */



/**
 *  Convert an integer number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
stritype intStr (inttype number)

  {
    register uinttype unsigned_number;
    booltype negative;
    strelemtype *buffer;
    memsizetype length;
    stritype result;

  /* intStr */
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uinttype) number;
    } else {
      unsigned_number = (uinttype) number;
    } /* if */
    length = DECIMAL_DIGITS(unsigned_number);
    if (negative) {
      length++;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = length;
      buffer = &result->mem[length];
      do {
        *(--buffer) = (strelemtype) (unsigned_number % 10 + '0');
      } while ((unsigned_number /= 10) != 0);
      if (negative) {
        result->mem[0] = (strelemtype) '-';
      } /* if */
    } /* if */
    return result;
  } /* intStr */



#ifdef ALLOW_STRITYPE_SLICES
stritype intStrToBuffer (inttype number, stritype buffer)

  {
    register uinttype unsigned_number;
    booltype negative;
    strelemtype *bufferPtr;

  /* intStrToBuffer */
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uinttype) number;
    } else {
      unsigned_number = (uinttype) number;
    } /* if */
    bufferPtr = &buffer->mem1[INTTYPE_DECIMAL_DIGITS + 1];
    do {
      *(--bufferPtr) = (strelemtype) (unsigned_number % 10 + '0');
    } while ((unsigned_number /= 10) != 0);
    if (negative) {
      *(--bufferPtr) = (strelemtype) '-';
    } /* if */
    buffer->mem = bufferPtr;
    buffer->size = (memsizetype) (&buffer->mem1[INTTYPE_DECIMAL_DIGITS + 1] - bufferPtr);
    return buffer;
  } /* intStrToBuffer */
#endif



bstritype intToBStriBe (inttype number)

  {
    uchartype buffer[8];
    int pos = 7;
    bstritype result;

  /* intToBStriBe */
    /* printf("intToBStriBe(%016lx)\n", number); */
    if (number > 0) {
      do {
        buffer[pos] = (uchartype) (number & 0xff);
        number >>= 8;
        pos--;
      } while (number != 0);
      if (buffer[pos + 1] <= 127) {
        pos++;
      } else {
        buffer[pos] = 0;
      } /* if */
    } else if (number < 0) {
      do {
        buffer[pos] = (uchartype) (number & 0xff);
#ifdef RSHIFT_DOES_SIGN_EXTEND
        number >>= 8;
#else
        number = ~(~number >> 8);
#endif
        pos--;
      } while (number != -1);
      if (buffer[pos + 1] >= 128) {
        pos++;
      } else {
        buffer[pos] = 255;
      } /* if */
    } else {
      buffer[7] = '\0';
    } /* if */
    if (!ALLOC_BSTRI_SIZE_OK(result, (memsizetype) (8 - pos))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = (memsizetype) (8 - pos);
      memcpy(result->mem, &buffer[pos],
             (memsizetype) (8 - pos) * sizeof(uchartype));
    } /* if */
    return result;
  } /* intToBStriBe */



bstritype intToBStriLe (inttype number)

  {
    uchartype buffer[8];
    int pos = 0;
    bstritype result;

  /* intToBStriLe */
    /* printf("intToBStriLe(%016lx)\n", number); */
    if (number > 0) {
      do {
        buffer[pos] = (uchartype) (number & 0xff);
        number >>= 8;
        pos++;
      } while (number != 0);
      if (buffer[pos - 1] <= 127) {
        pos--;
      } else {
        buffer[pos] = 0;
      } /* if */
    } else if (number < 0) {
      do {
        buffer[pos] = (uchartype) (number & 0xff);
#ifdef RSHIFT_DOES_SIGN_EXTEND
        number >>= 8;
#else
        number = ~(~number >> 8);
#endif
        pos++;
      } while (number != -1);
      if (buffer[pos - 1] >= 128) {
        pos--;
      } else {
        buffer[pos] = 255;
      } /* if */
    } else {
      buffer[0] = '\0';
    } /* if */
    if (!ALLOC_BSTRI_SIZE_OK(result, (memsizetype) (pos + 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = (memsizetype) (pos + 1);
      memcpy(result->mem, &buffer[0],
             (memsizetype) (pos + 1) * sizeof(uchartype));
    } /* if */
    return result;
  } /* intToBStriLe */
