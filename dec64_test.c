/* dec64_test.c

This is a test of dec64.asm.

dec64.com
2014-06-28
Public Domain

No warranty.
*/

#include <stdio.h>
#include "dec64.h"

static int level;
static int nr_fail;
static int nr_pass;

/* constants */

static dec64 nan;
static dec64 nannan;
static dec64 zero;
static dec64 zip;
static dec64 one;
static dec64 two;
static dec64 three;
static dec64 four;
static dec64 five;
static dec64 six;
static dec64 seven;
static dec64 eight;
static dec64 nine;
static dec64 ten;
static dec64 maxint;
static dec64 maxint_plus;
static dec64 maxnum;
static dec64 minnum;
static dec64 epsilon;
static dec64 almost_one;
static dec64 almost_negative_one;
static dec64 e;
static dec64 pi;
static dec64 half;
static dec64 cent;
static dec64 negative_one;
static dec64 negative_nine;
static dec64 negative_minnum;
static dec64 negative_maxint;
static dec64 negative_maxnum;
static dec64 negative_pi;

static void define_constants() {
    nan = dec64_nan();              /* not a number */
    nannan = 32896;                 /* a non-normal nan */
    zero = dec64_zero();            /* 0 */
    zip = 250;                      /* a non normal 0 */
    one = dec64_one();              /* 1 */
    two = dec64_new(2, 0);          /* 2 */
    three = dec64_new(3, 0);        /* 3 */
    four = dec64_new(4, 0);         /* 4 */
    five = dec64_new(5, 0);         /* 5 */
    six = dec64_new(6, 0);          /* 6 */
    seven = dec64_new(7, 0);        /* 7 */
    eight = dec64_new(8, 0);        /* 8 */
    nine = dec64_new(9, 0);         /* 9 */
    ten = dec64_new(10, 0);         /* 10 */
    minnum = dec64_new(1, -127);    /* the smallest possible number */
    epsilon = dec64_new(1, -16);    /* the smallest number addable to 1 */
    cent = dec64_new(1, -2);        /* 0.01 */
    half = dec64_new(5, -1);        /* 0.5 */
    almost_one = dec64_new(9999999999999999, -16);
                                    /* 0.9999999999999999 */
    e = dec64_new(27182818284590452, -16);
                                    /* e */
    pi = dec64_new(31415926535897932, -16);
                                    /* pi */
    maxint = dec64_new(36028797018963967, 0);
                                    /* the largest normal integer */
    maxint_plus = dec64_new(3602879701896397, 1);
                                    /* the smallest number larger than maxint */
    maxnum = dec64_new(36028797018963967, 127);
                                    /* the largest possible number */
    negative_minnum = dec64_new(-1, -127);
                                    /* the smallest possible negative number */
    negative_one = dec64_new(-1, 0);/* -1 */
    negative_nine = dec64_new(-9, 0);
                                    /* -9 */
    negative_pi = dec64_new(-31415926535897932, -16);
                                    /* -pi */
    negative_maxint = dec64_new(-36028797018963968, 0);
                                    /* the largest negative normal integer */
    negative_maxnum = dec64_new(-36028797018963968, 127);
                                    /* the largest possible negative number */
    almost_negative_one = dec64_new(-9999999999999999, -16);
                                    /* -0.9999999999999999 */
}

static void print_dec64(dec64 number) {
    int64 exponent = dec64_exponent(number);
    int64 coefficient = dec64_coefficient(number);
    printf("%20lli", coefficient);
    if (exponent == -128) {
        printf(" nan");
    } else if (exponent != 0) {
        printf("e%-+4lli", exponent);
    }
}

static dec64 normal(dec64 number) {
    int64 coef;
    int64 coefficient = dec64_coefficient(number);
    int64 exponent = dec64_exponent(number);
    if (exponent == 0) {
        return number;
    }
    if (exponent < 0) {
        do {
            coef = coefficient / 10;
            if (coefficient != coef * 10) {
                break;
            }
            coefficient = coef;
            exponent += 1;
        } while (exponent < 0);
    } else {
        do {
            coef = coefficient * 10;
            if (coef >= 36028797018963968 || coef < -36028797018963968) {
                break;
            }
            coefficient = coef;
            exponent -= 1;
        } while (exponent > 0);
    }
    return dec64_new(coefficient, exponent);
}

static int compare(dec64 first, dec64 second) {
    if (first != 0 && second != 0 && (first & 255) != 128 && (second & 255) != 128) {
        first = normal(first);
        second = normal(second);
    }
    return first == second;
}

static void judge_not(dec64 expected, dec64 actual, char * name, char * comment) {
    if (compare(expected, actual)) {
        nr_pass += 1;
        if (level >= 3) {
            printf("\n\npass %s: %s", name, comment);
            printf("\n%-4s", "=");
            print_dec64(actual);
        }
    } else {
        nr_fail += 1;
        if (level >= 1) {
            printf("\n\nFAIL %s: %s", name, comment);
            if (level >= 2) {
                printf("\n%-4s", "?");
                print_dec64(actual);
                printf("\n%-4s", "=");
                print_dec64(expected);
            }
        }
    }
}

static void judge_unary(dec64 first, dec64 expected, dec64 actual, char * name, char * op, char * comment) {
    if (compare(expected, actual)) {
        nr_pass += 1;
        if (level >= 3) {
            printf("\n\npass %s: %s", name, comment);
            printf("\n%-4s", op);
            print_dec64(first);
            printf("\n%-4s", "=");
            print_dec64(actual);
        }
    } else {
        nr_fail += 1;
        if (level >= 1) {
            printf("\n\nFAIL %s: %s", name, comment);
            if (level >= 2) {
                printf("\n%-4s", op);
                print_dec64(first);
                printf("\n%-4s", "?");
                print_dec64(actual);
                printf("\n%-4s", "=");
                print_dec64(expected);
            }
        }
    }
}

static void judge_binary(dec64 first, dec64 second, dec64 expected, dec64 actual, char * name, char * op, char * comment) {
    if (compare(expected, actual)) {
        nr_pass += 1;
        if (level >= 3) {
            printf("\n\npass %s: %s", name, comment);
            printf("\n%-4s", "");
            print_dec64(first);
            printf("\n%-4s", op);
            print_dec64(second);
            printf("\n%-4s", "=");
            print_dec64(actual);
        }
    } else {
        nr_fail += 1;
        if (level >= 1) {
            printf("\n\nFAIL %s: %s", name, comment);
            if (level >= 2) {
                printf("\n%-4s", "");
                print_dec64(first);
                printf("\n%-4s", op);
                print_dec64(second);
                printf("\n%-4s", "?");
                print_dec64(actual);
                printf("\n%-4s", "=");
                print_dec64(expected);
            }
        }
    }
}

static void judge_communitive(dec64 first, dec64 second, dec64 expected, dec64 actual, char * name, char * op, char * comment) {
    judge_binary(first, second, expected, actual, name, op, comment);
    if (first != second) {
        judge_binary(second, first, expected, actual, name, op, comment);
    }
}

static void test_abs(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_abs(first);
    judge_unary(first, expected, actual, "abs", "a", comment);
}

static void test_add(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_add(first, second);
    judge_communitive(first, second, expected, actual, "add", "+", comment);
}

static void test_ceiling(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_ceiling(first);
    judge_unary(first, expected, actual, "ceiling", "c", comment);
}

static void test_divide(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_divide(first, second);
    judge_binary(first, second, expected, actual, "divide", "/", comment);
}

static void test_equal(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_equal(first, second);
    judge_communitive(first, second, expected, actual, "equal", "=", comment);
}

static void test_floor(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_floor(first);
    judge_unary(first, expected, actual, "floor", "f", comment);
}

static void test_integer_divide(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_integer_divide(first, second);
    judge_binary(first, second, expected, actual, "integer_divide", "/", comment);
}

static void test_is_integer(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_is_integer(first);
    judge_unary(first, expected, actual, "is_integer", "i", comment);
}

static void test_is_nan(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_is_nan(first);
    judge_unary(first, expected, actual, "is_nan", "n", comment);
}

static void test_is_zero(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_is_zero(first);
    judge_unary(first, expected, actual, "is_zero", "0", comment);
}

static void test_less(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_less(first, second);
    judge_binary(first, second, expected, actual, "less", "<", comment);
}

static void test_modulo(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_modulo(first, second);
    judge_binary(first, second, expected, actual, "modulo", "%", comment);
}

static void test_multiply(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_multiply(first, second);
    judge_communitive(first, second, expected, actual, "multiply", "*", comment);
}

static void test_neg(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_neg(first);
    judge_unary(first, expected, actual, "neg", "n", comment);
}

static void test_new(int64 coefficient, int64 exponent, dec64 expected, char * comment) {
    dec64 actual = dec64_new(coefficient, exponent);
    judge_not(expected, actual, "new", comment);
}

static void test_normal(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_normal(first);
    judge_unary(first, expected, actual, "normal", "n", comment);
}

static void test_not(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_not(first);
    judge_unary(first, expected, actual, "not", "!", comment);
}

static void test_round(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_round(first, second);
    judge_binary(first, second, expected, actual, "round", "r", comment);
}

static void test_signum(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_signum(first);
    judge_unary(first, expected, actual, "signum", "s", comment);
}

static void test_subtract(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_subtract(first, second);
    judge_binary(first, second, expected, actual, "subtract", "-", comment);
}

static void test_all_abs() {
    test_abs(nan, nan, "nan");
    test_abs(nannan, nan, "nannan");
    test_abs(zero, zero, "zero");
    test_abs(zip, zero, "zip");
    test_abs(100, zero, "zero alias");
    test_abs(one, one, "one");
    test_abs(negative_one, one, "-1");
    test_abs(almost_negative_one, almost_one, "almost_negative_one");
    test_abs(negative_maxint, maxint_plus, "-maxint");
    test_abs(negative_maxnum, nan, "-maxnum");
    test_abs(maxnum, maxnum, "maxnum");
}

static void test_all_add() {
    test_add(nan, zero, nan, "nan + zero");
    test_add(nan, nan, nan, "nan + nan");
    test_add(nannan, one, nan, "nannan + 1");
    test_add(nannan, nannan, nan, "nannan + nannan");
    test_add(zero, nannan, nan, "0 + nannan");
    test_add(zero, zip, zero, "zero + zip");
    test_add(zip, zero, zero, "zip + zero");
    test_add(zip, zip, zero, "zip + zip");
    test_add(almost_one, epsilon, one, "almost_one + epsilon");
    test_add(almost_one, nine, ten, "almost_one + 1");
    test_add(one, nan, nan, "one + nan");
    test_add(one, one, two, "one + one");
    test_add(one, cent, dec64_new(101, -2), "one + cent");
    test_add(one, epsilon, dec64_new(10000000000000001, -16), "1 + epsilon");
    test_add(three, four, seven, "three + four");
    test_add(four, epsilon, dec64_new(4000000000000000, -15), "4 + epsilon");
    test_add(dec64_new(1, 2), dec64_new(-1, -2), dec64_new(9999, -2), "100 - 0.01");
    test_add(dec64_new(10, 10), dec64_new(20, 10), dec64_new(30, 10), "10e10 + 20e10");
    test_add(dec64_new(199, -2), dec64_new(299, -2), dec64_new(498, -2), "1.99 + 2.99");
    test_add(dec64_new(36028797018963967, 126), dec64_new(36028797018963967, 126), dec64_new(7205759403792793, 127), "test overflow with big exponents");
    test_add(dec64_new(9999999999999999, 0), one, dec64_new(10000000000000000, 0), "9999999999999999 + 1");
    test_add(negative_one, epsilon, almost_negative_one, "-1 + epsilon");
    test_add(negative_pi, pi, zero, "-pi + pi");
    test_add(maxint, one, maxint_plus, "maxint + one");
    test_add(maxint, half, maxint_plus, "maxint + half");
    test_add(maxint, cent, maxint, "maxint + cent");
    test_add(maxint, dec64_new(4999999999, -11), maxint, "maxint + 0.4999999999");
    test_add(maxint, maxint, dec64_new(7205759403792793, 1), "maxint + maxint");
    test_add(maxint, dec64_new(111, -2), maxint_plus, "maxint + 1.11");
    test_add(maxint, dec64_new(36028797018963967, -20), maxint, "maxint + something too small");
    test_add(maxint, dec64_new(30000000000000000, -16), maxint_plus, "maxint + 3");
    test_add(maxint, dec64_new(20000000000000000, -16), maxint_plus, "maxint + something too small");
    test_add(maxint, negative_maxint, dec64_new(-1, 0), "maxint + -maxint");
    test_add(maxnum, dec64_new(1, -127), maxnum, "insignificance");
    test_add(maxnum, one, maxnum, "insignificance");
    test_add(maxnum, maxint, maxnum, "insignificance");
    test_add(maxnum, dec64_new(1, 127), nan, "overflow the exponent");
    test_add(maxnum, dec64_new(10, 126), nan, "overflow the exponent");
    test_add(maxnum, dec64_new(100, 125), nan, "overflow the exponent");
    test_add(maxnum, dec64_new(1000, 124), nan, "overflow the exponent");
    test_add(maxnum, dec64_new(500, 124), nan, "overflow the exponent");
    test_add(maxnum, maxnum, nan, "overflow the exponent");
    test_add(maxnum, dec64_new(-36028797018963967, 127), zero, "extreme zero");
    test_add(almost_negative_one, one, epsilon, "almost_negative_one + one");
}

static void test_all_ceiling() {
    test_ceiling(nan, nan, "nan");
    test_ceiling(nannan, nan, "nannan");
    test_ceiling(zero, zero, "zero");
    test_ceiling(zip, zero, "zero");
    test_ceiling(minnum, one, "minnum");
    test_ceiling(epsilon, one, "epsilon");
    test_ceiling(cent, one, "cent");
    test_ceiling(half, one, "half");
    test_ceiling(one, one, "one");
    test_ceiling(negative_one, negative_one, "negative_one");
    test_ceiling(dec64_new(10000000000000001, -16), two, "1.0000000000000001");
    test_ceiling(dec64_new(-10000000000000001, -16), negative_one, "-1.0000000000000001");
    test_ceiling(dec64_new(20000000000000000, -16), two, "two");
    test_ceiling(e, three, "e");
    test_ceiling(pi, four, "pi");
    test_ceiling(negative_pi, dec64_new(-3, 0), "-pi");
    test_ceiling(maxint, maxint, "maxint");
    test_ceiling(maxnum, maxnum, "maxnum");
    test_ceiling(negative_maxint, negative_maxint, "negative_maxint");
    test_ceiling(dec64_new(11111111111111111, -17), one, "0.1...");
    test_ceiling(dec64_new(22222222222222222, -17), one, "0.2...");
    test_ceiling(dec64_new(33333333333333333, -17), one, "0.3...");
    test_ceiling(dec64_new(4444444444444444, -16), one, "0.4...");
    test_ceiling(dec64_new(5555555555555556, -16), one, "0.5...");
    test_ceiling(dec64_new(6666666666666667, -16), one, "0.6...");
    test_ceiling(dec64_new(7777777777777778, -16), one, "0.7...");
    test_ceiling(dec64_new(8888888888888889, -16), one, "0.8...");
    test_ceiling(dec64_new(10000000000000000, -16), one, "1");
    test_ceiling(dec64_new(-12500000000000000, -16), dec64_new(-1, 0), "-1.25");
    test_ceiling(dec64_new(-1500000000000000, -15), dec64_new(-1, 0), "-1.5");
    test_ceiling(dec64_new(-1560000000000000, -15), dec64_new(-1, 0), "-1.56");
    test_ceiling(dec64_new(-11111111111111111, -17), zero, "-0.1...");
    test_ceiling(dec64_new(-22222222222222222, -17), zero, "-0.2...");
    test_ceiling(dec64_new(-33333333333333333, -17), zero, "-0.3...");
    test_ceiling(dec64_new(-4444444444444444, -16), zero, "-0.4...");
    test_ceiling(dec64_new(-5555555555555556, -16), zero, "-0.5...");
    test_ceiling(dec64_new(-6666666666666667, -16), zero, "-0.6...");
    test_ceiling(dec64_new(-7777777777777778, -16), zero, "-0.7...");
    test_ceiling(dec64_new(-8888888888888889, -16), zero, "-0.8...");
    test_ceiling(dec64_new(-10000000000000000, -16), negative_one, "-10000000000000000e-16");
    test_ceiling(dec64_new(449, -2), five, "4.49");
    test_ceiling(dec64_new(-449, -2), dec64_new(-4, 0), "-4.49");
    test_ceiling(dec64_new(450, -2), five, "4.50");
    test_ceiling(dec64_new(-450, -2), dec64_new(-4, 0), "-4.50");
    test_ceiling(dec64_new(9, -1), one, "0.9");
    test_ceiling(dec64_new(-9, -1), zero, "-0.9");
    test_ceiling(almost_one, one, "almost_one");
    test_ceiling(almost_negative_one, zero, "almost_negative_one");
    test_ceiling(dec64_new(-999999999999999, -15), zero, "-0.9...");
    test_ceiling(dec64_new(-9999999999999998, -16), zero, "-0.9...8");
}

static void test_all_divide() {
    test_divide(dec64_new(4195835, 0), dec64_new(3145727, 0), dec64_new(13338204491362410, -16), "4195835 / 3145727");
    test_divide(nan, nan, nan, "nan / nan");
    test_divide(nan, three, nan, "nan / 3");
    test_divide(nannan, nannan, nan, "nannan / nannan");
    test_divide(nannan, one, nan, "nannan / 1");
    test_divide(zero, nan, zero, "0 / nan");
    test_divide(zero, nannan, zero, "0 / nannan");
    test_divide(zero, zip, zero, "zero / zip");
    test_divide(zip, nan, zero, "zip / nan");
    test_divide(zip, nannan, zero, "zip / nannan");
    test_divide(zip, zero, zero, "zip / zero");
    test_divide(zip, zip, zero, "zip / zip");
    test_divide(zero, one, zero, "0 / 1");
    test_divide(zero, zero, zero, "0 / 0");
    test_divide(one, zero, nan, "1 / 0");
    test_divide(one, negative_one, dec64_new(-10000000000000000, -16), "1 / -1");
    test_divide(negative_one, one, dec64_new(-10000000000000000, -16), "-1 / 1");
    test_divide(one, two, dec64_new(5000000000000000, -16), "1 / 2");
    test_divide(one, three, dec64_new(33333333333333333, -17), "1 / 3");
    test_divide(two, three, dec64_new(6666666666666667, -16), "2 / 3");
    test_divide(two, dec64_new(30000000000000000, -16), dec64_new(6666666666666667, -16), "2 / 3 alias");
    test_divide(dec64_new(20000000000000000, -16), three, dec64_new(6666666666666667, -16), "2 / 3 alias");
    test_divide(dec64_new(20000000000000000, -16), dec64_new(30000000000000000, -16), dec64_new(6666666666666667, -16), "2 / 3 alias");
    test_divide(five, three, dec64_new(16666666666666667, -16), "5 / 3");
    test_divide(five, dec64_new(-30000000000000000, -16), dec64_new(-16666666666666667, -16), "5 / -3");
    test_divide(dec64_new(-50000000000000000, -16), three, dec64_new(-16666666666666667, -16), "-5 / 3");
    test_divide(dec64_new(-50000000000000000, -16), dec64_new(-30000000000000000, -16), dec64_new(16666666666666667, -16), "-5 / -3");
    test_divide(six, nan, nan, "6 / nan");
    test_divide(six, three, dec64_new(20000000000000000, -16), "6 / 3");
    test_divide(zero, nine, zero, "0 / 9");
    test_divide(one, nine, dec64_new(11111111111111111, -17), "1 / 9");
    test_divide(two, nine, dec64_new(22222222222222222, -17), "2 / 9");
    test_divide(three, nine, dec64_new(33333333333333333, -17), "3 / 9");
    test_divide(four, nine, dec64_new(4444444444444444, -16), "4 / 9");
    test_divide(five, nine, dec64_new(5555555555555556, -16), "5 / 9");
    test_divide(six, nine, dec64_new(6666666666666667, -16), "6 / 9");
    test_divide(seven, nine, dec64_new(7777777777777778, -16), "7 / 9");
    test_divide(eight, nine, dec64_new(8888888888888889, -16), "8 / 9");
    test_divide(nine, nine, one, "9 / 9");
    test_divide(zero, negative_nine, zero, "0 / -9");
    test_divide(one, negative_nine, dec64_new(-11111111111111111, -17), "1 / -9");
    test_divide(two, negative_nine, dec64_new(-22222222222222222, -17), "2 / -9");
    test_divide(three, negative_nine, dec64_new(-33333333333333333, -17), "3 / -9");
    test_divide(four, negative_nine, dec64_new(-4444444444444444, -16), "4 / -9");
    test_divide(five, negative_nine, dec64_new(-5555555555555556, -16), "5 / -9");
    test_divide(six, negative_nine, dec64_new(-6666666666666667, -16), "6 / -9");
    test_divide(seven, negative_nine, dec64_new(-7777777777777778, -16), "7 / -9");
    test_divide(eight, negative_nine, dec64_new(-8888888888888889, -16), "8 / -9");
    test_divide(nine, negative_nine, negative_one, "9 / -9");
    test_divide(pi, negative_pi, dec64_new(-10000000000000000, -16), "pi / -pi");
    test_divide(negative_pi, pi, dec64_new(-10000000000000000, -16), "-pi / pi");
    test_divide(negative_pi, negative_pi, dec64_new(10000000000000000, -16), "-pi / -pi");
    test_divide(dec64_new(-16, 0), ten, dec64_new(-16, -1), "-16 / 10");
    test_divide(maxint, epsilon, dec64_new(36028797018963967, 16), "maxint / epsilon");
    test_divide(one, maxint, dec64_new(27755575615628914, -33), "one / maxint");
    test_divide(one, negative_maxint, dec64_new(-27755575615628914, -33), "one / -maxint");
    test_divide(maxnum, epsilon, nan, "maxnum / epsilon");
    test_divide(maxnum, maxnum, dec64_new(10000000000000000, -16), "maxnum / maxnum");
    test_divide(dec64_new(10, -1), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 1");
    test_divide(dec64_new(100, -2), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 2");
    test_divide(dec64_new(1000, -3), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 3");
    test_divide(dec64_new(10000, -4), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 4");
    test_divide(dec64_new(100000, -5), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 5");
    test_divide(dec64_new(1000000, -6), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 6");
    test_divide(dec64_new(10000000, -7), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 7");
    test_divide(dec64_new(100000000, -8), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 8");
    test_divide(dec64_new(1000000000, -9), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 9");
    test_divide(dec64_new(10000000000, -10), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 10");
    test_divide(dec64_new(100000000000, -11), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 11");
    test_divide(dec64_new(1000000000000, -12), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 12");
    test_divide(dec64_new(10000000000000, -13), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 13");
    test_divide(dec64_new(100000000000000, -14), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 14");
    test_divide(dec64_new(1000000000000000, -15), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 15");
    test_divide(dec64_new(10000000000000000, -16), maxint, dec64_new(27755575615628914, -33), "one / maxint alias 16");
}

static void test_all_equal() {
    test_equal(nan, nan, one, "nan = nan");
    test_equal(nan, zero, zero, "nan = zero");
    test_equal(nan, nannan, one, "nan = nannan");
    test_equal(nannan, nannan, one, "nannan = nannan");
    test_equal(nannan, nan, one, "nannan = nan");
    test_equal(nannan, one, zero, "nannan = 1");
    test_equal(zero, nan, zero, "zero = nan");
    test_equal(zero, nannan, zero, "0 = nannan");
    test_equal(zero, zip, one, "zero = zip");
    test_equal(zero, minnum, zero, "zero = minnum");
    test_equal(zero, one, zero, "zero = one");
    test_equal(zip, zero, one, "zip = zero");
    test_equal(zip, zip, one, "zip = zip");
    test_equal(one, negative_one, zero, "1 = -1");
    test_equal(two, two, one, "2 = 2");
    test_equal(two, dec64_new(2, -16), zero, "2 = 2e-16");
    test_equal(pi, three, zero, "pi = 3");
    test_equal(maxint, maxnum, zero, "maxint = maxnum");
    test_equal(negative_maxint, maxint, zero, "-maxint = maxint");
    test_equal(negative_maxint, negative_one, zero, "-maxint = -1");
}

static void test_all_floor() {
    test_floor(nan, nan, "nan");
    test_floor(nannan, nan, "nannan");
    test_floor(zero, zero, "zero");
    test_floor(zip, zero, "zero");
    test_floor(minnum, zero, "minnum");
    test_floor(epsilon, zero, "epsilon");
    test_floor(cent, zero, "cent");
    test_floor(half, zero, "half");
    test_floor(one, one, "one");
    test_floor(negative_one, negative_one, "negative_one");
    test_floor(dec64_new(10000000000000001, -16), one, "1.0000000000000001");
    test_floor(dec64_new(-10000000000000001, -16), dec64_new(-2, 0), "-1.0000000000000001");
    test_floor(dec64_new(20000000000000000, -16), two, "two");
    test_floor(e, two, "e");
    test_floor(pi, three, "pi");
    test_floor(negative_pi, dec64_new(-4, 0), "-pi");
    test_floor(maxint, maxint, "maxint");
    test_floor(maxnum, maxnum, "maxnum");
    test_floor(negative_maxint, negative_maxint, "negative_maxint");
    test_floor(dec64_new(11111111111111111, -17), zero, "0.1...");
    test_floor(dec64_new(22222222222222222, -17), zero, "0.2...");
    test_floor(dec64_new(33333333333333333, -17), zero, "0.3...");
    test_floor(dec64_new(4444444444444444, -16), zero, "0.4...");
    test_floor(dec64_new(5555555555555556, -16), zero, "0.5...");
    test_floor(dec64_new(6666666666666667, -16), zero, "0.6...");
    test_floor(dec64_new(7777777777777778, -16), zero, "0.7...");
    test_floor(dec64_new(8888888888888889, -16), zero, "0.8...");
    test_floor(dec64_new(9999999999999999, -16), zero, "0.9...");
    test_floor(dec64_new(10000000000000000, -16), one, "1");
    test_floor(dec64_new(-12500000000000000, -16), dec64_new(-2, 0), "-1.25");
    test_floor(dec64_new(-1500000000000000, -15), dec64_new(-2, 0), "-1.5");
    test_floor(dec64_new(-1560000000000000, -15), dec64_new(-2, 0), "-1.56");
    test_floor(dec64_new(-11111111111111111, -17), negative_one, "-0.1...");
    test_floor(dec64_new(-22222222222222222, -17), negative_one, "-0.2...");
    test_floor(dec64_new(-33333333333333333, -17), negative_one, "-0.3...");
    test_floor(dec64_new(-4444444444444444, -16), negative_one, "-0.4...");
    test_floor(dec64_new(-5555555555555556, -16), negative_one, "-0.5...");
    test_floor(dec64_new(-6666666666666667, -16), negative_one, "-0.6...");
    test_floor(dec64_new(-7777777777777778, -16), negative_one, "-0.7...");
    test_floor(dec64_new(-8888888888888889, -16), negative_one, "-0.8...");
    test_floor(dec64_new(-9999999999999999, -16), negative_one, "-0.9...");
    test_floor(dec64_new(-10000000000000000, -16), negative_one, "-1.0...");
    test_floor(dec64_new(449, -2), four, "4.49");
    test_floor(dec64_new(-449, -2), dec64_new(-5, 0), "-4.49");
    test_floor(dec64_new(450, -2), four, "4.50");
    test_floor(dec64_new(-450, -2), dec64_new(-5, 0), "-4.50");
    test_floor(dec64_new(-400, -2), dec64_new(-4, 0), "-4.00");
    test_floor(dec64_new(-400, -3), negative_one, "-0.400");
    test_floor(dec64_new(-1, -127), negative_one, "-1e-127");
    test_floor(dec64_new(-1, -13), negative_one, "-1e-13");
    test_floor(dec64_new(1, -12), zero, "1e-12");
    test_floor(dec64_new(-1, -12), negative_one, "-1e-12");
    test_floor(dec64_new(-1, -11), negative_one, "-1e-11");
    test_floor(dec64_new(-11, -11), negative_one, "-11e-11");
    test_floor(dec64_new(-111, -11), negative_one, "-111e-11");
    test_floor(dec64_new(-22, -11), negative_one, "-22e-11");
    test_floor(dec64_new(-1, -1), negative_one, "-1e-1");
    test_floor(dec64_new(-10, -3), negative_one, "-10e-3");
    test_floor(dec64_new(9, -1), zero, "0.9");
    test_floor(dec64_new(-9, -1), negative_one, "-0.9");
    test_floor(almost_one, zero, "almost_one");
    test_floor(almost_negative_one, negative_one, "almost_negative_one");
    test_floor(dec64_new(-999999999999999, -15), negative_one, "-0.9...");
    test_floor(dec64_new(-9999999999999998, -16), negative_one, "-0.9...8");
}

static void test_all_integer_divide() {
    test_integer_divide(nan, three, nan, "nan / 3");
    test_integer_divide(six, nan, nan, "6 / nan");
    test_integer_divide(nan, nan, nan, "nan / nan");
    test_integer_divide(nannan, one, nan, "nannan / 1");
    test_integer_divide(zip, zero, zero, "zip / zero");
    test_integer_divide(zero, zip, zero, "zero / zip");
    test_integer_divide(zip, zip, zero, "zip / zip");
    test_integer_divide(zero, nan, zero, "0 / nan");
    test_integer_divide(zero, nannan, zero, "0 / nannan");
    test_integer_divide(zip, nan, zero, "zip / nan");
    test_integer_divide(zip, nannan, zero, "zip / nannan");
    test_integer_divide(nannan, nannan, nan, "nannan / nannan");
    test_integer_divide(zero, one, zero, "0 / 1");
    test_integer_divide(zero, zero, zero, "0 / 0");
    test_integer_divide(one, one, one, "1 / 1");
    test_integer_divide(one, zero, nan, "1 / 0");
    test_integer_divide(one, negative_one, negative_one, "1 / -1");
    test_integer_divide(negative_one, one, negative_one, "-1 / 1");
    test_integer_divide(pi, negative_pi, negative_one, "pi / -pi");
    test_integer_divide(negative_pi, pi, negative_one, "-pi / pi");
    test_integer_divide(negative_pi, negative_pi, one, "-pi / -pi");
    test_integer_divide(six, three, two, "6 / 3");
    test_integer_divide(maxnum, maxnum, one, "maxnum / maxnum");
    test_integer_divide(one, two, zero, "1 / 2");
    test_integer_divide(one, three, zero, "1 / 3");
    test_integer_divide(two, three, zero, "2 / 3");
    test_integer_divide(two, dec64_new(30000000000000000, -16), zero, "2 / 3");
    test_integer_divide(dec64_new(20000000000000000, -16), three, zero, "2 / 3");
    test_integer_divide(three, three, one, "3 / 3");
    test_integer_divide(three, dec64_new(30000000000000000, -16), one, "3 / 3");
    test_integer_divide(four, three, one, "4 / 3");
    test_integer_divide(dec64_new(40000000000000000, -16), dec64_new(30000000000000000, -16), one, "4 / 3");
    test_integer_divide(five, three, one, "5 / 3");
    test_integer_divide(five, dec64_new(-3, 0), dec64_new(-2, 0), "5 / -3");
    test_integer_divide(five, dec64_new(-30000000000000000, -16), dec64_new(-2, 0), "5 / -3 alias");
    test_integer_divide(dec64_new(-5, 0), three, dec64_new(-2, 0), "-5 / 3");
    test_integer_divide(dec64_new(-5, 0), dec64_new(-3, 0), one, "-5 / -3");
    test_integer_divide(dec64_new(-5, 0), dec64_new(-30000000000000000, -16), one, "-5 / -3");
    test_integer_divide(dec64_new(-50000000000000000, -16), dec64_new(-30000000000000000, -16), one, "-5 / -3");
    test_integer_divide(dec64_new(-50000000000000000, -16), three, dec64_new(-2, 0), "-5 / 3");
    test_integer_divide(dec64_new(-16, 0), ten, dec64_new(-2, 0), "-16 / 10");
    test_integer_divide(maxnum, epsilon, nan, "maxnum / epsilon");
    test_integer_divide(maxint, epsilon, dec64_new(36028797018963967, 16), "maxint / epsilon");
    test_integer_divide(dec64_new(10, -1), maxint, zero, "one / maxint");
}

static void test_all_is_integer() {
    test_is_integer(nan, zero, "nan");
    test_is_integer(nannan, zero, "nannan");
    test_is_integer(zero, one, "zero");
    test_is_integer(zip, one, "zip");
    test_is_integer(minnum, zero, "minnum");
    test_is_integer(epsilon, zero, "epsilon");
    test_is_integer(cent, zero, "cent");
    test_is_integer(half, zero, "half");
    test_is_integer(one, one, "one");
    test_is_integer(negative_one, one, "negative_one");
    test_is_integer(dec64_new(10000000000000001, -16), zero, "1.0000000000000001");
    test_is_integer(dec64_new(-10000000000000001, -16), zero, "-1.0000000000000001");
    test_is_integer(dec64_new(20000000000000000, -16), one, "two");
    test_is_integer(e, zero, "e");
    test_is_integer(pi, zero, "pi");
    test_is_integer(negative_pi, zero, "-pi");
    test_is_integer(maxint, one, "maxint");
    test_is_integer(maxnum, one, "maxnum");
    test_is_integer(negative_maxint, one, "negative_maxint");
    test_is_integer(dec64_new(11111111111111111, -17), zero, "0.1...");
    test_is_integer(dec64_new(22222222222222222, -17), zero, "0.2...");
    test_is_integer(dec64_new(33333333333333333, -17), zero, "0.3...");
    test_is_integer(dec64_new(4444444444444444, -16), zero, "0.4...");
    test_is_integer(dec64_new(5555555555555556, -16), zero, "0.5...");
    test_is_integer(dec64_new(6666666666666667, -16), zero, "0.6...");
    test_is_integer(dec64_new(7777777777777778, -16), zero, "0.7...");
    test_is_integer(dec64_new(8888888888888889, -16), zero, "0.8...");
    test_is_integer(dec64_new(9999999999999999, -16), zero, "0.9...");
    test_is_integer(dec64_new(10000000000000000, -16), one, "1");
    test_is_integer(dec64_new(-12500000000000000, -16), zero, "-1.25");
    test_is_integer(dec64_new(-1500000000000000, -15), zero, "-1.5");
    test_is_integer(dec64_new(-1560000000000000, -15), zero, "-1.56");
    test_is_integer(dec64_new(-11111111111111111, -17), zero, "-0.1...");
    test_is_integer(dec64_new(-22222222222222222, -17), zero, "-0.2...");
    test_is_integer(dec64_new(-33333333333333333, -17), zero, "-0.3...");
    test_is_integer(dec64_new(-4444444444444444, -16), zero, "-0.4...");
    test_is_integer(dec64_new(-5555555555555556, -16), zero, "-0.5...");
    test_is_integer(dec64_new(-6666666666666667, -16), zero, "-0.6...");
    test_is_integer(dec64_new(-7777777777777778, -16), zero, "-0.7...");
    test_is_integer(dec64_new(-8888888888888889, -16), zero, "-0.8...");
    test_is_integer(dec64_new(-9999999999999999, -16), zero, "-0.9...");
    test_is_integer(dec64_new(-10000000000000000, -16), one, "-1.0...");
    test_is_integer(dec64_new(449, -2), zero, "4.49");
    test_is_integer(dec64_new(-449, -2), zero, "-4.49");
    test_is_integer(dec64_new(450, -2), zero, "4.50");
    test_is_integer(dec64_new(-450, -2), zero, "-4.50");
    test_is_integer(dec64_new(-400, -2), one, "-4.00");
    test_is_integer(dec64_new(-400, -3), zero, "-0.400");
    test_is_integer(dec64_new(-1, -127), zero, "-1e-127");
    test_is_integer(dec64_new(-1, -13), zero, "-1e-13");
    test_is_integer(dec64_new(1, -12), zero, "1e-12");
    test_is_integer(dec64_new(-1, -12), zero, "-1e-12");
    test_is_integer(dec64_new(-1, -11), zero, "-1e-11");
    test_is_integer(dec64_new(-11, -11), zero, "-11e-11");
    test_is_integer(dec64_new(-111, -11), zero, "-111e-11");
    test_is_integer(dec64_new(-22, -11), zero, "-22e-11");
    test_is_integer(dec64_new(-1, -1), zero, "-1e-1");
    test_is_integer(dec64_new(-10, -3), zero, "-10e-3");
    test_is_integer(dec64_new(9, -1), zero, "0.9");
    test_is_integer(dec64_new(-9, -1), zero, "-0.9");
    test_is_integer(almost_one, zero, "almost_one");
    test_is_integer(almost_negative_one, zero, "almost_negative_one");
    test_is_integer(dec64_new(-999999999999999, -15), zero, "-0.9...");
    test_is_integer(dec64_new(-9999999999999998, -16), zero, "-0.9...8");
}

static void test_all_is_nan() {
    test_is_nan(nan, one, "nan");
    test_is_nan(nannan, one, "nannan");
    test_is_nan(zero, zero, "zero");
    test_is_nan(zip, zero, "zip");
    test_is_nan(one, zero, "one");
    test_is_nan(cent, zero, "cent");
    test_is_nan(maxnum, zero, "maxnum");
    test_is_nan(negative_maxnum, zero, "-maxnum");
}

static void test_all_is_zero() {
    test_is_zero(nan, zero, "nan");
    test_is_zero(nannan, zero, "nannan");
    test_is_zero(zero, one, "zero");
    test_is_zero(zip, one, "zip");
    test_is_zero(one, zero, "one");
    test_is_zero(cent, zero, "cent");
    test_is_zero(maxnum, zero, "maxnum");
    test_is_zero(negative_maxnum, zero, "-maxnum");
}

static void test_all_less() {
    test_less(nan, nan, zero, "nan < nan");
    test_less(nan, nannan, zero, "nan < nannan");
    test_less(nan, zero, zero, "nan < zero");
    test_less(nannan, nan, zero, "nannan < nan");
    test_less(nannan, nannan, zero, "nannan < nannan");
    test_less(nannan, one, zero, "nannan < 1");
    test_less(zero, nan, one, "zero < nan");
    test_less(zero, nannan, one, "0 < nannan");
    test_less(zero, zip, zero, "zero < zip");
    test_less(zero, minnum, one, "zero < minnum");
    test_less(zero, one, one, "zero < one");
    test_less(zip, zero, zero, "zip < zero");
    test_less(zip, zip, zero, "zip < zip");
    test_less(one, negative_one, zero, "1 < -1");
    test_less(two, two, zero, "2 < 2");
    test_less(two, dec64_new(2, -16), zero, "2 < 2e-16");
    test_less(three, pi, one, "3 < pi");
    test_less(pi, three, zero, "pi < 3");
    test_less(maxint, maxnum, one, "maxint < maxnum");
    test_less(negative_maxint, maxint, one, "-maxint < maxint");
    test_less(negative_maxint, negative_one, one, "-maxint < -1");
    test_less(maxnum, nan, one, "maxnum < nan");
}

static void test_all_modulo() {
    test_modulo(nan, nan, nan, "nan % nan");
    test_modulo(nan, three, nan, "nan % 3");
    test_modulo(nannan, nannan, nan, "nannan % nannan");
    test_modulo(nannan, one, nan, "nannan % 1");
    test_modulo(zero, nan, zero, "0 % nan");
    test_modulo(zero, nannan, zero, "0 % nannan");
    test_modulo(zero, zero, zero, "0 % 0");
    test_modulo(zero, zip, zero, "zero % zip");
    test_modulo(zero, one, zero, "0 % 1");
    test_modulo(zero, maxnum, zero, "0 % maxnum");
    test_modulo(zip, nan, zero, "zip % nan");
    test_modulo(zip, nannan, zero, "zip % nannan");
    test_modulo(zip, zero, zero, "zip % zero");
    test_modulo(zip, zip, zero, "zip % zip");
    test_modulo(one, negative_one, zero, "1 % -1");
    test_modulo(one, zero, nan, "1 % 0");
    test_modulo(one, one, zero, "1 % 1");
    test_modulo(one, two, one, "1 % 2");
    test_modulo(one, three, one, "1 % 3");
    test_modulo(one, maxint, one, "one % maxint");
    test_modulo(dec64_new(10, -1), maxint, dec64_new(10, -1), "one % maxint");
    test_modulo(negative_one, one, zero, "-1 % 1");
    test_modulo(two, three, two, "2 % 3");
    test_modulo(dec64_new(20000000000000000, -16), three, two, "2 % 3");
    test_modulo(two, dec64_new(30000000000000000, -16), two, "2 % 3 alias");
    test_modulo(three, three, zero, "3 % 3");
    test_modulo(pi, e, dec64_new(4233108251307480, -16), "pi % e");
    test_modulo(pi, pi, zero, "pi % pi");
    test_modulo(pi, negative_pi, zero, "pi % -pi");
    test_modulo(negative_pi, pi, zero, "-pi % pi");
    test_modulo(negative_pi, negative_pi, zero, "-pi % -pi");
    test_modulo(four, three, one, "4 % 3");
    test_modulo(dec64_new(40000000000000000, -16), dec64_new(3000000000000000, -15), one, "4 % 3");
    test_modulo(dec64_new(40000000000000000, -16), dec64_new(30000000000000000, -16), one, "4 % 3");
    test_modulo(five, three, two, "5 % 3");
    test_modulo(five, dec64_new(-30000000000000000, -16), negative_one, "5 % -3 alias");
    test_modulo(five, dec64_new(-3, 0), dec64_new(-1, 0), "5 % -3");
    test_modulo(dec64_new(-5, 0), three, one, "-5 % 3");
    test_modulo(dec64_new(-5, 0), dec64_new(-3, 0), dec64_new(-2, 0), "-5 % -3");
    test_modulo(dec64_new(-5, 0), dec64_new(-30000000000000000, -16), dec64_new(-2, 0), "-5 % -3");
    test_modulo(dec64_new(-50000000000000000, -16), three, one, "-5 % 3");
    test_modulo(dec64_new(-50000000000000000, -16), dec64_new(-30000000000000000, -16), dec64_new(-2, 0), "-5 % -3");
    test_modulo(six, nan, nan, "6 % nan");
    test_modulo(six, three, zero, "6 % 3");
    test_modulo(maxint, epsilon, zero, "maxint % epsilon");
    test_modulo(maxnum, epsilon, nan, "maxnum % epsilon");
    test_modulo(maxnum, maxnum, zero, "maxnum % maxnum");
}

static void test_all_multiply() {
    test_multiply(nan, nan, nan, "nan * nan");
    test_multiply(nan, zero, zero, "nan * zero");
    test_multiply(nannan, nannan, nan, "nannan * nannan");
    test_multiply(nannan, one, nan, "nannan * 1");
    test_multiply(zero, nan, zero, "0 * nan");
    test_multiply(zero, nannan, zero, "0 * nannan");
    test_multiply(zero, zip, zero, "zero * zip");
    test_multiply(zero, maxnum, zero, "zero * maxnum");
    test_multiply(zip, zero, zero, "zip * zero");
    test_multiply(zip, zip, zero, "zip * zip");
    test_multiply(minnum, half, minnum, "minnum * half");
    test_multiply(minnum, minnum, zero, "minnum * minnum");
    test_multiply(epsilon, epsilon, dec64_new(1, -32), "epsilon * epsilon");
    test_multiply(one, nannan, nan, "1 * nannan");
    test_multiply(negative_one, one, negative_one, "-1 * 1");
    test_multiply(negative_one, negative_one, one, "-1 * -1");
    test_multiply(two, five, ten, "2 * 5");
    test_multiply(two, maxnum, nan, "2 * maxnum");
    test_multiply(two, dec64_new(36028797018963967, 126), dec64_new(7205759403792793, 127), "2 * a big one");
    test_multiply(three, two, six, "3 * 2");
    test_multiply(ten, dec64_new(36028797018963967, 126), maxnum, "10 * a big one");
    test_multiply(ten, dec64_new(1, 127), dec64_new(10, 127), "10 * 1e127");
    test_multiply(dec64_new(1, 2), dec64_new(1, 127), dec64_new(100, 127), "1e2 * 1e127");
    test_multiply(dec64_new(1, 12), dec64_new(1, 127), dec64_new(1000000000000, 127), "1e2 * 1e127");
    test_multiply(dec64_new(1, 12), dec64_new(1, 127), dec64_new(1000000000000, 127), "1e12 * 1e127");
    test_multiply(dec64_new(3, 16), dec64_new(1, 127), dec64_new(30000000000000000, 127), "3e16 * 1e127");
    test_multiply(dec64_new(3, 17), dec64_new(1, 127), nan, "3e16 * 1e127");
    test_multiply(dec64_new(-3, 16), dec64_new(1, 127), dec64_new(-30000000000000000, 127), "3e16 * 1e127");
    test_multiply(dec64_new(-3, 17), dec64_new(1, 127), nan, "3e16 * 1e127");
    test_multiply(dec64_new(9999999999999999, 0), ten, dec64_new(9999999999999999, 1), "9999999999999999 * 10");
    test_multiply(maxint, zero, zero, "maxint * zero");
    test_multiply(maxint, epsilon, dec64_new(36028797018963967, -16), "maxint * epsilon");
    test_multiply(maxint, maxint, dec64_new(12980742146337068, 17), "maxint * maxint");
    test_multiply(negative_maxint, nan, nan, "-maxint * nan");
    test_multiply(negative_maxint, maxint, dec64_new(-12980742146337069, 17), "-maxint * maxint");
    test_multiply(maxnum, maxnum, nan, "maxnum * maxnum");
    test_multiply(maxnum, minnum, maxint, "maxnum * minnum");
}

static void test_all_neg() {
    test_neg(nan, nan, "nan");
    test_neg(nannan, nan, "nannan");
    test_neg(100, zero, "zero alias");
    test_neg(zero, zero, "zero");
    test_neg(zip, zero, "zip");
    test_neg(one, negative_one, "one");
    test_neg(negative_one, one, "-1");
    test_neg(maxint, dec64_new(-36028797018963967, 0), "maxint");
    test_neg(negative_maxint, maxint_plus, "-maxint");
    test_neg(maxnum, dec64_new(-36028797018963967, 127), "maxnum");
    test_neg(negative_maxnum, nan, "-maxnum");
}

static void test_all_new() {
    test_new(0, 0, zero, "zero");
    test_new(0, 1000, zero, "0e1000");
    test_new(0, -1000, zero, "0e-1000");
    test_new(1, 0, (1 << 8), "one");
    test_new(1, 1000, nan, "0e1000");
    test_new(1, -1000, zero, "0e-1000");
    test_new(-1, 127, (-1 << 8) + 127, "-1e127");
    test_new(-1, 128, (-10 << 8) + 127, "-1e128");
    test_new(1, -128, zero, "1e-128");
    test_new(-1, 143, (-10000000000000000LL << 8) + 127, "-1e143");
    test_new(-1, 144, nan, "-1e144");
    test_new(10, -128, minnum, "10e-128");
    test_new(100, -129, minnum, "100e-129");
    test_new(10000000000000001, -16, (10000000000000001 << 8) + (0xff & -16), "10000000000000001, -16");
    test_new(36028797018963967,  0, (36028797018963967 << 8), "3602879701896397e0");
    test_new(-36028797018963967, 0, -36028797018963967 << 8, "-3602879701896397e0");
    test_new(36028797018963967, -128, (3602879701896397 << 8) + (0xff & -127), "36028797018963967e-128");
    test_new(36028797018963967, -129, (360287970189640 << 8) + (0xff & -127), "36028797018963967e-129");
    test_new(36028797018963967, -130, (36028797018964 << 8) + (0xff & -127), "36028797018963967e-130");
    test_new(36028797018963967, -131, (3602879701896 << 8) + (0xff & -127), "36028797018963967e-131");
    test_new(36028797018963967, -132, (360287970190 << 8) + (0xff & -127), "36028797018963967e-132");
    test_new(36028797018963967, -133, (36028797019 << 8) + (0xff & -127), "36028797018963967e-133");
    test_new(36028797018963967, -134, (3602879702LL << 8) + (0xff & -127), "36028797018963967e-134");
    test_new(36028797018963967, -135, (360287970LL << 8) + (0xff & -127), "36028797018963967e-135");
    test_new(36028797018963967, -136, (36028797LL << 8) + (0xff & -127), "36028797018963967e-136");
    test_new(36028797018963967, -137, (3602880LL << 8) + (0xff & -127), "36028797018963967e-137");
    test_new(36028797018963967, -138, (360288LL << 8) + (0xff & -127), "36028797018963967e-138");
    test_new(36028797018963967, -139, (36029LL << 8) + (0xff & -127), "36028797018963967e-139");
    test_new(36028797018963967, -140, (3603LL << 8) + (0xff & -127), "36028797018963967e-140");
    test_new(36028797018963967, -141, (360LL << 8) + (0xff & -127), "36028797018963967e-141");
    test_new(36028797018963967, -142, (36LL << 8) + (0xff & -127), "36028797018963967e-142");
    test_new(36028797018963967, -143, (4LL << 8) + (0xff & -127), "36028797018963967e-143");
    test_new(36028797018963967, -144, zero, "36028797018963967e-144");
    test_new( 360287970189639670,  0, ( 36028797018963967 << 8) + 1, "36028797018963970e0");
    test_new(-360287970189639670,  0, (-36028797018963967 << 8) + 1, "-36028797018963970e0");
    test_new( 3602879701896396700, 0, ( 36028797018963967 << 8) + 2, "3602879701896396700e0");
    test_new(-3602879701896396700, 0, (-36028797018963967 << 8) + 2, "-3602879701896396700e0");
    test_new( 3602879701896396701, 0, ( 36028797018963967 << 8) + 2, "3602879701896396701e0");
    test_new(-3602879701896396701, 0, (-36028797018963967 << 8) + 2, "-3602879701896396701e0");
    test_new( 360287970189639674,  0, ( 36028797018963967 << 8) + 1, "36028797018963974e0");
    test_new(-360287970189639674,  0, (-36028797018963967 << 8) + 1, "-36028797018963974e0");
    test_new( 3602879701896396740, 0, ( 36028797018963967 << 8) + 2, "3602879701896396740e0");
    test_new(-3602879701896396740, 0, (-36028797018963967 << 8) + 2, "-3602879701896396740e0");
    test_new( 3602879701896396749, 0, ( 36028797018963967 << 8) + 2, "3602879701896396749e0");
    test_new(-3602879701896396749, 0, (-36028797018963967 << 8) + 2, "-3602879701896396749e0");
    test_new(-360287970189639675,  0, (-36028797018963968 << 8) + 1, "-36028797018963975e0");
    test_new( 360287970189639675,  0, ( 3602879701896397  << 8) + 2, "36028797018963975e0");
    test_new(-3602879701896396750, 0, (-36028797018963968 << 8) + 2, "-3602879701896396750e0");
    test_new( 3602879701896396750, 0, ( 3602879701896397  << 8) + 3, "3602879701896396750e0");
    test_new(-36028797018963968,   0, (-36028797018963968 << 8), "-3602879701896398e0");
    test_new(-36028797018963968, -147, zero, "-36028797018963968e-147");
    test_new(-3602879701896396800, 0, (-36028797018963968 << 8) + 2, "-3602879701896396800e0");
    test_new( 3602879701896396800, 0, (3602879701896397 << 8) + 3, "3602879701896396800e0");
    test_new(4611686018427387903, 0, (4611686018427388 << 8) + 3, "4611686018427387903");
    test_new(-4611686018427387903, 0, (-4611686018427388 << 8) + 3, "-4611686018427387903");
    test_new(49, -129, zero, "49e-129");
    test_new(50, -129, minnum, "50e-129");
    test_new(500000000000, -139, minnum, "500000000000e-139");
    test_new(-500000000000, -139, negative_minnum, "-500000000000e-139");
    test_new(5000000000000000000, -145, (5 << 8) + (0x81), "5000000000000000000e-145");
    test_new(-5000000000000000000, -146, negative_minnum, "-5000000000000000000e-146");
    test_new(-5555555555555555, 0, (-5555555555555555 << 8) + 0, "-55555555555555555");
    test_new(-55555555555555555, 0, (-5555555555555556 << 8) + 1, "-55555555555555555");
    test_new(-555555555555555555, 0, (-5555555555555556 << 8) + 2, "-555555555555555555");
    test_new(-5555555555555555555, 0, (-5555555555555556 << 8) + 3, "-5555555555555555555");
    test_new( 576460752303423487, 0,  (5764607523034235 << 8) + 2, "1152921504606846975");
    test_new(-576460752303423487, 0, (-5764607523034235 << 8) + 2, "-1152921504606846975");
    test_new( 72057594037927935, 0,  (7205759403792794 << 8) + 1, "72057594037927935");
    test_new(-72057594037927935, 0, (-7205759403792794 << 8) + 1, "-72057594037927935");
    test_new( 9223372036854775807, 0,  (9223372036854776 << 8) + 3, "9223372036854775807");
    test_new(-9223372036854775807, 0, (-9223372036854776 << 8) + 3, "-9223372036854775807");
    test_new(-9223372036854775807, 124, (-9223372036854776 << 8) + 127, "-9223372036854775807e124");
    test_new(-9223372036854775807, 125, nan, "-9223372036854775807e125");
    test_new(-9223372036854775807, -132, (-92233720368548 << 8) + (0x81), "-9223372036854775807e-132");
    test_new(-9223372036854775807, -133, (-9223372036855 << 8) + (0x81), "-9223372036854775807e-133");
    test_new(9223372036854775807, -143, (922LL << 8) + (0xff & -127), "9223372036854775807e-143");
    test_new(9223372036854775807, -144, (92LL << 8) + (0xff & -127), "9223372036854775807e-144");
    test_new(-9223372036854775807, -145, (-9 << 8) + (0x81), "-9223372036854775807e-145");
    test_new(9223372036854775807, -145, (9LL << 8) + (0xff & -127), "9223372036854775807e-145");
    test_new(-9223372036854775807, -146, (-1 << 8) + (0x81), "-9223372036854775807e-146");
    test_new(9223372036854775807, -146, (1LL << 8) + (0xff & -127), "9223372036854775807e-146");
}

void test_all_normal() {
    test_normal(nan, nan, "nan");
    test_normal(nannan, nan, "nannan");
    test_normal(zero, zero, "zero");
    test_normal(zip, zero, "zip");
    test_normal(epsilon, epsilon, "epsilon");
    test_normal(ten, ten, "ten");
    test_normal(dec64_new(-10000000000000000, -16), negative_one, "-1");
    test_normal(one, one, "one");
    test_normal(dec64_new(1, 1), dec64_new(10, 0), "10");
    test_normal(dec64_new(1, 2), dec64_new(100, 0), "100");
    test_normal(dec64_new(1, 3), dec64_new(1000, 0), "1000");
    test_normal(dec64_new(1, 4), dec64_new(10000, 0), "10000");
    test_normal(dec64_new(1, 5), dec64_new(100000, 0), "100000");
    test_normal(dec64_new(1, 6), dec64_new(1000000, 0), "1000000");
    test_normal(dec64_new(1, 7), dec64_new(10000000, 0), "10000000");
    test_normal(dec64_new(1, 8), dec64_new(100000000, 0), "100000000");
    test_normal(dec64_new(1, 9), dec64_new(1000000000, 0), "1000000000");
    test_normal(dec64_new(1, 10), dec64_new(10000000000, 0), "100000000000");
    test_normal(dec64_new(1, 11), dec64_new(100000000000, 0), "1000000000000");
    test_normal(dec64_new(1, 12), dec64_new(1000000000000, 0), "10000000000000");
    test_normal(dec64_new(1, 13), dec64_new(10000000000000, 0), "100000000000000");
    test_normal(dec64_new(1, 14), dec64_new(100000000000000, 0), "1000000000000000");
    test_normal(dec64_new(1, 15), dec64_new(1000000000000000, 0), "10000000000000000");
    test_normal(dec64_new(1, 16), dec64_new(10000000000000000, 0), "100000000000000000");
    test_normal(dec64_new(1, 17), dec64_new(100000000000000000, 0), "1000000000000000000");
    test_normal(cent, cent, "cent");
    test_normal(dec64_new(10, -1), one, "one alias 1");
    test_normal(dec64_new(100, -2), one, "one alias 2");
    test_normal(dec64_new(1000, -3), one, "one alias 3");
    test_normal(dec64_new(10000, -4), one, "one alias 4");
    test_normal(dec64_new(100000, -5), one, "one alias 5");
    test_normal(dec64_new(1000000, -6), one, "one alias 6");
    test_normal(dec64_new(10000000, -7), one, "one alias 7");
    test_normal(dec64_new(100000000, -8), one, "one alias 8");
    test_normal(dec64_new(1000000000, -9), one, "one alias 9");
    test_normal(dec64_new(10000000000, -10), one, "one alias 10");
    test_normal(dec64_new(100000000000, -11), one, "one alias 11");
    test_normal(dec64_new(1000000000000, -12), one, "one alias 12");
    test_normal(dec64_new(10000000000000, -13), one, "one alias 13");
    test_normal(dec64_new(100000000000000, -14), one, "one alias 14");
    test_normal(dec64_new(1000000000000000, -15), one, "one alias 15");
    test_normal(dec64_new(10000000000000000, -16), one, "one alias 16");
    test_normal(dec64_new(-12500000000000000, -16), dec64_new(-125, -2), "-1.25");
}

static void test_all_not() {
    test_not(nan, nan, "nan");
    test_not(nannan, nan, "nannan");
    test_not(zero, one, "zero");
    test_not(zip, one, "zip");
    test_not(100, one, "zero alias");
    test_not(one, zero, "one");
    test_not(negative_one, zero, "-1");
    test_not(negative_maxint, zero, "-maxint");
    test_not(negative_maxnum, zero, "-maxnum");
}

static void test_all_round() {
    test_round(nan, nan, nan, "nan");
    test_round(nannan, nannan, nan, "nannan nannan");
    test_round(nannan, 0, nan, "nannan 0");
    test_round(0, nannan, nan, "0 nannan");
    test_round(zero, zip, zero, "zero zip");
    test_round(zip, zero, zero, "zip zero");
    test_round(zip, zip, zero, "zip zip");
    test_round(pi, dec64_new(-18, 0), pi, "pi -18");
    test_round(pi, dec64_new(-17, 0), pi, "pi -17");
    test_round(pi, dec64_new(-16, 0), dec64_new(31415926535897932, -16), "pi -16");
    test_round(pi, dec64_new(-15, 0), dec64_new(3141592653589793, -15), "pi -15");
    test_round(pi, dec64_new(-14, 0), dec64_new(314159265358979, -14), "pi -14");
    test_round(pi, dec64_new(-13, 0), dec64_new(31415926535898, -13), "pi -13");
    test_round(pi, dec64_new(-12, 0), dec64_new(314159265359, -11), "pi -12");
    test_round(pi, dec64_new(-11, 0), dec64_new(314159265359, -11), "pi -11");
    test_round(pi, dec64_new(-10, 0), dec64_new(31415926536, -10), "pi -10");
    test_round(pi, dec64_new(-9, 0), dec64_new(3141592654, -9), "pi -9");
    test_round(pi, dec64_new(-8, 0), dec64_new(314159265, -8), "pi -8");
    test_round(pi, dec64_new(-7, 0), dec64_new(31415927, -7), "pi -7");
    test_round(pi, dec64_new(-6, 0), dec64_new(3141593, -6), "pi -6");
    test_round(pi, dec64_new(-5, 0), dec64_new(314159, -5), "pi -5");
    test_round(pi, dec64_new(-4, 0), dec64_new(31416, -4), "pi -4");
    test_round(pi, dec64_new(-3, 0), dec64_new(3142, -3), "pi -3");
    test_round(pi, dec64_new(-2, 0), dec64_new(314, -2), "pi -2");
    test_round(pi, negative_one, dec64_new(31, -1), "pi -1");
    test_round(pi, zero, three, "pi 0");
    test_round(negative_pi, dec64_new(-18, 0), negative_pi, "-pi -18");
    test_round(negative_pi, dec64_new(-17, 0), negative_pi, "-pi -17");
    test_round(negative_pi, dec64_new(-16, 0), dec64_new(-31415926535897932, -16), "-pi -16");
    test_round(negative_pi, dec64_new(-15, 0), dec64_new(-3141592653589793, -15), "-pi -15");
    test_round(negative_pi, dec64_new(-14, 0), dec64_new(-314159265358979, -14), "-pi -14");
    test_round(negative_pi, dec64_new(-13, 0), dec64_new(-31415926535898, -13), "-pi -13");
    test_round(negative_pi, dec64_new(-12, 0), dec64_new(-314159265359, -11), "-pi -12");
    test_round(negative_pi, dec64_new(-11, 0), dec64_new(-314159265359, -11), "-pi -11");
    test_round(negative_pi, dec64_new(-10, 0), dec64_new(-31415926536, -10), "-pi -10");
    test_round(negative_pi, dec64_new(-9, 0), dec64_new(-3141592654LL, -9), "-pi -9");
    test_round(negative_pi, dec64_new(-8, 0), dec64_new(-314159265, -8), "-pi -8");
    test_round(negative_pi, dec64_new(-7, 0), dec64_new(-31415927, -7), "-pi -7");
    test_round(negative_pi, dec64_new(-6, 0), dec64_new(-3141593, -6), "-pi -6");
    test_round(negative_pi, dec64_new(-5, 0), dec64_new(-314159, -5), "-pi -5");
    test_round(negative_pi, dec64_new(-4, 0), dec64_new(-31416, -4), "-pi -4");
    test_round(negative_pi, dec64_new(-3, 0), dec64_new(-3142, -3), "-pi -3");
    test_round(negative_pi, dec64_new(-2, 0), dec64_new(-314, -2), "-pi -2");
    test_round(negative_pi, negative_one, dec64_new(-31, -1), "-pi -1");
    test_round(negative_pi, zero, dec64_new(-3, 0), "-pi 0");
    test_round(dec64_new(449, -2), dec64_new(-2, 0), dec64_new(449, -2), "4.49 -2");
    test_round(dec64_new(449, -2), negative_one, dec64_new(45, -1), "4.49 -1");
    test_round(dec64_new(449, -2), zero, four, "4.49 0");
    test_round(dec64_new(450, -2), zero, five, "4.50 0");
    test_round(dec64_new(-449, -2), dec64_new(-2, 0), dec64_new(-449, -2), "-4.49 -2");
    test_round(dec64_new(-449, -2), negative_one, dec64_new(-45, -1), "-4.49 -1");
    test_round(dec64_new(-449, -2), zero, dec64_new(-4, 0), "-4.49 0");
    test_round(dec64_new(-450, -2), zero, dec64_new(-5, 0), "-4.50 0");
    test_round(maxint, negative_one, maxint, "maxint -1");
    test_round(maxint, zero, maxint, "maxint 0");
    test_round(maxint, one, dec64_new(3602879701896397, 1), "maxint 1");
    test_round(maxint, two, dec64_new(3602879701896400, 1), "maxint 2");
    test_round(maxint, three, dec64_new(3602879701896400, 1), "maxint 3");
    test_round(maxint, four, dec64_new(36028797018960000, 0), "maxint 4");
    test_round(maxint, five, dec64_new(36028797019000000, 0), "maxint 5");
    test_round(maxint, six, dec64_new(36028797019000000, 0), "maxint 6");
    test_round(maxint, seven, dec64_new(36028797020000000, 0), "maxint 7");
    test_round(maxint, eight, dec64_new(36028797000000000, 0), "maxint 8");
    test_round(maxint, nine, dec64_new(36028797000000000, 0), "maxint 9");
    test_round(maxint, ten, dec64_new(36028800000000000, 0), "maxint 10");
    test_round(maxint, dec64_new(11, 0), dec64_new(36028800000000000, 0), "maxint 11");
    test_round(maxint, dec64_new(12, 0), dec64_new(36029000000000000, 0), "maxint 12");
    test_round(maxint, dec64_new(13, 0), dec64_new(36030000000000000, 0), "maxint 13");
    test_round(maxint, dec64_new(14, 0), dec64_new(36000000000000000, 0), "maxint 14");
    test_round(maxint, dec64_new(15, 0), dec64_new(36000000000000000, 0), "maxint 15");
    test_round(maxint, dec64_new(16, 0), dec64_new(40000000000000000, 0), "maxint 16");
    test_round(maxint, dec64_new(17, 0), zero, "maxint 17");
}

static void test_all_signum() {
    test_signum(nan, nan, "nan");
    test_signum(nannan, nan, "nannan");
    test_signum(zero, zero, "zero");
    test_signum(zip, zero, "zip");
    test_signum(dec64_new(0, -16), zero, "zero");
    test_signum(minnum, one, "minnum");
    test_signum(epsilon, one, "epsilon");
    test_signum(cent, one, "cent");
    test_signum(half, one, "half");
    test_signum(one, one, "one");
    test_signum(negative_one, negative_one, "negative_one");
    test_signum(dec64_new(-12500000000000000, -16), negative_one, "-1.25");
    test_signum(dec64_new(-1500000000000000, -15), negative_one, "-1.5");
    test_signum(dec64_new(-1560000000000000, -15), negative_one, "-1.56");
    test_signum(dec64_new(20000000000000000, -16), one, "two");
    test_signum(e, one, "e");
    test_signum(pi, one, "pi");
    test_signum(negative_maxint, negative_one, "negative_maxint");
    test_signum(maxint, one, "maxint");
    test_signum(maxnum, one, "maxnum");
}

static void test_all_subtract() {
    test_subtract(nan, three, nan, "nan - 3");
    test_subtract(nannan, nannan, nan, "nannan - nannan");
    test_subtract(nannan, one, nan, "nannan - 1");
    test_subtract(zero, nannan, nan, "0 - nannan");
    test_subtract(zero, zip, zero, "zero - zip");
    test_subtract(zero, negative_pi, pi, "0 - -pi");
    test_subtract(zero, pi, negative_pi, "0 - pi");
    test_subtract(zero, negative_maxint, maxint_plus, "-maxint");
    test_subtract(zero, negative_maxnum, nan, "0 - -maxnum");
    test_subtract(zip, zero, zero, "zip - zero");
    test_subtract(zip, zip, zero, "zip - zip");
    test_subtract(epsilon, epsilon, zero, "epsilon - epsilon");
    test_subtract(one, negative_maxint, maxint_plus, "1  - -maxint");
    test_subtract(one, epsilon, almost_one, "1 - epsilon");
    test_subtract(one, almost_one, epsilon, "1 - almost_one");
    test_subtract(negative_one, negative_maxint, maxint, "-1  - -maxint");
    test_subtract(three, nan, nan, "3 - nan");
    test_subtract(three, dec64_new(30000000000000000, -16), zero, "equal but with different exponents");
    test_subtract(three, four, dec64_new(-1, 0), "3 - 4");
    test_subtract(negative_pi, negative_pi, zero, "-pi - 0");
    test_subtract(negative_pi, zero, negative_pi, "-pi - 0");
    test_subtract(four, dec64_new(4000000000000000, -15), zero, "equal but with different exponents");
    test_subtract(ten, six, four, "10 - 6");
    test_subtract(maxint, negative_maxint, dec64_new(7205759403792794, 1), "-maxint");
    test_subtract(maxint, maxint_plus, dec64_new(-3, 0), "maxint - (maxint + 1)");
    test_subtract(negative_maxint, negative_maxint, zero, "-maxint - -maxint");
    test_subtract(maxnum, maxint, maxnum, "maxnum - maxint");
    test_subtract(maxnum, negative_maxint, maxnum, "maxnum - -maxint");
    test_subtract(maxnum, maxnum, zero, "maxnum - maxnum");
    test_subtract(almost_negative_one, almost_negative_one, zero, "almost_negative_one - almost_negative_one");
}

static int do_tests(int level_of_detail) {
/*
    Level of detail:
        3 full
        2 errors only
        1 error summary
        0 none
*/
    level = level_of_detail;
    nr_fail = 0;
    nr_pass = 0;

    test_all_abs();
    test_all_add();
    test_all_ceiling();
    test_all_divide();
    test_all_equal();
    test_all_floor();
    test_all_integer_divide();
    test_all_is_integer();
    test_all_is_nan();
    test_all_is_zero();
    test_all_less();
    test_all_modulo();
    test_all_multiply();
    test_all_neg();
    test_all_new();
    test_all_normal();
    test_all_not();
    test_all_round();
    test_all_signum();
    test_all_subtract();

    printf("\n\n%lli pass, %lli fail.\n", nr_pass, nr_fail);
    return nr_fail;
}

int main(int argc, char* argv[]) {
    define_constants();
    return do_tests(2);
}
