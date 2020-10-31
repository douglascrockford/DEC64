/* dec64.c

The DEC64 imlementation.
Ported from the original dec64.asm (created by Douglas Crockford) with a few tweaks.

Copyright (c) 2020 by vpisarev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "dec64.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

#undef abs
#undef min
#undef max
#define abs(x) ((x) < 0 ? -(x) : (x))
#define min(x, y) ((x) <= (y) ? (x) : (y))
#define max(x, y) ((x) >= (y) ? (x) : (y))

static const int64 power[] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000,
    10000000000,
    100000000000,
    1000000000000,
    10000000000000,
    100000000000000,
    1000000000000000,
    10000000000000000,
    100000000000000000,
    1000000000000000000,
    (int64)10000000000000000000ULL,
    0
};

dec64 dec64_is_false(dec64 f)
{
    return f == DEC64_FALSE ? DEC64_TRUE : DEC64_FALSE;
}

// The new/pack function will combine the coefficient and exponent into a dec64.
// Numbers that are too huge to be contained in this format become nan.
// Numbers that are too tiny to be contained in this format become zero.
dec64 dec64_new(int64 x, int e)
{
    //static int total_runs = 0;
    //static int dec64_new_adj_iters = 0;
    //++total_runs;
    //printf("\n==> dec64_new_adj_iters=%d out of %d runs\n", dec64_new_adj_iters, total_runs);
    if((x == 0) | (e <= -148)) return 0;
    while (e <= 127) {
        // If the exponent is too small, or if the coefficient is too large, then some
        // division is necessary. The "absolute" value of the coefficient (x_abs) is off by one
        // for the negative because
        //    negative_extreme_coefficent = -(extreme_coefficent + 1)
        int64 signmask = x >> 63; // signmask is -1 if x negative, or 0 if positive
        uint64 x_abs = (uint64)(x ^ signmask);
        if((uint64)x_abs >= 3602879701896396800ULL) {
            // pack large ...            
            x = (x ^ signmask) - signmask;
            x = (int64)((uint64)((unsigned __int128)x * (uint64)-3689348814741910323 >> 64) >> 3);
            x = (x ^ signmask) - signmask;
            //printf("\n==> dec64_new_adj_iters=%d out of %d runs\n", ++dec64_new_adj_iters, total_runs);
            e++;
        } else {
            int deficit = (x_abs > 36028797018963967ULL) + (x_abs > 360287970189639679);
            deficit = max(-127 - e, deficit);
            if (deficit == 0)
                // this is the "hot path":
                //   1. enter dec64_new
                //   2. enter while loop since e <= 127 most of the time
                //   3. pass quite liberal mantissa "smallness" check
                //   4. pass few more checks to ensure that deficit == 0
                //   5. and here we are, just pack the result and go.
                // that is, while dec64_new() looks heavy, most of the time
                // it finishes fast without a single multiplication or division,
                // just a few well predictable branches,
                // simple arithmetic and bit operations.
                return x != 0 ? (x << 8) | (e & 255) : 0;
            // pack increase
            if (deficit >= 20) return 0; // underflow
            dec64 scale = power[deficit];
            x = x > 0 ? (x + (scale>>1))/scale : (x - (scale>>1))/scale;
            e += deficit;
        }    
    }

    if (e >= 148) return DEC64_NULL;

    // If the exponent is too big (greater than 127).
    // We can attempt to reduce it by scaling back.
    // This can salvage values in a small set of cases.
    int64 abs_x = abs(x);
    int lsb = abs_x == 0 ? 63 : __builtin_clzll(abs_x) - 1;
    int log10_scale = lsb > 0 ? (lsb*77)>>8 : 0;
    log10_scale = min(e - 127, log10_scale);
    int64 x_scaled;
    if (!__builtin_mul_overflow(x, power[log10_scale], &x_scaled)) {
        x = x_scaled;
        e -= log10_scale;
    } /*else {
        // in theory, we should not get here
        assert(0);
    }*/

    for (; e > 127; e--) {
        // try multiplying the coefficient by 10
        // if it overflows, we failed to salvage
        if (__builtin_mul_overflow(x, 10LL, &x))
            return DEC64_NULL;
        //printf("\n==> dec64_new_adj_iters=%d out of %d runs\n", ++dec64_new_adj_iters, total_runs);
        // decrement the exponent and repeat if necessary
    }
    
    // check for overflow
    if((uint64)((x >> 55)+1) > 1)
        return DEC64_NULL;
    
    return x != 0 ? (x << 8) | (e & 255) : 0;
}

int64 dec64_coefficient(dec64 x)
{
    return x >> 8;
}

int64 dec64_exponent(dec64 x)
{
    return (signed char)x;
}

dec64 dec64_is_equal(dec64 x, dec64 y)
{
    int ex = (signed char)x, ey = (signed char)y; 
    int ediff = ex - ey;
    if (ediff == 0)
        return ((ex == -128) | (x == y)) ? DEC64_TRUE : DEC64_FALSE;

    if ((x ^ y) < 0) return DEC64_FALSE;
    
    // Let's do exact comparison instead of relying on dec64_subtract() rounding logic.
    // This is also faster, because we do not need to pack the final result, and
    // we don't have to fix possible overflow; instead we use the fact of
    // overflow to claim inequality
    
    // If user wants to check whether x ~ y,
    // he/she can use the slower dec64_is_zero(dec64_subtract(x, y))

    // Before comparison can take place, the exponents must be made to match.
    // Swap the numbers if the second exponent is greater than the first.
    int do_swap = ediff < 0;
    dec64 x1 = do_swap ? y : x;
    dec64 y1 = do_swap ? x : y;
    ediff = abs(ediff);

    // if one of the arguments is nan or the difference between
    // exponents is very big, they are not equal
    if ((ediff > 17) | (ex == -128) | (ey == -128)) return DEC64_FALSE;
    
    dec64 x_scaled, p = power[ediff];

    // try to bring e0 -> e1 by scaling up x.
    // before scaling x's exponent bits are cleared not to
    // affect the coefficient bits.
    // If we get overflow, it means that x cannot be represented
    // with the same exponent as y, which means that x != y
    if (__builtin_mul_overflow(x1 & ~255, p, &x_scaled))
        return DEC64_FALSE;

    return x_scaled == (y1 & ~255) ? DEC64_TRUE : DEC64_FALSE;
}

dec64 dec64_eps_equal(dec64 x, dec64 y, dec64 eps) {
    if (dec64_is_equal(x, y) == DEC64_TRUE)
        return DEC64_TRUE;
    return dec64_is_less(dec64_abs(dec64_subtract(x, y)), eps);
}

dec64 dec64_is_integer(dec64 x)
{
    // If the number contains a non-zero fractional part or if it is nan,
    // return false. Otherwise, return true.
    int e = (signed char)x;
    x >>= 8;
    // a positive exponent means an integer,
    // zero is an integer and
    // nan is not an integer
    if (e >= 0 || (x == 0 && e != -128))
        return DEC64_TRUE;
    // huge negative exponents can never be int,
    // this check handles nan too.
    if (e < -17) return DEC64_FALSE;
    return x % power[-e] == 0 ? DEC64_TRUE : DEC64_FALSE;
}

dec64 dec64_is_less(dec64 x, dec64 y)
{
    // Compare two dec64 numbers. If the first is less than the second, return true,
    // otherwise return false. Any nan value is greater than any number value

    // The other 3 comparison functions are easily implemented with dec64_is_less:

    //    dec64_is_greater(a, b)          => dec64_is_less(b, a)
    //    dec64_is_greater_or_equal(a, b) => dec64_is_false(dec64_is_less(a, b))
    //    dec64_is_less_or_equal(a, b)    => dec64_is_false(dec64_is_less(b, a))

    // If the exponents are the same, then do a simple compare.
    int ex = (signed char)x, ey = (signed char)y;
    if (ex == ey)
        return ((ex != -128) & (x < y)) ? DEC64_TRUE : DEC64_FALSE;
    
    // The exponents are not the same.
    if (ex == -128)
        return DEC64_FALSE;
    if (ey == -128)
        return DEC64_TRUE;
    
    int ediff = ex - ey;
    x >>= 8;
    y >>= 8;
    if (ediff > 0) {
        // The maximum cofficient is 36028797018963967. 10**18 is more than that.
        ediff = min(ediff, 18);
        // We need to make them conform before we can compare. Multiply the first
        // coefficient by 10**(first exponent - second exponent)
        __int128 x_scaled = (__int128)x*power[ediff];
        dec64 x_high = (dec64)(x_scaled >> 64);
        x = (dec64)x_scaled;
        
        // in the case of overflow check the sign of higher 64-bit half;
        // otherwise compare numbers with equalized exponents
        return (x_high == x >> 63 ? x < y : x_high < 0) ? DEC64_TRUE : DEC64_FALSE;
    } else {
        // The maximum cofficient is 36028797018963967. 10**18 is more than that.
        ediff = min(-ediff, 18);
        // We need to make them conform before we can compare. Multiply the first
        // coefficient by 10**(first exponent - second exponent)
        __int128 y_scaled = (__int128)y*power[ediff];
        dec64 y_high = (dec64)(y_scaled >> 64);
        y = (dec64)y_scaled;
        
        // in the case of overflow check the sign of higher 64-bit half;
        // otherwise compare numbers with equalized exponents
        return (y_high == y >> 63 ? x < y : y_high >= 0) ? DEC64_TRUE : DEC64_FALSE;
    }
}

dec64 dec64_is_nan(dec64 x)
{
    return (signed char)x == -128 ? DEC64_TRUE : DEC64_FALSE;
}

dec64 dec64_is_zero(dec64 x)
{
    if((signed char)x == -128)
        return DEC64_FALSE;
    return (x & ~255) == 0 ? DEC64_TRUE : DEC64_FALSE;
}

static dec64 __dec64_add_slow(int64 x, int ex, int64 y, int ey)
{
    //static int total_runs = 0;
    //static int add_slow_adj_iters = 0;
    //++total_runs;

    // The slower path is taken when the exponents are different.
    // Before addition can take place, the exponents must be made to match.
    // Swap the numbers if the second exponent is greater than the first.
    int m = ex > ey;
    int64 r0 = m ? x : y, r0_ = r0;
    int64 r1 = m ? y : x;
    int e0 = m ? ex : ey, e1 = m ? ey : ex, e0_ = e0;

    // it's enough to check only e1 or -128;
    // if e1 is not 128, e0 cannot be -128, since it's greater
    if (e1 == -128) return DEC64_NULL;
    if (r0 == 0) e0 = e1;

    if (e0 > e1) {
        int64 abs_r0 = abs(r0);
        int lsb = abs_r0 == 0 ? 63 : __builtin_clzll(abs_r0) - 1;
        int log10_scale = lsb > 0 ? (lsb*77)>>8 : 0;
        log10_scale = min(e0 - e1, log10_scale);
        int64 r0_scaled;
        if (!__builtin_mul_overflow(r0, power[log10_scale], &r0_scaled)) {
            r0 = r0_scaled;
            e0 -= log10_scale;
        } /*else {
            // in theory, we should not get here
            assert(0);
        }*/

        for (;e0 > e1; e0--) {
            // First, try to decrease the first exponent using "lossless" multiplication
            // of the first coefficient by multiplying it by 10 at a time.
            if (__builtin_mul_overflow(r0, 10LL, &r0_scaled)) {
                // We cannot decrease the first exponent any more, so we must instead try to
                // increase the second exponent, which will result in a loss of significance.
                // That is the heartbreak of floating point.

                // Determine how many places need to be shifted. If it is more than 17, there is
                // nothing more to add.
                int ediff = e0 - e1;
                if (ediff > 17)
                    return dec64_new(r0_, e0_);
                r1 = r1 / power[ediff]; // shell we add +/-power[ediff]/2 before division?
                if (r1 == 0)
                    return dec64_new(r0_, e0_);
                return dec64_new(r0 + r1, e0);
            }
            //printf("\n==> add_slow_adj_iters=%d (out of %d runs)\n", ++add_slow_adj_iters, total_runs);
            r0 = r0_scaled;
        }
    }

    return dec64_new(r0 + r1, e0);
}

dec64 dec64_add(dec64 x, dec64 y)
{
    dec64 r;
    // Add two dec64 numbers together.
    // If the two exponents are both zero (which is usually the case for integers)
    // we can take the fast path. Since the exponents are both zero, we can simply
    // add the numbers together and check for overflow.
    
    if ((unsigned char)(x | y) == 0) {
        // integer case: both exponents are zero.
        if(!__builtin_add_overflow(x, y, &r))
            return r;
        // If there was an overflow (extremely unlikely) then we must make it fit.
        // pack knows how to do that.
        return dec64_new((x >> 8) + (y >> 8), 0);
    }
    else if ((unsigned char)(x ^ y) == 0) {
        int e = (signed char)x;
        if (e == -128) return DEC64_NULL;

        // The exponents match so we may add now. Zero out one of the exponents so there
        // will be no carry into the coefficients when the coefficients are added.
        // If the result is zero, then return the normal zero.
        if(!__builtin_add_overflow(x, y & ~255, &r))
            return (r & ~255) == 0 ? 0 : r;

        return dec64_new((x >> 8) + (y >> 8), e);
    }
    else
        return __dec64_add_slow(x >> 8, (signed char)x, y >> 8, (signed char)y);
}

dec64 dec64_subtract(dec64 x, dec64 y)
{
    dec64 r;
    // Add two dec64 numbers together.
    // If the two exponents are both zero (which is usually the case for integers)
    // we can take the fast path. Since the exponents are both zero, we can simply
    // add the numbers together and check for overflow.

    if ((unsigned char)(x | y) == 0) {
        // integer case: both exponents are zero.
        if(!__builtin_sub_overflow(x, y, &r))
            return r;
        // If there was an overflow (extremely unlikely) then we must make it fit.
        // pack knows how to do that.
        return dec64_new((x >> 8) - (y >> 8), 0);
    }
    else if ((unsigned char)(x ^ y) == 0) {
        int e = (signed char)x;
        if (e == -128) return DEC64_NULL;

        // The exponents match so we may add now. Zero out one of the exponents so there
        // will be no carry into the coefficients when the coefficients are added.
        // If the result is zero, then return the normal zero.
        if(!__builtin_sub_overflow(x, y & ~255, &r))
            return (r & ~255) == 0 ? 0 : r;

        return dec64_new((x >> 8) - (y >> 8), e);
    }
    
    return __dec64_add_slow(x >> 8, (signed char)x, -(y >> 8), (signed char)y);
}

dec64 dec64_divide(dec64 x, dec64 y)
{
    static const unsigned char fasttab[][2] = {
        {1, 0}, {5, 1}, {0, 0}, {25, 2}, {2, 1}, {0, 0}, {0, 0}, {125, 3}, {0, 0}, {1, 1},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {5, 2},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {4, 2}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {25, 3},
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {2, 2},
    };
    // (x: dec64, y: dec64) returns quotient: dec64
    // Divide a dec64 number by another.
    // Begin unpacking the components.
    int ex = (signed char)x;
    int ey = (signed char)y;
    x >>= 8;
    y >>= 8;
    if (x == 0 && ex != -128) return 0; // 0/y ~ 0, even if y == 0 or y == nan
    if ((ex == -128) | (ey == -128) | (y == 0)) return DEC64_NULL;

    // if both x and y are even then we can simplify the ratio lossless
    int b0 = __builtin_ctzll(x);
    int b1 = __builtin_ctzll(y);
    b0 = min(b0, b1);
    x >>= b0;
    y >>= b0;
    
    uint64 abs_y = (uint64)abs(y);
    int scale = 0;
    if (abs_y <= 50 && (scale=fasttab[abs_y-1][0]) != 0) {
        // fast division by some popular small constants
        // x/2 ~ (x*5)/10, x/5 ~ (x*2)/10, ...
        // and division by a power of 10 is just shift of the exponent
        return dec64_new(x*(y < 0 ? -scale : scale), ex - ey - fasttab[abs_y-1][1]);
    }

    // We want to get as many bits into the quotient as possible in order to capture
    // enough significance. But if the quotient has more than 64 bits, then there
    // will be a hardware fault. To avoid that, we compare the magnitudes of the
    // dividend coefficient and divisor coefficient, and use that to scale the
    // dividend to give us a good quotient.
    int log2_y = 63 - __builtin_clzll(abs_y);
    int log10_prescale = 0;
    
    for(;;) {
        uint64 abs_x = (uint64)abs(x);
        int log2_x = 63 - __builtin_clzll(abs_x);
    
        // Scale up the dividend to be approximately 58 bits longer than the divisor.
        // Scaling uses factors of 10, so we must convert from a bit count to a digit
        // count by multiplication by 77/256 (approximately LN2/LN10).
        log10_prescale = (log2_y + 58 - log2_x)*77 >> 8;
        if (log10_prescale <= 18) break;
        
        // If the number of scaling digits is larger than 18, then we will have to
        // scale in two steps: first prescaling the dividend to fill a register, and
        // then repeating to fill a second register. This happens when the divisor
        // coefficient is much larger than the dividend coefficient.
        
        // we want 58 bits or so in the dividend
        log10_prescale = (58 - log2_x)*77 >> 8;
        x *= power[log10_prescale];
        ex -= log10_prescale;
    }
        
    // Multiply the dividend by the scale factor, and divide that 128 bit result by
    // the divisor. Because of the scaling, the quotient is guaranteed to use most
    // of the 64 bits in r0, and never more. Reduce the final exponent by the number
    // of digits scaled.
    dec64 q = (dec64)((__int128)x*power[log10_prescale]/y);
    return dec64_new(q, ex - ey - log10_prescale);
}        

static dec64 __dec64_to_int(dec64 x_, int round_dir)
{
    // Produce the largest integer that is less than or equal to 'x' (round_dir == -1)
    // or greater than or equal to 'x' (round_dir == 1).
    // In the result, the exponent will be greater than or equal to zero unless it is nan.
    // Numbers with positive exponents will not be modified,
    // even if the numbers are outside of the safe integer range.

    int e = (signed char)x_;
    dec64 x = x_ >> 8;
    if (e == -128) return DEC64_NULL;

    if (x == 0) return 0;
    if (e >= 0) return x_;
    
    e = -e;
    int64 rem;
    if (e < 17) {
        int64 p = power[e];
        int64 x_scaled = x / p;
        rem = x - x_scaled*p;
        if (rem == 0)
            return x_scaled << 8;
        x = x_scaled;
    } else {
        // deal with a micro number
        rem = x;
        x = 0;
    }
    int delta = ((rem ^ round_dir) >= 0)*round_dir;
    return (x + delta) << 8;
}

dec64 dec64_floor(dec64 x)
{
    return __dec64_to_int(x, -1);
}

dec64 dec64_ceiling(dec64 x)
{
    return __dec64_to_int(x, 1);
}

dec64 dec64_integer_divide(dec64 x, dec64 y)
{
    int ex = (signed char)x, ey = (signed char)y;
    if (ex == ey) {
        x >>= 8;
        y >>= 8;
        if (x == 0 && ex != -128) return 0; // 0/y ~ 0, even if y == 0 or y == nan
        if ((ex == -128) | (ey == -128) | (y == 0)) return DEC64_NULL;
        // augment numerator to mimic floor(x/y), i.e. rounding towards minus infinity
        dec64 delta = (x ^ y) >= 0 ? 0 : (y > 0 ? 1 : -1) - y;
        return ((x + delta)/ y) << 8;
    }
    return dec64_floor(dec64_divide(x, y));
}

dec64 dec64_modulo(dec64 x, dec64 y)
{
    int ex = (signed char)x, ey = (signed char)y;
    if (ex == ey) {
        x >>= 8;
        y >>= 8;
        if (x == 0 && ex != -128) return 0; // 0 % y ~ 0, even if y == 0 or y == nan
        if ((ex == -128) | (ey == -128) | (y == 0)) return DEC64_NULL;
        dec64 rem = x % y;
        // augment result to mimic x mod y == x - floor(x/y)*y
        return rem == 0 ? 0 : ((rem + ((x ^ y) < 0 ? y : 0))<< 8) | (ex & 255);
    }
    return dec64_subtract(x, dec64_multiply(dec64_integer_divide(x, y), y));
}

dec64 dec64_multiply(dec64 x, dec64 y)
{
    // Multiply two dec64 numbers together.
    int ex = (signed char)x, ey = (signed char)y;
    
    x >>= 8; y >>= 8;
    
    // The result is nan if one or both of the operands is nan and neither of the
    // operands is zero.
    if ((x == 0 && ex != -128) || (y == 0 && ey != -128)) return 0;
    if (ex == -128 || ey == -128) return DEC64_NULL;
    
    __int128 r_big = (__int128)x*y;
    dec64 r_high = (dec64)(r_big >> 64);
    dec64 r = (dec64)r_big;
    int e = ex + ey;
    if (r_high == r >> 63) // no overflow
        return dec64_new(r, e);
    
    dec64 abs_r_high = abs(r_high);
    int delta_er = abs_r_high == 0 ? 1 : ((63 - __builtin_clzll(abs_r_high))*77 >> 8) + 2;
    
    // divide by the power of ten & pack the final result
    r = (dec64)(r_big / power[delta_er]);
    return dec64_new(r, e + delta_er);
}

dec64 dec64_neg(dec64 x)
{
    int e = (signed char)x;
    if (e == -128) return DEC64_NULL;

    dec64 r;
    if (!__builtin_add_overflow(x ^ ~255, 256, &r)) {
        return (r & ~255) != 0 ? r : 0;
    }

    // only one overflow case for x -> -x is possible, when x = 0x800...000 (2^55)
    return dec64_new(36028797018963968LL, e);
}

dec64 dec64_abs(dec64 x)
{
    if ((signed char)x == -128) return DEC64_NULL;
    return x < 0 ? dec64_neg(x) : (x >> 8) == 0 ? 0 : x;
}

dec64 dec64_normal(dec64 x)
{
    // Make the exponent as close to zero as possible without losing any signficance.
    // Usually normalization is not needed since it does not materially change the
    // value of a number.

    int e = (signed char)x;
    if (e == -128) return DEC64_NULL;
    if (e == 0) return x;

    x >>= 8;
    if(x == 0) return 0;
    
    if (e < 0) {
        // While the exponent is less than zero, divide the coefficient by 10 and
        // increment the exponent.
        do {
            dec64 tmp = x/10;
            if (x != tmp*10)
                break;
            x = tmp;
        } while(++e < 0);
        return (x << 8) | (e & 255);
    } else {
        // we keep the coefficient scaled by 256 to catch the overflow earlier,
        // inside 56 coefficient
        x <<= 8;
        
        // While the exponent is greater than zero, multiply the coefficient by 10 and
        // decrement the exponent. If the coefficient gets too large, wrap it up.
        do {
            dec64 tmp;
            if (__builtin_mul_overflow(x, 10, &tmp))
                break;
            x = tmp;
        } while(--e > 0);
        return x | (e & 255);
    }
}

dec64 dec64_round(dec64 x, dec64 place_)
{
    // The place argument indicates at what decimal place to round.
    //    -2        nearest cent
    //     0        nearest integer
    //     3        nearest thousand
    //     6        nearest million
    //     9        nearest billion

    // The place should be between -16 and 16.
    int ep = (signed char)place_;
    int64 place;
    if(ep != 0) {
        if (ep == -128) place = 0;
        else {
            place = dec64_normal(place_);
            if ((signed char)place != 0) return DEC64_NULL;
            place >>= 8;
        }
    } else place = place_ >> 8;

    int64 e = (signed char)x;
    int64 xc = x >> 8;
    
    if (e == -128) return DEC64_NULL;
    if (xc == 0) return 0;
    
    // no rounding required
    if (e >= place) return x;
    
    int is_neg = x < 0;
    uint64 abs_xc = (uint64)abs(xc), abs_xc_scaled = 0;
    do {
        abs_xc_scaled = (uint64)((unsigned __int128)abs_xc * (uint64)-3689348814741910323 >> 64);
        abs_xc = abs_xc_scaled >> 3;
    } while (++e < place);
    
    // Round if necessary and return the result.
    abs_xc += (abs_xc_scaled >> 2) & 1;
    // Restore the correct sign
    xc = is_neg ? -(int64)abs_xc : (int64)abs_xc;
    
    return dec64_new(xc, e);
}

dec64 dec64_signum(dec64 x)
{
    // If the number is nan, the result is nan.
    // If the number is less than zero, the result is -1.
    // If the number is zero, the result is 0.
    // If the number is greater than zero, the result is 1.
    int e = (signed char)x;
    if (e == -128) return DEC64_NULL;
    
    return x < 0 ? -(1LL << 8) : (x >> 8) == 0 ? 0 : 1 << 8;
}

dec64 dec64_from_double(double d)
{
    int e2, shift = 19;
    double m = frexp(d, &e2);
    double e10 = e2*0.3010299956639811952137388947;
    int e = (int)ceil(e10);
    m *= pow(10., (e10 - e));
    long long m64 = (long long)round(m*10000000000000000000.0);
    return dec64_round(dec64_new(m64, e - shift), (int)ceil(e10-15)*256);
}
