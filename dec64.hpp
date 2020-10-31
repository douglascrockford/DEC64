/* dec64.hpp

The DEC64 C++ wrapper on top of dec64.h.

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

#ifndef __DEC64_HPP__
#define __DEC64_HPP__

#include "dec64.h"
#include <math.h>
#include <string>
#include <limits.h>
#include <iostream>

struct dec64_ {
    dec64_() {}
    // There is no implicit or explicit constructor from a "raw" 64-bit representation,
    // not to misuse it as a constructor from integer,
    // i.e. should hypothetical dec64_(long long x) do val=x or val=x<<8???
    // Instead, there are static methods dec64_::raw() and dec64_::from_int().
    // Also, popular binary operations (+[=], -[=], *[=], /[=], %[=], ==, !=, <, >, <=, >=)
    // have overloaded 'mixed' variants with integer arguments.
    // This is just safer solution that helps to prevent some programming errors.
    // See also _d "constructor" below to quickly build floating-point literals.
    explicit dec64_(long long v, int e) : val(dec64_new(v, e)) {}
    // dec64_ is so lightweight class,
    // so it's faster to pass it by value rather than by reference
    static dec64_ raw(dec64 v) { dec64_ d; d.val = v; return d; }
    static dec64_ from_int(int i) { dec64_ d; d.val = i << 8; return d; }
    static dec64_ from_string(const std::string& str)
    {
        dec64_ d;
        d.val = str.empty() ? DEC64_NULL :
            dec64_from_string(dec64_default_state(), str.c_str());
        return d;
    }
    static dec64_ random() { dec64_ d; d.val = dec64_random(); return d; }
    dec64_ normalize() const { dec64_ d; d.val = dec64_normal(val); return d; }
    std::string as_string() const { return std::string(dec64_dump(val)); }
    int as_int() const {
        dec64 v = dec64_normal(dec64_round(val, 0));
        if ((signed char)v != 0) return INT_MAX;
        v >>= 8;
        return v < INT_MIN ? INT_MIN : v > INT_MAX ? INT_MAX : (int)v;
    }
    
    dec64_ div(dec64_ y) { return dec64_::raw(dec64_integer_divide(val, y.val)); }
    dec64_ div(int y) { return dec64_::raw(dec64_integer_divide(val, y << 8)); }
    dec64_ mod(dec64_ y) { return dec64_::raw(dec64_modulo(val, y.val)); }
    dec64_ mod(int y) { return dec64_::raw(dec64_modulo(val, y << 8)); }
    
    int exponent() const { return (signed char)val; }
    long long coef() const { return val >> 8; }
    
    dec64 val; // there is nothing to hide
    
    static dec64_ Nan, False, True;
};

// watch the precision!
// for critical things it's better to use dec64_::raw() or dec64_::dec64_(coef, exp).
static inline dec64_ operator "" _d(long double x) {
    dec64_ d;
    d.val = dec64_from_double((double)x);
    return d;
}

static inline bool isnan(dec64_ x) { return dec64_is_nan(x.val) == DEC64_TRUE; }
static inline bool isint(dec64_ x) { return dec64_is_integer(x.val) == DEC64_TRUE; }
static inline bool iszero(dec64_ x) { return dec64_is_zero(x.val) == DEC64_TRUE; }
static inline bool isfalse(dec64_ x) { return dec64_is_false(x.val) == DEC64_TRUE; }
static inline bool operator < (dec64_ x, dec64_ y) { return dec64_is_less(x.val, y.val) == DEC64_TRUE; }
static inline bool operator < (int x, dec64_ y) { return dec64_is_less(x<<8, y.val) == DEC64_TRUE; }
static inline bool operator < (dec64_ x, int y) { return dec64_is_less(x.val, y<<8) == DEC64_TRUE; }
static inline bool operator > (dec64_ x, dec64_ y) { return dec64_is_less(y.val, x.val) == DEC64_TRUE; }
static inline bool operator > (int x, dec64_ y) { return dec64_is_less(y.val, x<<8) == DEC64_TRUE; }
static inline bool operator > (dec64_ x, int y) { return dec64_is_less(y<<8, x.val) == DEC64_TRUE; }
static inline bool operator <= (dec64_ x, dec64_ y) { return dec64_is_less(y.val, x.val) == DEC64_FALSE; }
static inline bool operator <= (int x, dec64_ y) { return dec64_is_less(y.val, x<<8) == DEC64_FALSE; }
static inline bool operator <= (dec64_ x, int y) { return dec64_is_less(y<<8, x.val) == DEC64_FALSE; }
static inline bool operator >= (dec64_ x, dec64_ y) { return dec64_is_less(x.val, y.val) == DEC64_FALSE; }
static inline bool operator >= (int x, dec64_ y) { return dec64_is_less(x<<8, y.val) == DEC64_FALSE; }
static inline bool operator >= (dec64_ x, int y) { return dec64_is_less(x.val, y<<8) == DEC64_FALSE; }
static inline bool operator == (dec64_ x, dec64_ y) { return dec64_is_equal(x.val, y.val) == DEC64_TRUE; }
static inline bool operator == (int x, dec64_ y) { return dec64_is_equal(x<<8, y.val) == DEC64_TRUE; }
static inline bool operator == (dec64_ x, int y) { return dec64_is_equal(x.val, y<<8) == DEC64_TRUE; }
static inline bool operator != (dec64_ x, dec64_ y) { return dec64_is_equal(x.val, y.val) == DEC64_FALSE; }
static inline bool operator != (int x, dec64_ y) { return dec64_is_equal(x<<8, y.val) == DEC64_FALSE; }
static inline bool operator != (dec64_ x, int y) { return dec64_is_equal(x.val, y<<8) == DEC64_FALSE; }
static inline bool operator !(dec64_ x) {
    return dec64_is_false(x.val) == DEC64_TRUE || dec64_is_zero(x.val) == DEC64_TRUE;
}

static inline dec64_ operator + (dec64_ x, dec64_ y) { return dec64_::raw(dec64_add(x.val, y.val)); }
static inline dec64_ operator + (int x, dec64_ y) { return dec64_::raw(dec64_add(x << 8, y.val)); }
static inline dec64_ operator + (dec64_ x, int y) { return dec64_::raw(dec64_add(x.val, y << 8)); }
static inline dec64_ operator - (dec64_ x, dec64_ y) { return dec64_::raw(dec64_subtract(x.val, y.val)); }
static inline dec64_ operator - (int x, dec64_ y) { return dec64_::raw(dec64_subtract(x << 8, y.val)); }
static inline dec64_ operator - (dec64_ x, int y) { return dec64_::raw(dec64_subtract(x.val, y << 8)); }
static inline dec64_ operator * (dec64_ x, dec64_ y) { return dec64_::raw(dec64_multiply(x.val, y.val)); }
static inline dec64_ operator * (int x, dec64_ y) { return dec64_::raw(dec64_multiply(x << 8, y.val)); }
static inline dec64_ operator * (dec64_ x, int y) { return dec64_::raw(dec64_multiply(x.val, y << 8)); }
static inline dec64_ operator / (dec64_ x, dec64_ y) { return dec64_::raw(dec64_divide(x.val, y.val)); }
static inline dec64_ operator / (int x, dec64_ y) { return dec64_::raw(dec64_divide(x << 8, y.val)); }
static inline dec64_ operator / (dec64_ x, int y) { return dec64_::raw(dec64_divide(x.val, y << 8)); }
static inline dec64_ operator % (dec64_ x, dec64_ y) { return dec64_::raw(dec64_modulo(x.val, y.val)); }
static inline dec64_ operator % (int x, dec64_ y) { return dec64_::raw(dec64_modulo(x << 8, y.val)); }
static inline dec64_ operator % (dec64_ x, int y) { return dec64_::raw(dec64_modulo(x.val, y << 8)); }
static inline dec64_ operator - (dec64_ x) { return dec64_::raw(dec64_neg(x.val)); }
static inline dec64_ abs(dec64_ x) { return dec64_::raw(dec64_abs(x.val)); }
static inline dec64_ sign(dec64_ x) { return dec64_::raw(dec64_signum(x.val)); }
static inline int isign(dec64_ x) {
    int e = (signed char)x.val;
    if (e == -128) return 0;
    long long v = x.val;
    return v < 0 ? -1 : v > 0;
}

static inline dec64_& operator += (dec64_& x, dec64_ y) { x.val = dec64_add(x.val, y.val); return x; }
static inline dec64_& operator += (dec64_& x, int y) { x.val = dec64_add(x.val, y << 8); return x; }
static inline dec64_& operator -= (dec64_& x, dec64_ y) { x.val = dec64_subtract(x.val, y.val); return x; }
static inline dec64_& operator -= (dec64_& x, int y) { x.val = dec64_subtract(x.val, y << 8); return x; }
static inline dec64_& operator *= (dec64_& x, dec64_ y) { x.val = dec64_multiply(x.val, y.val); return x; }
static inline dec64_& operator *= (dec64_& x, int y) { x.val = dec64_multiply(x.val, y << 8); return x; }
static inline dec64_& operator %= (dec64_& x, dec64_ y) { x.val = dec64_modulo(x.val, y.val); return x; }
static inline dec64_& operator %= (dec64_& x, int y) { x.val = dec64_modulo(x.val, y << 8); return x; }

static inline dec64_ floor(dec64_ x) { return dec64_::raw(dec64_floor(x.val)); }
static inline dec64_ ceil(dec64_ x) { return dec64_::raw(dec64_ceiling(x.val)); }
static inline dec64_ round(dec64_ x, int place=0) { return dec64_::raw(dec64_round(x.val, place << 8)); }
static inline dec64_ round(dec64_ x, dec64_ place) { return dec64_::raw(dec64_round(x.val, place.val)); }
static inline dec64_ sin(dec64_ x) { return dec64_::raw(dec64_sin(x.val)); }
static inline dec64_ cos(dec64_ x) { return dec64_::raw(dec64_cos(x.val)); }
static inline dec64_ tan(dec64_ x) { return dec64_::raw(dec64_tan(x.val)); }
static inline dec64_ asin(dec64_ x) { return dec64_::raw(dec64_asin(x.val)); }
static inline dec64_ acos(dec64_ x) { return dec64_::raw(dec64_acos(x.val)); }
static inline dec64_ atan(dec64_ x) { return dec64_::raw(dec64_atan(x.val)); }
static inline dec64_ atan2(dec64_ y, dec64_ x) { return dec64_::raw(dec64_atan2(y.val, x.val)); }
static inline dec64_ exp(dec64_ x) { return dec64_::raw(dec64_exp(x.val)); }
static inline dec64_ log(dec64_ x) { return dec64_::raw(dec64_log(x.val)); }
static inline dec64_ pow(dec64_ x, dec64_ y) { return dec64_::raw(dec64_raise(x.val, y.val)); }
static inline dec64_ sqrt(dec64_ x) { return dec64_::raw(dec64_sqrt(x.val)); }

static inline std::ostream& operator << (std::ostream& ostrm, dec64_ x)
{
    ostrm << dec64_dump(x.val);
    return ostrm;
}

static inline std::istream& operator << (std::istream& istrm, dec64_& x)
{
    std::string str;
    istrm >> str;
    x = dec64_::from_string(str);
    return istrm;
}

#endif
