/* dec64_math_test.c

This is a test of dec64_math.c.

dec64.com
2017-01-22
Public Domain

No warranty.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dec64.h"
#include "dec64_string.h"
#include "dec64_math.h"

static int level;
static int nr_fail;
static int nr_pass;
static dec64_string_state state;

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
static dec64 e;
static dec64 epsilon;
static dec64 almost_one;
static dec64 almost_negative_one;
static dec64 pi;
static dec64 half_pi;
static dec64 half;
static dec64 cent;
static dec64 negative_one;
static dec64 negative_nine;
static dec64 negative_minnum;
static dec64 negative_maxint;
static dec64 negative_maxnum;
static dec64 negative_pi;

static void define_constants() {
    nan = DEC64_NAN;                /* not a number */
    nannan = 32896;                 /* a non-normal nan */
    zero = DEC64_ZERO;              /* 0 */
    zip = 1;                        /* a non normal 0 */
    one = DEC64_ONE;                /* 1 */
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
    e = dec64_new(27182818284590452, -16);                  /* e */
    epsilon = dec64_new(1, -16);    /* the smallest number addable to 1 */
    cent = dec64_new(1, -2);        /* 0.01 */
    half = dec64_new(5, -1);        /* 0.5 */
    almost_one = dec64_new(9999999999999999, -16);
                                    /* 0.9999999999999999 */
    pi = dec64_new(31415926535897932, -16);
                                    /* pi */
    half_pi = dec64_new(15707963267948966, -16);
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
    dec64_string_char actual[32];
    if (dec64_is_any_nan(number) == DEC64_TRUE) {
        if (number == DEC64_TRUE) {
            printf("true");
        } else if (number == DEC64_FALSE) {
            printf("false");
        } else {
            printf("nan");
        }
    } else {
        dec64_to_string(state, number, actual);
        printf("%s", actual);
    }
}

void p(dec64 number, dec64_string_char name[]) {
    if (name) {
        printf("\n[%s ", name);
    } else {
        printf("\n[");
    }
    print_dec64(number);
    printf("]");
}

static void judge_unary(
    dec64 first,
    dec64 expected,
    dec64 actual,
    char * name,
    char * op,
    char * comment
) {
    if (dec64_equal(expected, actual) == DEC64_TRUE) {
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
                printf("  (error ");
                print_dec64(dec64_subtract(actual, expected));
                printf(")\n%-4s", "=");
                print_dec64(expected);
            }
        }
    }
}

static void judge_binary(
    dec64 first,
    dec64 second,
    dec64 expected,
    dec64 actual,
    char * name,
    char * op,
    char * comment
) {
    if (dec64_equal(expected, actual) == DEC64_TRUE) {
        nr_pass += 1;
        if (level >= 3) {
            printf("\n\npass %s: %s\n    ", name, comment);
            print_dec64(first);
            printf("\n%-4s", op);
            print_dec64(second);
            printf("\n=   ");
            print_dec64(actual);
        }
    } else {
        nr_fail += 1;
        if (level >= 1) {
            printf("\n\nFAIL %s: %s", name, comment);
            if (level >= 2) {
                printf("\n    ");
                print_dec64(first);
                printf("\n%-4s", op);
                print_dec64(second);
                printf("\n%-4s", "?");
                print_dec64(actual);
                printf("  (error ");
                print_dec64(dec64_subtract(actual, expected));
                printf(")\n=   ");
                print_dec64(expected);
            }
        }
    }
}

static void test_acos(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_acos(first);
    judge_unary(first, expected, actual, "acos", "ac", comment);
}

static void test_asin(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_asin(first);
    judge_unary(first, expected, actual, "asin", "as", comment);
}

static void test_atan(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_atan(first);
    judge_unary(first, expected, actual, "atan", "at", comment);
}

static void test_cos(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_cos(first);
    judge_unary(first, expected, actual, "cos", "c", comment);
}

static void test_exp(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_exp(first);
    judge_unary(first, expected, actual, "exp", "e", comment);
}

static void test_factorial(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_factorial(first);
    judge_unary(first, expected, actual, "fac", "!", comment);
}

static void test_log(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_log(first);
    judge_unary(first, expected, actual, "log", "ln", comment);
}

static void test_raise(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_raise(first, second);
    judge_binary(first, second, expected, actual, "raise", "^", comment);
}

static void test_root(dec64 first, dec64 second, dec64 expected, char * comment) {
    dec64 actual = dec64_root(first, second);
    judge_binary(first, second, expected, actual, "root", "|", comment);
}

static void test_sin(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_sin(first);
    judge_unary(first, expected, actual, "sin", "s", comment);
}

static void test_sqrt(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_sqrt(first);
    judge_unary(first, expected, actual, "sqrt", "s", comment);
}

static void test_tan(dec64 first, dec64 expected, char * comment) {
    dec64 actual = dec64_tan(first);
    judge_unary(first, expected, actual, "tan", "t", comment);
}

static void test_all_acos() {
    test_acos(negative_one, pi, "-1");
    test_acos(zero, half_pi, "0");
    test_acos(epsilon, dec64_new(15707963267948965, -16), "epsilon");
    test_acos(cent, dec64_new(15607961601207295, -16), "0.01");
    test_acos(half, dec64_new(10471975511965977, -16), "0.5");
    test_acos(one, zero, "1");
    test_acos(half_pi, nan, "pi/2");
}

static void test_all_asin() {
    test_asin(negative_one, dec64_neg(half_pi), "-1");
    test_asin(zero, zero, "0");
    test_asin(epsilon, epsilon, "epsilon");
    test_asin(cent, dec64_new(10000166674167113, -18), "0.01");
    test_asin(half, dec64_new(5235987755982989, -16), "0.5");
    test_asin(one, half_pi, "1");
    test_asin(half_pi, nan, "pi/2");
}

static void test_all_atan() {
    test_atan(DEC64_NEGATIVE_ONE, dec64_new(-7853981633974483, -16), "-1");
    test_atan(zero, zero, "0");
    test_atan(cent, dec64_new(9999666686665238, -18), "1/100");
    test_atan(half, dec64_new(4636476090008061, -16), "1/2");
    test_atan(one, dec64_new(7853981633974483, -16), "1");
    test_atan(half_pi, dec64_new(10038848218538872, -16), "pi/2");
    test_atan(e, dec64_new(12182829050172776, -16), "e");
    test_atan(pi, dec64_new(12626272556789117, -16), "pi");
    test_atan(ten, dec64_new(14711276743037346, -16), "10");
}

static void test_all_cos() {
    test_cos(zero, one, "0");
    test_cos(cent, dec64_new(99995000041666528, -17), "0.01");
    test_cos(pi, negative_one, "pi");
    test_cos(half_pi, zero, "pi");
    test_cos(ten, dec64_new(-8390715290764525, -16), "10");
}

static void test_all_exp() {
    test_exp(zero, one, "0");
    test_exp(cent, dec64_new(10100501670841681, -16), "0.01");
    test_exp(half, dec64_new(16487212707001281, -16), "0.5");
    test_exp(one, e, "1");
    test_exp(two,  dec64_new(7389056098930650, -15), "2");
    test_exp(ten,  dec64_new(22026465794806717, -12), "10");
}

static void test_all_factorial() {
    test_factorial(zero, one, "0!");
    test_factorial(one, one, "1!");
    test_factorial(dec64_new(18, 0), dec64_new(6402373705728000, 0), "18!");
    test_factorial(dec64_new(19, 0), dec64_new(121645100408832000, 0), "19!");
    test_factorial(dec64_new(20, 0), dec64_new(2432902008176640000, 0), "20!");
    test_factorial(dec64_new(21, 0), dec64_new(5109094217170944000, 1), "21!");
    test_factorial(dec64_new(22, 0), dec64_new(11240007277776077, 5), "22!");
    test_factorial(dec64_new(92, 0), dec64_new(12438414054641307, 126), "92!");
    test_factorial(dec64_new(93, 0), nan, "93!");
    test_factorial(nan, nan, "nan!");
    test_factorial(pi, nan, "pi!");
    test_factorial(negative_one, nan, "-1!");
}

static void test_all_log() {
    test_log(zero, nan, "0");
    test_log(cent, dec64_new(-4605170185988091, -16), "0.01");
    test_log(half, dec64_new(-6931471805599453, -16), "1/2");
    test_log(one, zero, "1");
    test_log(half_pi, dec64_new(4515827052894549, -16), "pi/2");
    test_log(two, dec64_new(6931471805599453, -16), "2");
    test_log(e, one, "e");
    test_log(pi, dec64_new(11447298858494002, -16), "pi");
    test_log(ten, dec64_new(23025850929940457, -16), "10");
}

static void test_all_raise() {
    test_raise(e, zero, one, "e^0");
    test_raise(e, cent, dec64_new(10100501670841681, -16), "e^0.01");
    test_raise(e, half, dec64_new(16487212707001281, -16), "e^0.5");
    test_raise(e, one, e, "e^1");
    test_raise(e, two,  dec64_new(7389056098930650, -15), "e^2");
    test_raise(e, ten,  dec64_new(22026465794806717, -12), "e^10");
    test_raise(four, half,  two, "4^0.5");
    test_raise(two, ten,  dec64_new(1024, 0), "2^10");
}

static void test_all_root() {
    test_root(two, zero, zero, "2|zero");
    test_root(three, zero, zero, "3|zero");
    test_root(three, half, dec64_new(7937005259840997, -16), "3|1/2");
    test_root(three, dec64_new(27, 0), three, "3|27");
    test_root(three, dec64_new(-27, 0), dec64_new(-3, 0), "3|-27");
    test_root(three, pi, dec64_new(14645918875615233, -16), "3|pi");
    test_root(four, dec64_new(-27, 0), nan, "4|-27");
    test_root(four, dec64_new(256, 0), four, "4|256");
    test_root(four, dec64_new(1, 4), ten, "4|1e4");
    test_root(four, dec64_new(1, 16), dec64_new(1, 4), "4|1e16");
    test_root(four, pi, dec64_new(13313353638003897, -16), "4|pi");
}

static void test_all_sin() {
    test_sin(zero, zero, "0");
    test_sin(epsilon, epsilon, "epsilon");
    test_sin(cent, dec64_new(9999833334166665, -18), "0.01");
    test_sin(one, dec64_new(8414709848078965, -16), "1");
    test_sin(half_pi, one, "pi/2");
    test_sin(two, dec64_new(9092974268256817, -16), "2");
    test_sin(e, dec64_new(4107812905029087, -16), "e");
    test_sin(three, dec64_new(14112000805986722, -17), "3");
    test_sin(pi, zero, "pi");
    test_sin(four, dec64_new(-7568024953079283, -16), "4");
    test_sin(five, dec64_new(-9589242746631385, -16), "5");
    test_sin(ten, dec64_new(-5440211108893698, -16), "10");
    test_sin(dec64_new(-1, 0), dec64_new(-8414709848078965, -16), "-1");
}

static void test_all_sqrt() {
    test_sqrt(zero, zero, "0");
    test_sqrt(half, dec64_new(7071067811865475, -16), "1/2");
    test_sqrt(two, dec64_new(14142135623730950, -16), "2");
    test_sqrt(one, one, "1");
    test_sqrt(pi, dec64_new(17724538509055160, -16), "pi");
    test_sqrt(dec64_new(10, 0), dec64_new(31622776601683793, -16), "10");
    test_sqrt(dec64_new(16, 0), dec64_new(4, 0), "16");
    test_sqrt(dec64_new(100, 0), dec64_new(10, 0), "100");
    test_sqrt(dec64_new(10000, -2), dec64_new(10, 0), "100");
    test_sqrt(dec64_new(1000000, -4), dec64_new(10, 0), "100");
}

static void test_all_tan() {
    test_tan(zero, zero, "0");
    test_tan(cent, dec64_new(10000333346667206, -18), "0.01");
    test_tan(half, dec64_new(5463024898437905, -16), "1/2");
    test_tan(one, dec64_new(15574077246549022, -16), "1");
    test_tan(half_pi, nan, "pi/2");
    test_tan(pi, zero, "pi");
    test_tan(ten, dec64_new(6483608274590867, -16), "10");
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
    state = dec64_string_begin();

    test_all_acos();
    test_all_asin();
    test_all_atan();
    test_all_cos();
    test_all_exp();
    test_all_factorial();
    test_all_log();
    test_all_raise();
    test_all_root();
    test_all_sin();
    test_all_sqrt();
    test_all_tan();

    printf("\n\n%i pass, %i fail.\n", nr_pass, nr_fail);
    dec64_string_end(state);
    return nr_fail;
}

int main(int argc, char* argv[]) {
    define_constants();
    return do_tests(2);
}
