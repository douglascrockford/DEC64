/*
dec64_math.c
Elementary functions for DEC64.

dec64.com
2016-01-13
Public Domain

No warranty.

This file is a placeholder. It should be replaced with functions that are
faster and more accurate.
*/

#include <stdlib.h>
#include "dec64.h"
#include "dec64_math.h"

#define D_2                           0x200LL
#define D_E              0x6092A113D8D574F0LL
#define D_HALF                        0x5FFLL
#define D_HALF_PI        0x37CE4F32BB21A6F0LL
#define D_NPI            0x9063619A89BCB4F0LL
#define D_PI             0x6F9C9E6576434CF0LL
#define D_2PI            0x165286144ADA42F1LL

dec64 dec64_acos(dec64 x) {
    dec64 result = dec64_subtract(
        D_HALF_PI,
        dec64_asin(x)
    );
    return result;
}

dec64 dec64_asin(dec64 x) {
    if (dec64_equal(x, DEC64_ONE) == DEC64_TRUE) {
        return D_HALF_PI;
    }
    if (dec64_equal(x, DEC64_NEGATIVE_ONE) == DEC64_TRUE) {
        return dec64_neg(D_HALF_PI);
    }
    if (
        dec64_is_any_nan(x) == DEC64_TRUE ||
        dec64_less(DEC64_ONE, dec64_abs(x)) == DEC64_TRUE
    ) {
        return DEC64_NAN;
    }
    dec64 bottom = D_2;
    dec64 factor = x;
    dec64 x2 = dec64_multiply(x, x);
    dec64 result = factor;
    while (1) {
        factor = dec64_divide(
            dec64_multiply(
                dec64_multiply(dec64_dec(bottom), x2), 
                factor
            ), 
            bottom
        );
        dec64 progress = dec64_add(
            result, 
            dec64_divide(factor, dec64_inc(bottom))
        );
        if (result == progress) {
            break;
        }
        result = progress;
        bottom = dec64_add(bottom, D_2);
    }
    return result;
}

dec64 dec64_atan(dec64 x) {
    return dec64_asin(
        dec64_divide(
            x, 
            dec64_sqrt(dec64_inc(dec64_multiply(x, x)))
        )
    );
}

dec64 dec64_atan2(dec64 y, dec64 x) {
    if (dec64_is_zero(x) == DEC64_TRUE) {
        if (dec64_is_zero(y) == DEC64_TRUE) {
            return DEC64_NAN;
        } else if (y < 0) {
            return dec64_neg(D_HALF_PI);
        } else {
            return D_HALF_PI;
        }
    } else {
        dec64 atan = dec64_atan(dec64_divide(y, x));
        if (x < 0) {
            if (y < 0) {
                return dec64_subtract(atan, D_HALF_PI);
            } else {
                return dec64_add(atan, D_HALF_PI);
            }
        } else {
            return atan;
        }
    }
}

dec64 dec64_cos(dec64 x) {
    return dec64_sin(dec64_add(x, D_HALF_PI));
}

dec64 dec64_exp(dec64 x) {
        dec64 result = dec64_inc(x);
        dec64 divisor = D_2;
        dec64 term = x;
        while (1) {
            term = dec64_divide(
                dec64_multiply(term, x), 
                divisor
            );
            dec64 progress = dec64_add(result, term);
            if (result == progress) {
                break;
            }
            result = progress;
            divisor = dec64_inc(divisor);
        }
        return result;
}

dec64 dec64_exponentiate(dec64 coefficient, dec64 exponent) {
    if (dec64_is_zero(exponent) == DEC64_TRUE) {
        return DEC64_ONE;
    }

// Adjust for a negative exponent.

    if (exponent < 0) {
        coefficient = dec64_divide(DEC64_ONE, coefficient);
        exponent = dec64_neg(exponent);
    }
    if (dec64_is_any_nan(coefficient) == DEC64_TRUE) {
        return DEC64_NAN;
    }
    if (dec64_is_zero(coefficient) == DEC64_TRUE) {
        return 0;
    }

// If the exponent is an integer, then use the squaring method.

    if (exponent > 0 && dec64_exponent(exponent) == 0) {
        dec64 aux = DEC64_ONE;
        int64 n = dec64_coefficient(exponent);
        if (n <= 1) {
            return coefficient;
        }
        while (n > 1) {
            if ((n & 1) != 0) {
                aux = dec64_multiply(aux, coefficient);
            }
            coefficient = dec64_multiply(coefficient, coefficient);
            n /= 2;
        }
        return (n == 1)
            ? dec64_multiply(aux, coefficient)
            : aux;
    }

// Otherwise do it the hard way.

    return dec64_exp(dec64_multiply(
        dec64_log(coefficient), 
        exponent
    ));
}

dec64 dec64_log(dec64 x)
{
    if (x <= 0 || dec64_is_any_nan(x) == DEC64_TRUE) {
        return DEC64_NAN;
    }
    if (dec64_equal(x, DEC64_ONE) == DEC64_TRUE) {
        return DEC64_ZERO;
    }
    if (dec64_equal(x, D_HALF) == DEC64_TRUE) {
        return dec64_new(-6931471805599453, -16);
    }
    if (x == D_E) {
        return DEC64_ONE;
    }
    dec64 y = dec64_divide(dec64_dec(x), x);
    dec64 factor = y;
    dec64 result = factor;
    dec64 divisor = D_2;
    while (1) {
        factor = dec64_multiply(factor, y);
        dec64 progress = dec64_add(
            result, 
            dec64_divide(factor, divisor)
        );
        if (result == progress) {
            break;
        }
        result = progress;
        divisor = dec64_inc(divisor);
    }
    return result;
}

static int64 slush = 0;

dec64 dec64_random() {

/* Return a randomish number between 0 and 1 containing 16 randomy digits. */

    dec64 mantissa = 0;
/* 
    rand() produces low quality 15 bit numbers. We'll use 7 of them to make up
    ours.
*/

    do {
        slush = (slush << 8) ^ (slush >> 55) ^ rand();
        slush = (slush << 8) ^ (slush >> 55) ^ rand();
        slush = (slush << 8) ^ (slush >> 55) ^ rand();
        slush = (slush << 8) ^ (slush >> 55) ^ rand();
        slush = (slush << 8) ^ (slush >> 55) ^ rand();
        slush = (slush << 8) ^ (slush >> 55) ^ rand();
        slush = (slush << 8) ^ (slush >> 55) ^ rand();
        mantissa = slush & 0x3FFFFFFFFFFFFFLL;
/*
    mantissa contains an integer between 0 and 18014398509481983.
    If it is less than or equal to 9999999999999999 then we are done.
*/
    } while (mantissa > 9999999999999999LL);
    return dec64_new(mantissa, -16);
}

dec64 dec64_root(dec64 degree, dec64 radicand) {
    int repeat;
    dec64 result;
    degree = dec64_normal(degree);
    if (
        dec64_is_any_nan(radicand) == DEC64_TRUE ||
        dec64_is_zero(degree) == DEC64_TRUE ||
        degree < 0 ||
        dec64_exponent(degree) != 0 ||
        (
            radicand < 0 &&
            (dec64_coefficient(degree) & 1) == 0
        )
    ) {
        return DEC64_NAN;
    }
    if (dec64_is_zero(radicand) == DEC64_TRUE) {
        return DEC64_ZERO;
    }
    if (degree == DEC64_ONE) {
        return radicand;
    }
    if (degree == D_2) {
        return dec64_sqrt(radicand);
    }
    dec64 degree_minus_one = dec64_dec(degree);
    repeat = 64;
    result = DEC64_ONE;
    while (repeat > 0) {
        dec64 progress = dec64_divide(
            dec64_add(
                dec64_multiply(result, degree_minus_one),
                dec64_divide(
                    radicand,
                    dec64_exponentiate(result, degree_minus_one)
                )
            ),
            degree
        );
        if (progress == result) {
            break;
        }
        result = progress;
        repeat -= 1;
    }
    return result;
}

dec64 dec64_sin(dec64 x) {
    while (dec64_less(D_PI, x) == DEC64_TRUE) {
        x = dec64_subtract(x, D_PI);
        x = dec64_subtract(x, D_PI);
    }
    while (dec64_less(x, D_NPI) == DEC64_TRUE) {
        x = dec64_add(x, D_PI);
        x = dec64_add(x, D_PI);
    }
    int neg = 0;
    if (x < 0) {
        x = dec64_neg(x);
        neg = -1;
    }
    if (dec64_less(D_HALF_PI, x) == DEC64_TRUE) {
        x = dec64_subtract(D_PI, x);
    }
    dec64 result;
    if (x == D_HALF_PI) {
        result = DEC64_ONE;
    } else {
        dec64 x2 = dec64_multiply(x, x);
        dec64 order = DEC64_ONE;
        dec64 term = x;
        result = term;
        while (1) {
            term = dec64_multiply(term, x2);
            order = dec64_inc(order);
            term = dec64_divide(term, order);
            order = dec64_inc(order);
            term = dec64_divide(term, order);
            dec64 progress = dec64_subtract(result, term);

            term = dec64_multiply(term, x2);
            order = dec64_inc(order);
            term = dec64_divide(term, order);
            order = dec64_inc(order);
            term = dec64_divide(term, order);
            progress = dec64_add(progress, term);

            if (progress == result) {
                break;
            }
            result = progress;
        }
    }
    if (neg) {
        result = dec64_neg(result);
    }
    return result;
}

dec64 dec64_sqrt(dec64 radicand) {
    if (dec64_is_any_nan(radicand) != DEC64_TRUE && radicand >= 0) {
        if (dec64_coefficient(radicand) == 0) {
            return DEC64_ZERO;
        }
        int repeat = 8;
        dec64 result = radicand;
        do {
            result = dec64_half(dec64_add(
                result,
                dec64_divide(radicand, result)
            ));
            repeat -= 1;
        } while (repeat > 0);
        return result;
    } else {
        return DEC64_NAN;
    }
}

dec64 dec64_tan(dec64 x) {
    return dec64_divide(
        dec64_sin(x), 
        dec64_cos(x)
    );
}
