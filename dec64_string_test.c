/* dec64_string_test.c

This is a test of dec64_string.c.

dec64.com
2016-02-04
Public Domain

No warranty.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dec64.h"
#include "dec64_string.h"

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
static dec64 epsilon;
static dec64 almost_one;
static dec64 almost_negative_one;
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
    epsilon = dec64_new(1, -16);    /* the smallest number addable to 1 */
    cent = dec64_new(1, -2);        /* 0.01 */
    half = dec64_new(5, -1);        /* 0.5 */
    almost_one = dec64_new(9999999999999999, -16);
    								/* 0.9999999999999999 */
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

static void test_from(dec64_string_char * string, dec64 expected) {
    dec64 actual = dec64_from_string(state, string);
    if (dec64_equal(expected, actual) == DEC64_TRUE) {
        nr_pass += 1;
        if (level >= 3) {
            printf("\n\npass from: %s", string);
            printf("\n%-4s", "=");
            print_dec64(actual);
        }
    } else {
        nr_fail += 1;
        if (level >= 1) {
            printf("\n\nFAIL from: %s", string);
            if (level >= 2) {
                printf("\n%-4s", "?");
                print_dec64(actual);
                printf("\n%-4s", "=");
                print_dec64(expected);
            }
        }
    }
}

static void test_to(dec64 number, dec64_string_char * expected) {
    dec64_string_char actual[32];
    dec64_to_string(state, number, actual);
    if (strcmp(expected, actual) == 0) {
        nr_pass += 1;
        if (level >= 3) {
            printf("\n\npass to: ");
            print_dec64(number);
            printf("\n%-4s\"%s\"", "=", expected);
        }
    } else {
        nr_fail += 1;
        if (level >= 1) {
            printf("\n\nFAIL to: ");
            print_dec64(number);
            if (level >= 2) {
                printf("\n%-4s\"%s\"", "?", actual);
                printf("\n%-4s\"%s\"", "=", expected);
            }
        }
    }
}

static void test_to_standard() {
    test_to(nan, "");
    test_to(nannan, "");
    test_to(zero, "0");
    test_to(zip, "0");
    test_to(one, "1");
    test_to(two, "2");
    test_to(three, "3");
    test_to(four, "4");
    test_to(five, "5");
    test_to(six, "6");
    test_to(seven, "7");
    test_to(eight, "8");
    test_to(nine, "9");
    test_to(ten, "10");
    test_to(maxint, "36028797018963967");
    test_to(maxint_plus, "36028797018963970");
    test_to(maxnum, "3.6028797018963967e143");
    test_to(minnum, "1e-127");
    test_to(epsilon, "0.0000000000000001");
    test_to(almost_one, "0.9999999999999999");
    test_to(almost_negative_one, "-0.9999999999999999");
    test_to(pi, "3.1415926535897932");
    test_to(half, "0.5");
    test_to(cent, "0.01");
    test_to(negative_one, "-1");
    test_to(negative_nine, "-9");
    test_to(negative_minnum, "-1e-127");
    test_to(negative_maxint, "-36028797018963968");
    test_to(negative_maxnum, "-3.6028797018963968e143");
    test_to(negative_pi, "-3.1415926535897932");
}

static void test_to_separated() {
    dec64_string_separation(state, 3);
    dec64_string_separator(state, ',');

    test_to(nan, "");
    test_to(nannan, "");
    test_to(zero, "0");
    test_to(zip, "0");
    test_to(one, "1");
    test_to(two, "2");
    test_to(three, "3");
    test_to(four, "4");
    test_to(five, "5");
    test_to(six, "6");
    test_to(seven, "7");
    test_to(eight, "8");
    test_to(nine, "9");
    test_to(ten, "10");
    test_to(maxint, "36,028,797,018,963,967");
    test_to(maxint_plus, "36,028,797,018,963,970");
    test_to(maxnum, "3.6028797018963967e143");
    test_to(minnum, "1e-127");
    test_to(epsilon, "0.0000000000000001");
    test_to(almost_one, "0.9999999999999999");
    test_to(almost_negative_one, "-0.9999999999999999");
    test_to(pi, "3.1415926535897932");
    test_to(half, "0.5");
    test_to(cent, "0.01");
    test_to(negative_one, "-1");
    test_to(negative_nine, "-9");
    test_to(negative_minnum, "-1e-127");
    test_to(negative_maxint, "-36,028,797,018,963,968");
    test_to(negative_maxnum, "-3.6028797018963968e143");
    test_to(negative_pi, "-3.1415926535897932");
    test_to(dec64_new(1, 0), "1");
    test_to(dec64_new(1, 1), "10");
    test_to(dec64_new(1, 2), "100");
    test_to(dec64_new(1, 3), "1,000");
    test_to(dec64_new(1, 4), "10,000");
    test_to(dec64_new(1, 5), "100,000");
    test_to(dec64_new(1, 6), "1,000,000");
}

static void test_to_place() {
    dec64_string_places(state, 2);

    test_to(nan, "");
    test_to(nannan, "");
    test_to(zero, "0");
    test_to(zip, "0");
    test_to(one, "1.00");
    test_to(two, "2.00");
    test_to(three, "3.00");
    test_to(four, "4.00");
    test_to(five, "5.00");
    test_to(six, "6.00");
    test_to(seven, "7.00");
    test_to(eight, "8.00");
    test_to(nine, "9.00");
    test_to(ten, "10.00");
    test_to(maxint, "36,028,797,018,963,967.00");
    test_to(maxint_plus, "36,028,797,018,963,970.00");
    test_to(maxnum, "3.6028797018963967e143");
    test_to(minnum, "1e-127");
    test_to(epsilon, "0.0000000000000001");
    test_to(almost_one, "0.9999999999999999");
    test_to(almost_negative_one, "-0.9999999999999999");
    test_to(pi, "3.1415926535897932");
    test_to(half, "0.50");
    test_to(cent, "0.01");
    test_to(negative_one, "-1.00");
    test_to(negative_nine, "-9.00");
    test_to(negative_minnum, "-1e-127");
    test_to(negative_maxint, "-36,028,797,018,963,968.00");
    test_to(negative_maxnum, "-3.6028797018963968e143");
    test_to(negative_pi, "-3.1415926535897932");
    test_to(dec64_new(1, 0), "1.00");
    test_to(dec64_new(1, 1), "10.00");
    test_to(dec64_new(1, 2), "100.00");
    test_to(dec64_new(1, 3), "1,000.00");
    test_to(dec64_new(1, 4), "10,000.00");
    test_to(dec64_new(1, 5), "100,000.00");
    test_to(dec64_new(1, 6), "1,000,000.00");
    test_to(dec64_new(10000, 0), "10,000.00");
    test_to(dec64_new(10000, -1), "1,000.00");
    test_to(dec64_new(10000, -2), "100.00");
    test_to(dec64_new(10000, -3), "10.00");
    test_to(dec64_new(10000, -4), "1.00");
    test_to(dec64_new(10000, -5), "0.10");
    test_to(dec64_new(10000, -6), "0.01");
    test_to(dec64_new(10000, -7), "0.001");
    test_to(dec64_new(12345, 0), "12,345.00");
    test_to(dec64_new(12345, -1), "1,234.50");
    test_to(dec64_new(12345, -2), "123.45");
    test_to(dec64_new(12345, -3), "12.345");
    test_to(dec64_new(12345, -4), "1.2345");
    test_to(dec64_new(12345, -5), "0.12345");
    test_to(dec64_new(12345, -6), "0.012345");
    test_to(dec64_new(12345, -7), "0.0012345");
    test_to(dec64_new(10, -128), "1e-127");
}

static void test_to_scientific() {
    dec64_string_scientific(state);

    test_to(nan, "");
    test_to(nannan, "");
    test_to(zero, "0");
    test_to(zip, "0");
    test_to(one, "1");
    test_to(two, "2");
    test_to(three, "3");
    test_to(four, "4");
    test_to(five, "5");
    test_to(six, "6");
    test_to(seven, "7");
    test_to(eight, "8");
    test_to(nine, "9");
    test_to(ten, "1e1");
    test_to(maxint, "3.6028797018963967e16");
    test_to(maxint_plus, "3.602879701896397e16");
    test_to(maxnum, "3.6028797018963967e143");
    test_to(minnum, "1e-127");
    test_to(epsilon, "1e-16");
    test_to(almost_one, "9.999999999999999e-1");
    test_to(almost_negative_one, "-9.999999999999999e-1");
    test_to(pi, "3.1415926535897932");
    test_to(half, "5e-1");
    test_to(cent, "1e-2");
    test_to(negative_one, "-1");
    test_to(negative_nine, "-9");
    test_to(negative_minnum, "-1e-127");
    test_to(negative_maxint, "-3.6028797018963968e16");
    test_to(negative_maxnum, "-3.6028797018963968e143");
    test_to(negative_pi, "-3.1415926535897932");
    test_to(dec64_new(1, 0), "1");
    test_to(dec64_new(1, 1), "1e1");
    test_to(dec64_new(1, 2), "1e2");
    test_to(dec64_new(1, 3), "1e3");
    test_to(dec64_new(1, 4), "1e4");
    test_to(dec64_new(1, 5), "1e5");
    test_to(dec64_new(1, 6), "1e6");
    test_to(dec64_new(10000, 0), "1e4");
    test_to(dec64_new(10000, -1), "1e3");
    test_to(dec64_new(10000, -2), "1e2");
    test_to(dec64_new(10000, -3), "1e1");
    test_to(dec64_new(10000, -4), "1");
    test_to(dec64_new(10000, -5), "1e-1");
    test_to(dec64_new(10000, -6), "1e-2");
    test_to(dec64_new(10000, -7), "1e-3");
    test_to(dec64_new(12345, 110), "1.2345e114");
    test_to(dec64_new(12345, 100), "1.2345e104");
    test_to(dec64_new(12345, 90), "1.2345e94");
    test_to(dec64_new(12345, 10), "1.2345e14");
    test_to(dec64_new(12345, 0), "1.2345e4");
    test_to(dec64_new(12345, -1), "1.2345e3");
    test_to(dec64_new(12345, -2), "1.2345e2");
    test_to(dec64_new(12345, -3), "1.2345e1");
    test_to(dec64_new(12345, -4), "1.2345");
    test_to(dec64_new(12345, -5), "1.2345e-1");
    test_to(dec64_new(12345, -6), "1.2345e-2");
    test_to(dec64_new(12345, -7), "1.2345e-3");
}

static void test_to_engineering() {
    dec64_string_engineering(state);

    test_to(nan, "");
    test_to(nannan, "");
    test_to(zero, "0");
    test_to(zip, "0");
    test_to(one, "1");
    test_to(two, "2");
    test_to(three, "3");
    test_to(four, "4");
    test_to(five, "5");
    test_to(six, "6");
    test_to(seven, "7");
    test_to(eight, "8");
    test_to(nine, "9");
    test_to(ten, "10");
    test_to(maxint, "36.028797018963967e15");
    test_to(maxint_plus, "36.02879701896397e15");
    test_to(maxnum, "360.28797018963967e141");
    test_to(minnum, "100e-129");
    test_to(epsilon, "100e-18");
    test_to(almost_one, "999.9999999999999e-3");
    test_to(almost_negative_one, "-999.9999999999999e-3");
    test_to(pi, "3.1415926535897932");
    test_to(half, "500e-3");
    test_to(cent, "10e-3");
    test_to(negative_one, "-1");
    test_to(negative_nine, "-9");
    test_to(negative_minnum, "-100e-129");
    test_to(negative_maxint, "-36.028797018963968e15");
    test_to(negative_maxnum, "-360.28797018963968e141");
    test_to(negative_pi, "-3.1415926535897932");
    test_to(dec64_new(1, 0), "1");
    test_to(dec64_new(1, 1), "10");
    test_to(dec64_new(1, 2), "100");
    test_to(dec64_new(1, 3), "1e3");
    test_to(dec64_new(1, 4), "10e3");
    test_to(dec64_new(1, 5), "100e3");
    test_to(dec64_new(1, 6), "1e6");
    test_to(dec64_new(10000, 0), "10e3");
    test_to(dec64_new(10000, -1), "1e3");
    test_to(dec64_new(10000, -2), "100");
    test_to(dec64_new(10000, -3), "10");
    test_to(dec64_new(10000, -4), "1");
    test_to(dec64_new(10000, -5), "100e-3");
    test_to(dec64_new(10000, -6), "10e-3");
    test_to(dec64_new(10000, -7), "1e-3");
    test_to(dec64_new(12345, 0), "12.345e3");
    test_to(dec64_new(12345, -1), "1.2345e3");
    test_to(dec64_new(12345, -2), "123.45");
    test_to(dec64_new(12345, -3), "12.345");
    test_to(dec64_new(12345, -4), "1.2345");
    test_to(dec64_new(12345, -5), "123.45e-3");
    test_to(dec64_new(12345, -6), "12.345e-3");
    test_to(dec64_new(12345, -7), "1.2345e-3");
    test_to(dec64_new(1024, 0), "1.024e3");
}

static void test_all_from() {
    test_from("", nan);
    test_from("nan", nan);
    test_from("1.1.1", nan);
    test_from("1e1.1", nan);
    test_from("1e1e1", nan);
    test_from("1h1e1", nan);
    test_from("1e", nan);
    test_from("1e+", nan);
    test_from("1e+999", nan);
    test_from(".", nan);
    test_from("1e,1", nan);
    test_from(",-0", nan);

    test_from("0", zero);
    test_from("-0", zero);
    test_from("0.", zero);
    test_from("00", zero);
    test_from("0000000000000000000000000000000000000000000000000", zero);
    test_from("000000.0000000000000000000000000000000000000000000", zero);
    test_from("00,0000.00000000000000000000000000000000000000000000000000000000e-128", zero);
    test_from(".00", zero);
    test_from("0.00", zero);
    test_from("0.00e-999", zero);

    test_from("1", one);
    test_from("01", one);
    test_from("1.00", one);
    test_from("1e+00", one);
    test_from(".100e00000000001", one);
    test_from("1.00e0000000000", one);
    test_from("10.00e-00000000001", one);


    test_from("2", two);
    test_from("3", three);
    test_from("4", four);
    test_from("5", five);
    test_from("6", six);
    test_from("7,,,,,,,,,", seven);
    test_from(",,,,,,,,,,,8", eight);
    test_from("9", nine);
    test_from("10", ten);
    test_from("10.0", ten);
    test_from("10.00e-0000000000", ten);
    test_from("1e1", ten);

    test_from("36028797018963967", maxint);
    test_from("36,028,797,018,963,967.00", maxint);
    test_from("36.028797018963967e15", maxint);
    test_from("36028797018963970", maxint_plus);
    test_from("36.02879701896397e15", maxint_plus);
    test_from("36,028,797,018,963,970.000,000", maxint_plus);
    test_from("360.28797018963967e141", maxnum);
    test_from("3.6028797018963967e143", maxnum);
    test_from("36028797018963967e127", maxnum);
    test_from("1e-127", minnum);
    test_from("100e-129", minnum);
    test_from("0.0000000000000001", epsilon);
    test_from("0.000000,00000000010000000000000000000000000", epsilon);
    test_from("1e-16", epsilon);
    test_from("100e-18", epsilon);

    test_from("999.9999999999999e-3", almost_one);
    test_from("9.999999999999999e-1", almost_one);
    test_from(".9999999999999999", almost_one);
    test_from("0.999999999999999900000000000000", almost_one);
    test_from("0.9999999999999999", almost_one);
    test_from("-999.9999999999999e-3", almost_negative_one);
    test_from("-9.999999999999999e-1", almost_negative_one);
    test_from("-.9999999999999999", almost_negative_one);
    test_from("-0.9999999999999999", almost_negative_one);
    test_from("-000000000000000000000000.999999999999999900000000000000000", almost_negative_one);
    test_from("3.1415926535897932", pi);
    test_from("31415926535897932E-16", pi);
    test_from("-3.1415926535897932", negative_pi);
    test_from("-31415926535897932E-16", negative_pi);
    test_from("0.5", half);
    test_from("0.50", half);
    test_from("5e-1", half);
    test_from("500e-3", half);
    test_from("10e-3", cent);
    test_from("1e-2", cent);
    test_from("0.01", cent);
    test_from("-1", negative_one);
    test_from("-1.00", negative_one);
    test_from("-9E-0000000000000000000000", negative_nine);
    test_from("-9.00", negative_nine);
    test_from("-1e-127", negative_minnum);
    test_from("-100e-129", negative_minnum);
    test_from("-36028797018963968", negative_maxint);
    test_from("-36,028,797,018,963,968.00", negative_maxint);
    test_from("-3.6028797018963968e16", negative_maxint);
    test_from("-36.028797018963968e15", negative_maxint);
    test_from("-36028797018963968e127", negative_maxnum);
    test_from("-3.6028797018963968e143", negative_maxnum);
    test_from("-360.28797018963968e141", negative_maxnum);

    test_from("100", dec64_new(1, 2));
    test_from("100.00", dec64_new(1, 2));
    test_from("1e2", dec64_new(1, 2));
    test_from("1,000", dec64_new(1, 3));
    test_from("1,000.00", dec64_new(1, 3));
    test_from("1e3", dec64_new(1, 3));
    test_from("10,000", dec64_new(1, 4));
    test_from("10,000.00", dec64_new(1, 4));
    test_from("1e4", dec64_new(1, 4));
    test_from("10e3", dec64_new(1, 4));
    test_from(",100,000", dec64_new(1, 5));
    test_from("0100,000.00", dec64_new(1, 5));
    test_from("01e5", dec64_new(1, 5));
    test_from("100e3", dec64_new(1, 5));
    test_from("1,000,000", dec64_new(1, 6));
    test_from("1,000,000.00", dec64_new(1, 6));
    test_from("1e6", dec64_new(1, 6));

    test_from("1.2345e114", dec64_new(12345, 110));
    test_from("1.2345e104", dec64_new(12345, 100));
    test_from("1.2345e94", dec64_new(12345, 90));
    test_from("1.2345e14", dec64_new(12345, 10));
    test_from("1.2345e4", dec64_new(12345, 0));
    test_from("12,345.00", dec64_new(12345, 0));
    test_from("12.345e3", dec64_new(12345, 0));
    test_from("1.2345e3", dec64_new(12345, -1));
    test_from("1,234.50", dec64_new(12345, -1));
    test_from("1.2345e3", dec64_new(12345, -1));
    test_from("1.2345e2", dec64_new(12345, -2));
    test_from("123.45", dec64_new(12345, -2));
    test_from("12.345", dec64_new(12345, -3));
    test_from("1.2345e1", dec64_new(12345, -3));
    test_from("1.2345", dec64_new(12345, -4));
    test_from("0.12345", dec64_new(12345, -5));
    test_from("1.2345e-1", dec64_new(12345, -5));
    test_from("123.45e-3", dec64_new(12345, -5));
    test_from("0.012345", dec64_new(12345, -6));
    test_from("1.2345e-2", dec64_new(12345, -6));
    test_from("12.345e-3", dec64_new(12345, -6));
    test_from("0.0012345", dec64_new(12345, -7));
    test_from("1.2345e-3", dec64_new(12345, -7));
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

    test_to_standard();
    test_to_separated();
    test_to_place();
    test_to_scientific();
    test_to_engineering();

    test_all_from();

    printf("\n\n%i pass, %i fail.\n", nr_pass, nr_fail);
    dec64_string_end(state);
    return nr_fail;
}

int main(int argc, char* argv[]) {
    define_constants();
    return do_tests(2);
}
