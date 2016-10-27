/* dec64.h

The DEC64 header file. This is the companion to dec64.asm.

dec64.com
2016-10-27
Public Domain

No warranty.
*/

#define DEC64_NAN           (0x80LL)
#define DEC64_ZERO          (0x00LL)
#define DEC64_ONE           (0x100LL)
#define DEC64_TRUE          (0x380LL)
#define DEC64_FALSE         (0x280LL)
#define DEC64_NEGATIVE_ONE  (0xFFFFFFFFFFFFFF00LL)

typedef long long int64;
typedef unsigned long long uint64;
typedef int64 dec64;

extern int64 dec64_coefficient(dec64 number)                  /* coefficient */;
extern int64 dec64_exponent(dec64 number)                        /* exponent */;

extern dec64 dec64_equal(dec64 comparahend, dec64 comparator)  /* comparison */;
extern dec64 dec64_is_any_nan(dec64 number)                    /* comparison */;
extern dec64 dec64_is_integer(dec64 number)                    /* comparison */;
extern dec64 dec64_is_zero(dec64 number)                       /* comparison */;
extern dec64 dec64_less(dec64 comparahend, dec64 comparator)   /* comparison */;

extern dec64 dec64_abs(dec64 number)                           /* absolution */;
extern dec64 dec64_add(dec64 augend, dec64 addend)                    /* sum */;
extern dec64 dec64_ceiling(dec64 number)                          /* integer */;
extern dec64 dec64_dec(dec64 minuend)                      /* decrementation */;
extern dec64 dec64_divide(dec64 dividend, dec64 divisor)         /* quotient */;
extern dec64 dec64_floor(dec64 dividend)                          /* integer */;
extern dec64 dec64_half(dec64 dividend)                          /* quotient */;
extern dec64 dec64_inc(dec64 augend)                       /* incrementation */;
extern dec64 dec64_int(dec64 number)                              /* integer */;
extern dec64 dec64_integer_divide(dec64 dividend, dec64 divisor) /* quotient */;
extern dec64 dec64_modulo(dec64 dividend, dec64 divisor)       /* modulation */;
extern dec64 dec64_multiply(dec64 multiplicand, dec64 multiplier) /* product */;
extern dec64 dec64_neg(dec64 number)                             /* negation */;
extern dec64 dec64_new(int64 coefficient, int64 exponent)          /* number */;
extern dec64 dec64_normal(dec64 number)                     /* normalization */;
extern dec64 dec64_not(dec64 boolean)                            /* notation */;
extern dec64 dec64_round(dec64 number, dec64 place)          /* quantization */;
extern dec64 dec64_signum(dec64 number)                         /* signature */;
extern dec64 dec64_subtract(dec64 minuend, dec64 subtrahend)   /* difference */;
