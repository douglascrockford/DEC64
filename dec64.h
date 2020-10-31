/* dec64.h

The DEC64 header file. This is the companion to dec64.asm.

dec64.com
2019-09-03
Public Domain

No warranty.
*/
#ifndef __DEC64_H__
#define __DEC64_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DEC64_ZERO          (0x000LL)
#define DEC64_ONE           (0x100LL)
#define DEC64_TWO           (0x200LL)
#define DEC64_NEGATIVE_ONE  (0xFFFFFFFFFFFFFF00LL)

#define DEC64_NULL          (0x8000000000000080LL)
#define DEC64_TRUE          (0x8000000000000380LL)
#define DEC64_FALSE         (0x8000000000000280LL)
#define DEC64_NAN           DEC64_NULL 

typedef long long int64;
typedef unsigned long long uint64;
typedef int64 dec64;

extern int64 dec64_coefficient(dec64 number)                /*   coefficient */;
extern int64 dec64_exponent(dec64 number)                   /*      exponent */;

extern dec64 dec64_is_equal(dec64 comparahend, dec64 comparator)/*comparison */;
extern dec64 dec64_is_false(dec64 boolean)                  /*    comparison */;
extern dec64 dec64_is_integer(dec64 number)                 /*    comparison */;
extern dec64 dec64_eps_equal(dec64 comparahend, dec64 comparator, dec64 eps)/*comparison */;
extern dec64 dec64_is_less(dec64 comparahend, dec64 comparator)/* comparison */;
extern dec64 dec64_is_nan(dec64 number)                     /*    comparison */;
extern dec64 dec64_is_zero(dec64 number)                    /*    comparison */;

extern dec64 dec64_abs(dec64 number)                        /*    absolution */;
extern dec64 dec64_add(dec64 augend, dec64 addend)          /*           sum */;
extern dec64 dec64_ceiling(dec64 number)                    /*       integer */;
extern dec64 dec64_divide(dec64 dividend, dec64 divisor)    /*      quotient */;
extern dec64 dec64_floor(dec64 dividend)                    /*       integer */;
extern dec64 dec64_integer_divide(dec64 dividend, dec64 divisor)/*  quotient */;
extern dec64 dec64_modulo(dec64 dividend, dec64 divisor)    /*    modulation */;
extern dec64 dec64_multiply(dec64 multiplicand, dec64 multiplier)/*  product */;
extern dec64 dec64_neg(dec64 number)                        /*      negation */;
extern dec64 dec64_new(int64 coefficient, int exponent)     /*        number */;
extern dec64 dec64_normal(dec64 number)                     /* normalization */;
extern dec64 dec64_round(dec64 number, dec64 place)         /*  quantization */;
extern dec64 dec64_signum(dec64 number)                     /*     signature */;
extern dec64 dec64_subtract(dec64 minuend, dec64 subtrahend)/*    difference */;
extern dec64 dec64_from_double(double d);

#ifdef __cplusplus
}
#endif

#include "dec64_string.h"
#include "dec64_math.h"

#endif
