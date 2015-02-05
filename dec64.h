/* dec64.h

The DEC64 header file. This is the companion to dec64.asm.

dec64.com
2015-01-23
Public Domain

No warranty.
*/

typedef long long int64;
typedef int64 bool64;
typedef int64 dec64;

extern int64 dec64_coefficient(dec64 number)                  /* coefficient */;
extern int64 dec64_exponent(dec64 number)                        /* exponent */;

extern bool64 dec64_equal(dec64 comparahend, dec64 comparator) /* comparison */;
extern bool64 dec64_is_integer(dec64 number)                   /* comparison */;
extern bool64 dec64_is_nan(dec64 number)                       /* comparison */;
extern bool64 dec64_is_zero(dec64 number)                      /* comparison */;
extern bool64 dec64_less(dec64 comparahend, dec64 comparator)  /* comparison */;

extern dec64 dec64_abs(dec64 number)                           /* absolution */;
extern dec64 dec64_add(dec64 augend, dec64 addend)                    /* sum */;
extern dec64 dec64_ceiling(dec64 number)                          /* integer */;
extern dec64 dec64_divide(dec64 dividend, dec64 divisor)         /* quotient */;
extern dec64 dec64_floor(dec64 number)                            /* integer */;
extern dec64 dec64_int(dec64 number)                              /* integer */;
extern dec64 dec64_integer_divide(dec64 dividend, dec64 divisor) /* quotient */;
extern dec64 dec64_modulo(dec64 dividend, dec64 divisor)       /* modulation */;
extern dec64 dec64_multiply(dec64 multiplicand, dec64 multiplier) /* product */;
extern dec64 dec64_nan()                                              /* NaN */;
extern dec64 dec64_neg(dec64 number)                             /* negation */;
extern dec64 dec64_new(int64 coefficient, int64 exponent)          /* number */;
extern dec64 dec64_not(dec64 number)                             /* notation */;
extern dec64 dec64_one()                                              /* one */;
extern dec64 dec64_round(dec64 number, dec64 place)            /* roundation */;
extern dec64 dec64_signum(dec64 number)                         /* signature */;
extern dec64 dec64_subtract(dec64 minuend, dec64 subtrahend)   /* difference */;
extern dec64 dec64_zero()                                            /* zero */;
