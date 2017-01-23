/*
dec64_math.c
Elementary functions for DEC64.

dec64.com
2017-01-22
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
#define D_NHALF_PI       0xC831B0CD44DE59F0LL
#define D_NPI            0x9063619A89BCB4F0LL
#define D_PI             0x6F9C9E6576434CF0LL
#define D_2PI            0x165286144ADA42F1LL

#define FAC              93

dec64 factorials[FAC] = {
    (1LL << 8) + 0,
    (1LL << 8) + 0,
    (2LL << 8) + 0,
    (6LL << 8) + 0,
    (24LL << 8) + 0,
    (120LL << 8) + 0,
    (720LL << 8) + 0,
    (5040LL << 8) + 0,
    (40320LL << 8) + 0,
    (362880LL << 8) + 0,
    (3628800LL << 8) + 0,
    (39916800LL << 8) + 0,
    (479001600LL << 8) + 0,
    (6227020800LL << 8) + 0,
    (87178291200LL << 8) + 0,
    (1307674368000LL << 8) + 0,
    (20922789888000LL << 8) + 0,
    (355687428096000LL << 8) + 0,
    (6402373705728000LL << 8) + 0,
    (12164510040883200LL << 8) + 1,
    (24329020081766400LL << 8) + 2,
    (5109094217170944LL << 8) + 4,
    (11240007277776077LL << 8) + 5,
    (25852016738884977LL << 8) + 6,
    (6204484017332394LL << 8) + 8,
    (15511210043330986LL << 8) + 9,
    (4032914611266056LL << 8) + 11,
    (10888869450418352LL << 8) + 12,
    (30488834461171386LL << 8) + 13,
    (8841761993739702LL << 8) + 15,
    (26525285981219106LL << 8) + 16,
    (8222838654177923LL << 8) + 18,
    (26313083693369353LL << 8) + 19,
    (8683317618811886LL << 8) + 21,
    (29523279903960414LL << 8) + 22,
    (10333147966386145LL << 8) + 24,
    (3719933267899012LL << 8) + 26,
    (13763753091226345LL << 8) + 27,
    (5230226174666011LL << 8) + 29,
    (20397882081197443LL << 8) + 30,
    (8159152832478977LL << 8) + 32,
    (33452526613163807LL << 8) + 33,
    (14050061177528799LL << 8) + 35,
    (6041526306337384LL << 8) + 37,
    (26582715747884488LL << 8) + 38,
    (11962222086548019LL << 8) + 40,
    (5502622159812089LL << 8) + 42,
    (25862324151116818LL << 8) + 43,
    (12413915592536073LL << 8) + 45,
    (6082818640342676LL << 8) + 47,
    (30414093201713378LL << 8) + 48,
    (15511187532873823LL << 8) + 50,
    (8065817517094388LL << 8) + 52,
    (4274883284060026LL << 8) + 54,
    (23084369733924138LL << 8) + 55,
    (12696403353658276LL << 8) + 57,
    (7109985878048635LL << 8) + 59,
    (4052691950487722LL << 8) + 61,
    (23505613312828786LL << 8) + 62,
    (13868311854568984LL << 8) + 64,
    (8320987112741390LL << 8) + 66,
    (5075802138772248LL << 8) + 68,
    (31469973260387938LL << 8) + 69,
    (19826083154044401LL << 8) + 71,
    (12688693218588416LL << 8) + 73,
    (8247650592082471LL << 8) + 75,
    (5443449390774431LL << 8) + 77,
    (3647111091818869LL << 8) + 79,
    (24800355424368306LL << 8) + 80,
    (17112245242814131LL << 8) + 82,
    (11978571669969892LL << 8) + 84,
    (8504785885678623LL << 8) + 86,
    (6123445837688609LL << 8) + 88,
    (4470115461512684LL << 8) + 90,
    (33078854415193864LL << 8) + 91,
    (24809140811395398LL << 8) + 93,
    (18854947016660503LL << 8) + 95,
    (14518309202828587LL << 8) + 97,
    (11324281178206298LL << 8) + 99,
    (8946182130782975LL << 8) + 101,
    (7156945704626380LL << 8) + 103,
    (5797126020747368LL << 8) + 105,
    (4753643337012842LL << 8) + 107,
    (3945523969720659LL << 8) + 109,
    (33142401345653533LL << 8) + 110,
    (28171041143805503LL << 8) + 112,
    (24227095383672732LL << 8) + 114,
    (21077572983795277LL << 8) + 116,
    (18548264225739844LL << 8) + 118,
    (16507955160908461LL << 8) + 120,
    (14857159644817615LL << 8) + 122,
    (1352001527678403LL << 8) + 124,
    (12438414054641307LL << 8) + 126
};

dec64 dec64_acos(dec64 slope) {
    dec64 result = dec64_subtract(
        D_HALF_PI,
        dec64_asin(slope)
    );
    return result;
}

dec64 dec64_asin(dec64 slope) {
    if (dec64_equal(slope, DEC64_ONE) == DEC64_TRUE) {
        return D_HALF_PI;
    }
    if (dec64_equal(slope, DEC64_NEGATIVE_ONE) == DEC64_TRUE) {
        return D_NHALF_PI;
    }
    if (
        dec64_is_any_nan(slope) == DEC64_TRUE ||
        dec64_less(DEC64_ONE, dec64_abs(slope)) == DEC64_TRUE
    ) {
        return DEC64_NAN;
    }
    dec64 bottom = D_2;
    dec64 factor = slope;
    dec64 x2 = dec64_multiply(slope, slope);
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

dec64 dec64_atan(dec64 slope) {
    return dec64_asin(
        dec64_divide(
            slope,
            dec64_sqrt(dec64_inc(dec64_multiply(slope, slope)))
        )
    );
}

dec64 dec64_atan2(dec64 y, dec64 x) {
    if (dec64_is_zero(x) == DEC64_TRUE) {
        if (dec64_is_zero(y) == DEC64_TRUE) {
            return DEC64_NAN;
        } else if (y < 0) {
            return D_NHALF_PI;
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

dec64 dec64_cos(dec64 radians) {
    return dec64_sin(dec64_add(radians, D_HALF_PI));
}

dec64 dec64_exp(dec64 exponent) {
        dec64 result = dec64_inc(exponent);
        dec64 divisor = D_2;
        dec64 term = exponent;
        while (1) {
            term = dec64_divide(
                dec64_multiply(term, exponent),
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

dec64 dec64_factorial(dec64 x) {
    dec64 n = dec64_normal(x);
    int64 c = dec64_coefficient(n);
    if (c >= 0 && c < FAC && dec64_exponent(n) == 0) {
        return factorials[c];
    }
    return DEC64_NAN;
}

dec64 dec64_log(dec64 x) {
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
        if (result == progress || progress == DEC64_NAN) {
            break;
        }
        result = progress;
        divisor = dec64_inc(divisor);
    }
    return result;
}

dec64 dec64_raise(dec64 coefficient, dec64 exponent) {
    if (dec64_is_zero(exponent) == DEC64_TRUE) {
        return DEC64_ONE;
    }

// Adjust for a negative exponent.

    if (exponent < 0) {
        coefficient = dec64_divide(DEC64_ONE, coefficient);
        exponent = dec64_neg(exponent);
    }
    if (dec64_is_zero(coefficient) == DEC64_TRUE) {
        return 0;
    }
    if (dec64_is_any_nan(coefficient) == DEC64_TRUE) {
        return DEC64_NAN;
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

/*
    The seed variables contain the random number generator's state.
    They can be set by dec64_seed.
*/

static uint64 seed_0 = D_E;
static uint64 seed_1 = D_2PI;

dec64 dec64_random() {
/*
    Return a number between 0 and 1 containing 16 randomy digits.
    It uses xorshift128+.
*/
    while (1) {
        uint64 s1 = seed_0;
        uint64 s0 = seed_1;
        s1 ^= s1 << 23;
        s1 ^= s0 ^ (s0 >> 5) ^ (s1 >> 18);
        seed_0 = s0;
        seed_1 = s1;
        uint64 mantissa = (s1 + s0) >> 10;
/*
    mantissa contains an integer between 0 and 18014398509481983.
    If it is less than or equal to 9999999999999999 then we are done.
*/
        if (mantissa <= 9999999999999999LL) {
            return dec64_new(mantissa, -16);
        }
    }
}

dec64 dec64_root(dec64 degree, dec64 radicand) {
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
    result = DEC64_ONE;
    dec64 prosult = DEC64_NAN;
    while (1) {
        dec64 progress = dec64_divide(
            dec64_add(
                dec64_multiply(result, degree_minus_one),
                dec64_divide(
                    radicand,
                    dec64_raise(result, degree_minus_one)
                )
            ),
            degree
        );
        if (progress == result) {
            return result;
        }
        if (progress == prosult) {
            return dec64_half(dec64_add(progress, result));
        }
        prosult = result;
        result = progress;
    }
}

void dec64_seed(dec64 seed) {
/*
    Seed the dec64_random function. It takes any 64 bits as the seed value.
    It initializes the seed variables.
*/
    seed_0 = seed;
    seed_1 = (seed | 1) * D_2PI;
}

dec64 dec64_sin(dec64 radians) {
    while (dec64_less(D_PI, radians) == DEC64_TRUE) {
        radians = dec64_subtract(radians, D_PI);
        radians = dec64_subtract(radians, D_PI);
    }
    while (dec64_less(radians, D_NPI) == DEC64_TRUE) {
        radians = dec64_add(radians, D_PI);
        radians = dec64_add(radians, D_PI);
    }
    int neg = 0;
    if (radians < 0) {
        radians = dec64_neg(radians);
        neg = -1;
    }
    if (dec64_less(D_HALF_PI, radians) == DEC64_TRUE) {
        radians = dec64_subtract(D_PI, radians);
    }
    dec64 result;
    if (radians == D_HALF_PI) {
        result = DEC64_ONE;
    } else {
        dec64 x2 = dec64_multiply(radians, radians);
        dec64 order = DEC64_ONE;
        dec64 term = radians;
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
        dec64 result = radicand;
        while (1) {
            dec64 progress = dec64_half(dec64_add(
                result,
                dec64_divide(radicand, result)
            ));
            if (progress == result) {
                return result;
            }
            result = progress;
        }
        return result;
    } else {
        return DEC64_NAN;
    }
}

dec64 dec64_tan(dec64 radians) {
    return dec64_divide(
        dec64_sin(radians),
        dec64_cos(radians)
    );
}
