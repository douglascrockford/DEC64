/* dec64.h

The dec64_math header file. This is the companion to dec64_math.c.

dec64.com
2018-01-20
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
extern dec64 dec64_root(dec64 index, dec64 radicand);
extern void  dec64_seed(uint64 part_0, uint64 part_1);
extern dec64 dec64_sin(dec64 radians);
extern dec64 dec64_sqrt(dec64 radicand);
extern dec64 dec64_tan(dec64 radians);
