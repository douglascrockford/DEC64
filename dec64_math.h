/* dec64.h

The dec64_math header file. This is the companion to dec64_math.c.

dec64.com
2017-01-22
Public Domain

No warranty.
*/

extern dec64 dec64_acos(dec64 slope);
extern dec64 dec64_asin(dec64 slope);
extern dec64 dec64_atan(dec64 slope);
extern dec64 dec64_atan2(dec64 y, dec64 x);
extern dec64 dec64_cos(dec64 radians);
extern dec64 dec64_exp(dec64 exponent);
extern dec64 dec64_factorial(dec64 x);
extern dec64 dec64_log(dec64 x);
extern dec64 dec64_raise(dec64 coefficient, dec64 exponent);
extern dec64 dec64_random();
extern void  dec64_seed(dec64 seed);
extern dec64 dec64_sin(dec64 radians);
extern dec64 dec64_root(dec64 degree, dec64 radicand);
extern dec64 dec64_sqrt(dec64 radicand);
extern dec64 dec64_tan(dec64 radians);
