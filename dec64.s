; dec64.s
; 2019-10-03
; Public Domain

; No warranty expressed or implied. Use at your own risk. You have been warned.


; This file contains the source assembly code for the ARM64 implementation of
; the DEC64 library, being the elementary arithmetic operations for DEC64, a
; decimal floating point type.
;
; This file has been tested with Visual Studio 19.

; DEC64 uses 64 bits to represent a number. The low order 8 bits contain an
; exponent that ranges from -127 to 127. The exponent is not biased. The
; exponent -128 is reserved for nan (not a number). The remaining 56 bits,
; including the sign bit, are the coefficient in the range
; -36 028 797 018 963 968 thru 36 028 797 018 963 967. The exponent and the
; coefficient are both twos complement signed numbers.
;
; The value of any non-nan DEC64 number is coefficient * (10 ** exponent).
;
; Rounding is to the nearest value. Ties are rounded away from zero. Integer
; division is floored. The result of modulo has the sign of the divisor.
; There is no negative zero.
;
; All 72057594037927936 values with an exponent of -128 are nan values.

; When these functions return nan, they will always return DEC64_NULL,
; the normal nan value.

; These operations will produce a result of DEC64_NULL:
;
;   dec64_abs(nan)
;   dec64_ceiling(nan)
;   dec64_floor(nan)
;   dec64_neg(nan)
;   dec64_normal(nan)
;   dec64_signum(nan)
;
; These operations will produce a result of zero for all values of n,
; even if n is nan:
;
;   dec64_divide(0, n)
;   dec64_integer_divide(0, n)
;   dec64_modulo(0, n)
;   dec64_multiply(0, n)
;   dec64_multiply(n, 0)
;
; These operations will produce a result of DEC64_NULL for all values of n
; except zero:
;
;   dec64_divide(n, 0)
;   dec64_divide(n, nan)
;   dec64_integer_divide(n, 0)
;   dec64_integer_divide(n, nan)
;   dec64_modulo(n, 0)
;   dec64_modulo(n, nan)
;   dec64_multiply(n, nan)
;   dec64_multiply(nan, n)
;
; These operations will produce a result of normal nan for all values of n:
;
;   dec64_add(n, nan)
;   dec64_add(nan, n)
;   dec64_divide(nan, n)
;   dec64_integer_divide(nan, n)
;   dec64_modulo(nan, n)
;   dec64_round(nan, n)
;   dec64_subtract(n, nan)
;   dec64_subtract(nan, n)
;
;       You know what goes great with those defective pentium chips?
;       Defective pentium salsa! (David Letterman)

; All public symbols have a dec64_ prefix. All other symbols are private.

    global dec64_abs [func];(number: dec64)
;   returns absolution: dec64

    global dec64_add [func];(augend: dec64, addend: dec64)
;   returns sum: dec64

    global dec64_ceiling [func];(number: dec64)
;   returns integer: dec64

    global dec64_coefficient [func];(number: dec64)
;   returns coefficient: int64

    global dec64_divide [func];(dividend: dec64, divisor: dec64)
;   returns quotient: dec64

    global dec64_exponent [func];(number: dec64)
;   returns exponent: int64

    global dec64_floor [func];(number: dec64)
;   returns integer: dec64

    global dec64_integer_divide [func];(dividend: dec64, divisor: dec64)
;   returns quotient: dec64

    global dec64_is_equal [func];(comparahend: dec64, comparator: dec64)
;   returns comparison: dec64

    global dec64_is_false [func];(boolean: dec64)
;   returns comparison: dec64

    global dec64_is_integer [func];(number: dec64)
;   returns comparison: dec64

    global dec64_is_less [func];(comparahend: dec64, comparator: dec64)
;   returns comparison: dec64

    global dec64_is_nan [func];(number: dec64)
;   returns comparison: dec64

    global dec64_is_zero [func];(number: dec64)
;   returns comparison: dec64

    global dec64_modulo [func];(dividend: dec64, divisor: dec64)
;   returns modulus: dec64

    global dec64_multiply [func];(multiplicand: dec64, multiplier: dec64)
;   returns product: dec64

    global dec64_neg [func];(number: dec64)
;   returns negation: dec64

    global dec64_new [func];(coefficient: int64, exponent: int64)
;   returns number: dec64

    global dec64_normal [func];(number: dec64)
;   returns normalization: dec64

    global dec64_round [func];(number: dec64, place: dec64)
;   returns quantization: dec64

    global dec64_signum [func];(number: dec64)
;   returns signature: dec64

    global dec64_subtract [func];(minuend: dec64, subtrahend: dec64)
;   returns difference: dec64


; All of the public functions in this file accept up to two arguments,
; which are passed in registers (x0, x1), returning a result in x0.

; Registers x0 thru x14 and x8 thru x7 may be clobbered.
; The other registers are not disturbed.
; The stack is not touched in any way.
; Register x14 is used as an auxiliary return link register.

    area dec64, align=4, code, readonly

power                               ; the powers of 10
    dcq     1                       ; 0
    dcq     10                      ; 1
    dcq     100                     ; 2
    dcq     1000                    ; 3
    dcq     10000                   ; 4
    dcq     100000                  ; 5
    dcq     1000000                 ; 6
    dcq     10000000                ; 7
    dcq     100000000               ; 8
    dcq     1000000000              ; 9
    dcq     10000000000             ; 10
    dcq     100000000000            ; 11
    dcq     1000000000000           ; 12
    dcq     10000000000000          ; 13
    dcq     100000000000000         ; 14
    dcq     1000000000000000        ; 15
    dcq     10000000000000000       ; 16
    dcq     100000000000000000      ; 17
    dcq     1000000000000000000     ; 18
    dcq     10000000000000000000    ; 19

;  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_coefficient;(number: dec64) returns coefficient: int64

; Return the coefficient part from a dec64 number.

    asr     x0, x0, 8
    ret

;  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_exponent;(number: dec64) returns exponent: int64

; Return the exponent part, sign extended to 64 bits, from a dec64 number.
; dec64_exponent(nan) returns -128.

    sxtb    x0, w0
    ret

;  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_new;(coefficient: int64, exponent: int64) returns number: dec64

; The dec64_new function will combine the coefficient and exponent into a dec64.
; Numbers that are too tiny to be contained in this format become zero.
; Numbers that are too huge to be contained in this format become nan.
; Clobbers x1, x4 thru x7, x11.

; The coefficient is in x0.
; The exponent is in x1.

    mov     x11, x1                 ; x11 is the exponent
    mov     x1, xzr                 ; x1 is zero

new

    mov     x7, 10

; If the exponent is less than -127, or if abs(coefficient) >= 2**55 then we
; must shrink the coefficient.

    asr     x4, x0, 55              ; x4 is the 9 msb of exponent, sign extended
    add     x4, x4, x4, lsr 63      ; x4 is zero if the number fits
    sub     x5, x11, -127           ; x5 is negative if exponent is too negative
    orr     x4, x4, x5, asr 63
    cbnz    x4, new_shrink

new_almost_done

; If the exponent is too large, then we must grow the coefficient.

    subs    xzr, x11, 127
    b.gt    new_grow

new_done

; Pack the exponent and coefficient together to form a new dec64.

    cbz     x0, return
    and     x4, x11, 0xFF
    add     x0, x4, x0, lsl 8
    ret

new_grow

; The coefficient is good, but the exponent is too big.
; We will try to grow the coefficient by mutliplying by ten.

    mul     x0, x0, x7
    sub     x11, x11, 1

; Is the coefficient still in range?

    asr     x4, x0, 55              ; x4 is the 9 msb of exponent, sign extended
    add     x4, x4, x4, lsr 63      ; x4 is zero if the number fits

; If so, we are almost done.

    cbz     x4, new_almost_done

; The number is too big to represent as a DEC64. So sad.

    b       return_null

new_shrink

; Divide the coefficient by 10 (remembering its old value in x6)
; and increment the exponent.

    mov     x6, x0
    sdiv    x0, x0, x7
    add     x11, x11, 1

; Are the coefficient and exponent now in range? If not, keep shrinking.

    asr     x4, x0, 55              ; x4 is the 9 msb of exponent, sign extended
    add     x4, x4, x4, lsr 63      ; x4 is zero if the number fits
    sub     x5, x11, -127           ; x5 is negative if exponent is too negative
    orr     x4, x4, x5, asr 63
    cbnz    x4, new_shrink

; Is the absolute value of the remainder greater than or equal to 5?

    msub    x5, x0, x7, x6          ; x5 is old coefficient - coefficient * 10
    ands    xzr, x5, x5             ; is the remainder negative?
    cneg    x5, x5, mi              ; x5 is abs(remainder)
    subs    xzr, x5, 5              ; is the remainder 5 or more?
    b.ge    new_round               ; rounding is required
    subs    xzr, x11, 127           ; is the exponent still in range?
    b.le    new_done
    b       return_null

new_round

; If so, round the coefficient away from zero.

    asr     x6, x6, 63              ; x6 is the sign extension
    orr     x6, x6, 1               ; x6 is -1 or 1
    add     x0, x0, x6              ; x0 is the rounded coefficient
    b       new

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_round;(number: dec64, place: dec64) returns quantization: dec64

; The place argument indicates at what decimal place to round.
;    -2        nearest cent
;     0        nearest integer
;     3        nearest thousand
;     6        nearest million
;     9        nearest billion

; The place should be between -16 and 16.

    asr     x4, x0, 8               ; x4 is coefficient of number
    sxtb    x11, w0                 ; x11 is exponent of number
    asr     x6, x1, 8               ; x6 is coefficient places
    sxtb    x5, w1                  ; x5 is exponent of places
    subs    xzr, x11, -128
    b.eq    return_null
    cbz     x4, return_zero         ; is number zero?

; If places is null, use zero.

    subs    xzr, x5, -128
    csel    x5, xzr, x5, eq
    csel    x6, xzr, x6, eq
    cbnz    x5, round_normal        ; if places is not an integer, normalize

    subs    xzr, x11, x6            ; are we done?
    b.ge    return
    mov     x10, 10

round_loop

; Increment the exponent and divide the coefficient by 10 until the target
; exponent is reached.

    cbz     x4, return_zero
    mov     x5, x4                  ; x5 is old coefficient
    sdiv    x4, x4, x10             ; x4 is coefficient / 10
    add     x11, x11, 1

    subs    xzr, x11, x6
    b.lt    round_loop

; If abs(remainder) is 5 or more, bump the coefficient.

    msub    x5, x4, x10, x5         ; x5 is the remainder
    ands    xzr, x4, x4             ; is the number negative?
    cneg    x5, x5, mi              ; x5 is abs(remainder)
    asr     x8, x4, 63              ; x8 is -1 or 0
    orr     x8, x8, 1               ; x8 is -1 or 1
    subs    xzr, x5, 5
    csel    x8, x8, xzr, ge         ; x8 is zero if no rounding needed
    add     x0, x4, x8
    b       new

round_normal

; If places is not obviously an integer, then attempt to normalize it.

    mov     x14, x0                 ; x14 is the number
    mov     x15, x30                ; x15 is return address
    mov     x0, x1
    adr     x30, 8
    b       dec64_normal
    mov     x30, x15
    ands    xzr, x0, 0xFF
    b.ne    return_null
    mov     x1, x0
    mov     x0, x14
    b       dec64_round

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_add;(augend: dec64, addend: dec64) returns sum: dec64

; Add two dec64 numbers together.

; If the two exponents are both zero (which is usually the case for integers)
; we can take the fastest path. Since the exponents are both zero, we can
; simply add the numbers together and check for overflow.
; Clobbers x4 thru x11

    orr     x5, x0, x1
    ands    xzr, x5, 255
    b.ne    add_begin
    adds    x0, x0, x1
    b.vs    add_overflow
    ret

add_overflow

; The fast add overflowed. This is very uncommon. The exponent is in the bottom
; of x1. x0 contains the scaled coefficient, missing its most significant bit.

    sub     x0, x0, x1              ; undo the addition
    sxtb    x11, w1                 ; x11 is the exponent
    asr     x0, x0, 8
    add     x0, x0, x1, asr 8
    b       new

add_begin

; If the exponents are equal, then we can add fast

    sxtb    x5, w0                  ; x5 is the first exponent
    subs    xzr, x5, -128           ; Make sure the augend is not nan.
    b.eq    return_null
    sxtb    x7, w1                  ; x7 is the second exponent
    bfxil   x0, xzr, 0, 8           ; clear the x0 exponent
    subs    xzr, x5, x7             ; are the exponents equal?
    b.eq    add_fast

; The exponents must be made equal before we can add.

    mov     x10, 10                 ; x10 is 10
    asr     x4, x0, 8               ; x4 is the first coefficient
    asr     x6, x1, 8               ; x6 is the second coefficient

add_slow

; Make sure that the number with the larger exponent is in (x4, x5).
; The other goes in (x6, x7).

    subs    xzr, x7, -128           ; Make sure the addend is not nan.
    b.eq    return_null
    subs    xzr, x5, x7
    b.ge    add_grow
    mov     x8, x4
    mov     x9, x5
    mov     x4, x6
    mov     x5, x7
    mov     x6, x8
    mov     x7, x9

add_grow

; If the exponents are not equal, try growing x4.
; First make sure there is some headroom.

    subs    xzr, x5, x7
    b.eq    add_ready
    asr     x11, x4, 58
    eor     x11, x11, x4, asr 63
    cbnz    x11, add_shrink
    mul     x4, x4, x10
    sub     x5, x5, 1
    b       add_grow

add_shrink

; If the exponents are not equal yet, try shrinking x6.

    sdiv    x6, x6, x10
    add     x7, x7, 1
    subs    xzr, x5, x7
    b.ne    add_shrink

add_ready

; The exponents are equal. We are ready to add.

    add     x0, x4, x6
    mov     x11, x5
    b       new

add_fast

    adds    x0, x0, x1
    b.vs    add_overflow
    asr     x10, x0, 8
    cbz     x10, return_zero
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_subtract;(minuend: dec64, subtrahend: dec64) returns difference: dec64

; Subtract two dec64 by negating the subtrahend and adding. The complication
; is that the coeffient -36028797018963968 is not like the other coefficients.

; Negate the subtrahend coefficient without changing the exponent.

    eor     x1, x1, 0xFFFFFFFFFFFFFF00
    adds    x1, x1, 256             ; 2s complement adds 1 after a 'not'
    b.vc    dec64_add               ; if it did not overflow, we can add

; Negating the subtrahend caused an overflow.
; Set things up to jump into the add slow path.

    asr     x4, x0, 8               ; x4 is the first coefficient
    sxtb    x5, w0                  ; x5 is the first exponent
    mov     x6, 0x80000000000000
    sxtb    x7, w1                  ; x7 is the second exponent
    subs    xzr, x5, -128           ; make sure the minuend is not nan
    b.ne    add_slow
    b       return_null

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_ceiling;(number: dec64) returns integer: dec64

; Produce the smallest integer that is greater than or equal to the number.
; In the result, the exponent will be greater than or equal to zero unless
; it is nan. Numbers with positive exponents will not be modified, even if
; the numbers are outside of the safe integer range.

    mov     x7, 1                   ; x7 is the incrementor (round up)
    b       floor_begin

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_floor;(number: dec64) returns integer: dec64

; Produce the largest integer that is less than or equal to the number. This
; is sometimes called the entier function. In the result, the exponent will
; be greater than or equal to zero unless it is nan. Numbers with positive
; exponents will not be modified, even if the numbers are outside of the safe
; integer range.
: Clobbers x4 thru x10

    mov     x7, -1                  ; x7 is the incrementor (round down)

floor_begin

    asr     x4, x0, 8               ; x4 is the coefficient
    tbnz    x0, 7, floor_fractional
    cbz     x4, return_zero
    ret

floor_fractional

    sxtb    x5, w0                  ; x5 is the exponent
    sub     x5, xzr, x5             ; x5 is abs(exponent)
    subs    xzr, x5, 16             ; is the number super dinky?
    b.gt    floor_dinky
    adr     x10, power
    ldr     x10, [x10, x5 lsl 3]    ; get a power of 10
    sdiv    x0, x4, x10             ; x0 is coefficient / power of 10
    msub    x8, x0, x10, x4         ; x8 is the remainder

; Three things determine if x0 needs to be incremented, decremented,
; or left alone:
;       Is this 'floor' or 'ceiling' (x7)
;       Is the remainder zero (x8)
;       Is the coefficient negative (x4)

floor_inc

    eor     x9, x4, x7              ; if number & incrementor are not same sign
    bic     x7, x7, x9, asr 63      ; then incrementor is zero
    subs    xzr, x8, xzr            ; if the remainder is zero
    csel    x7, xzr, x7, eq         ; then incrementor is zero
    add     x0, x0, x7
    lsl     x0, x0, 8
    ret

floor_dinky

    subs    xzr, x5, 0x80
    b.ge    return_null
    mov     x8, x4
    mov     x0, xzr
    b       floor_inc

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_multiply;(multiplicand: dec64, multiplier: dec64) returns product: dec64

; Multiply two dec64 numbers together.
; Clobber: x4 thru x11

    asr     x4, x0, 8               ; x4 is the first coefficient
    sxtb    x5, w0                  ; x5 is the first exponent
    asr     x6, x1, 8               ; x6 is the second coefficient
    sxtb    x7, w1                  ; x7 is the second exponent

    subs    xzr, x5, -128
    csetm   x8, ne                  ; x8 is 0 if first exponent is null
    subs    xzr, x7, -128
    csel    x8, x8, xzr, ne         ; x8 is 0 if either exponent is null
    cbz     x8, multiply_null

    ands    xzr, x4, x4
    csetm   x8, ne                  ; x8 is 0 if first coefficient is 0
    ands    xzr, x6, x6
    csel    x8, x8, xzr, ne         ; x8 is 0 if either coefficient is 0
    cbz     x8, return_zero

    eor     x0, x4, x6              ; x0 is the sign of the result
    ands    xzr, x4, x4
    cneg    x4, x4, mi              ; x4 is abs(first coefficient)
    ands    xzr, x6, x6
    cneg    x6, x6, mi              ; x6 is abs(second coefficient)
    mul     x8, x4, x6              ; x8 is the low part of the product
    umulh   x9, x4, x6              ; x9 is the high part of the product
    add     x11, x5, x7             ; x11 is the new exponent

multiply_size

    cbnz    x9, multiply_reduce     ; is the product too big?
    tbz     x8, 63, multiply_sign   ; does the product fit?

; The product coefficient contains one too many bits. Divide it by 10 and
; increment the exponent.

    mov     x10, 10
    udiv    x8, x8, x10
    add     x11, x11, 1

multiply_sign

    ands    xzr, x0, x0
    cneg    x0, x8, mi              ; x0 is signed product coefficient
    b       new

multiply_reduce

; The product coefficent is in two words (x9, x8). We need to get it down to one
; word. Count the number of leading zero bits to get an estimate of the number
; of excess digits. Then divide.

    clz     x5, x9                  ; x5 is the number of leading zeros
    mov     x4, 69                  ; x4 is 69 (64 + fudge)
    mov     x7, 77                  ; x7 is 77
    sub     x4, x4, x5              ; x4 is number of excess bits
    mul     x4, x4, x7              ; x4 is x5 * 77
    lsr     x4, x4, 8               ; x4 is number of excess digits
    adr     x10, power
    ldr     x6, [x10, x4, lsl 3]    ; x6 is a power of ten
    add     x11, x11, x4            ; pump up the exponent
    clz     x7, x6                  ; count the leading 0 in high dividend
    mov     x4, 64
    sub     x7, x4, x7              ; x7 is sigbits in power of ten

; x0 is the sign of the product
; x6 is the power of ten
; x7 is the sigbits in the power of ten
; x9, x8 is the oversized product
; x11 is the exponent

    b       divide_big

multiply_null

    subs    xzr, x0, 0x80
    csel    x4, x0, x4, eq
    subs    xzr, x1, 0x80
    csel    x6, x1, x6, eq
    cbz     x4, return_zero
    cbz     x6, return_zero
    b       return_null

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_divide;(dividend: dec64, divisor: dec64) returns quotient: dec64

; Divide a dec64 number by another.
; Clobbers x4 thru x11.

    asr     x4, x0, 8               ; x4 is the dividend coefficient
    cbz     x4, divide_zero         ; is the dividend is 0?
    sxtb    x5, w0                  ; x5 is the dividend exponent
    asr     x6, x1, 8               ; x6 is the divisor coefficient
    sxtb    x7, w1                  ; x7 is the divisor exponent

; If the divisor is zero, or if either number is nan, the result is nan.

    subs    xzr, x5, -128
    csetm   x9, ne                  ; x9 is 0 if dividend exponent is nan
    subs    xzr, x7, -128
    csel    x9, x9, xzr, ne         ; x9 is 0 if divisor exponent is nan
    ands    xzr, x6, x6
    csel    x9, x9, xzr, ne         ; x9 is 0 if divisor coefficient is 0
    cbz     x9, return_null

; There are a couple of interesting special cases.

    subs    xzr, x1, 0x200          ; is the divisor 2?
    b.eq    divide_two
    subs    xzr, x1, 0x100          ; is the divisor 1?
    b.eq    return

; The exponent of the quotient is the difference of the input exponents.

    sub     x11, x5, x7             ; x11 is the quotient exponent

; Save the sign of the quotient. We will mostly be using unsigned arithmetic.

    eor     x0, x4, x6              ; x0 is the sign of the quotient

; Make the arguments positive.

    ands    xzr, x4, x4
    cneg    x4, x4, mi              ; x4 is abs(dividend coefficient)
    ands    xzr, x6, x6
    cneg    x6, x6, mi              ; x6 is abs(divisor coefficient)

; This is a floating point divide, so we want to preserve as much information in
; the quotient as possible. To do this, we will scale up the dividend by a
; suitable power of ten, reducing the exponent by a comparable amount.

    clz     x5, x4                  ; x5 is leading zeros of dividend
    mov     x9, 64                  ; x9 is 64
    clz     x7, x6                  ; x7 is leading zeros of divisor
    sub     x5, x9, x5              ; x5 is sigbits in dividend
    sub     x7, x9, x7              ; x7 is sigbits in divisor
    add     x8, x7, 59              ; x8 is sigbits needed in dividend
    sub     x8, x8, x5              ; x8 is additional sigbits required

; To convert bits to digits, we multiply by log10/log2 (0.30103), which is almost
; equal to 77/256 (0.30078).

    mov     x10, 77
    mul     x8, x8, x10
    lsr     x8, x8, 8               ; x8 is the number of digits to inflate
    sub     x11, x11, x8            ; subtract the new digits from the exponent

; The number of new digits could be as great as 34, but the ARM64 multiplier can
; only take multipliers as great as 19 digits. So it might be necessary to split
; the multiplication into two parts.

; The optional first part produces a product that fits in one word.

    adr     x9, power               ; x9 is the address of the power of 10 table
    subs    x10, x8, 19             ; is the number of digits more than 19?
    b.le    divide_inflate          ; if not, then a single multiply is needed
    ldr     x10, [x9, x10, lsl 3]   ; x10 is the first power of ten
    mul     x4, x4, x10             ; multiply the dividend by the first part
    mov     x8, 19                  ; the next multiply will be by 1e19

divide_inflate

; Put the inflated dividend in (x9, x8).

    ldr     x10, [x9, x8, lsl 3]    ; load the power of ten
    mul     x8, x4, x10             ; x8 is the low half of the dividend
    umulh   x9, x4, x10             ; x9 is the high half of the dividend

; Align the dividend and divisor by their leading 1 bits.
; How we do this depends on the size of the dividend.

    cbnz    x9, divide_big

; If the dividend has only 1 word, then shift the divisor.

    clz     x5, x8                  ; count the leading 0 in low dividend
    mov     x4, 64                  ; x4 is 64
    mov     x9, x8                  ; move the low part to the high part
    sub     x5, x4, x5              ; x5 is sigbits in dividend
    mov     x8, xzr                 ; zero out the low part
    sub     x10, x5, x7             ; x10 is the countdown
    lsl     x6, x6, x10             ; align the divisor
    b       divide_ready

divide_big

; If the dividend has two words, then shift the dividend.

    clz     x5, x9                  ; count the leading 0 in high dividend
    mov     x4, 64                  ; x4 is 64 (word size)
    sub     x5, x4, x5              ; x5 is sigbits in high dividend
    sub     x10, x7, x5             ; x10 is left shift distance
    sub     x4, x4, x10             ; x4 is right shift distance
    lsl     x9, x9, x10             ; shift high dividend
    lsr     x4, x8, x4              ; x4 is the carry
    orr     x9, x9, x4              ; insert the carry into the high dividend
    lsl     x8, x8, x10             ; shift the low dividend
    add     x10, x5, 64             ; x10 is sigbits in whole dividend (x9, x8)
    sub     x10, x10, x7            ; x10 is the countdown

divide_ready

    mov     x7, xzr                 ; x7 is the quotient

divide_step

; In each divide step:
;           Double the quotient
;           Find the difference between the aligned dividend and divisor
;           If the difference is not negative
;               Add 1 to the quotient
;               Subtract the divisor from the dividend
;           Double the dividend (x9, x8)
;           Decrement the countdown

    subs    x4, x9, x6              ; x4 is high dividend - divisor
    cset    x5, pl                  ; x5 is 1 if difference is positive
    csel    x9, x4, x9, pl          ; x9 is the difference if positive
    add     x7, x5, x7, lsl 1       ; double quotient and + 1 if positive diff
    lsr     x5, x8, 63              ; x5 is carry (high bit of low dividend)
    orr     x9, x5, x9, lsl 1       ; shift high dividend and insert carry
    lsl     x8, x8, 1               ; shift low dividend
    subs    x10, x10, 1             ; decrement countdown
    b.pl    divide_step             ; is it done?

; Correct the sign and get out.

    ands    xzr, x0, x0             ; should it negative?
    cneg    x0, x7, mi              ; correct the sign
    b       new

divide_zero

; If x0 is not 0x80, return zero.

    subs    xzr, x0, 0x80
    b.ne    return_zero
    b       return_null

divide_two

; Divide a dec64 number by two. If it is even, we can do a shift. If it is odd,
; then we decrement the exponent and multiply the coefficient by 5.

    tbnz    x4, 0, divide_two_odd   ; the coefficient is odd
    cbz     x4, return_zero
    lsl     x0, x4, 7
    bfi     x0, x5, 0, 8
    ret

divide_two_odd

; Multiply by 5 and divide by 10.

    sub     x11, x5, 1              ; decrease the exponent
    add     x0, x4, x4, lsl 2       ; x0 is coefficient * 5
    b       new

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_integer_divide;(dividend: dec64, divisor: dec64) returns quotient: dec64

; Divide, with a floored integer result. It produces the same result as
;    dec64_floor(dec64_divide(dividend, divisor))

; Clobbers x12. It can also clobber more via dec64_divide and dec64_floor.

; If either exponent is not zero, or if either coefficient is negative, then do
; it the hard way.

    orr     x12, x0, x1
    ands    xzr, x12, 255
    orr     x12, x12, x12, asr 63
    cbnz    x12, integer_divide_hard

    cbz     x0, return
    asr     x12, x1, 8              ; x12 is the divisor
    cbz     x12, return_null        ; divide by zero

    sdiv    x0, x0, x12
    lsl     x0, x0, 8
    ret

integer_divide_hard

    mov     x12, x30                ; save the return address in x12
    bl      dec64_divide
    mov     x30, x12                ; restore the return address
    b       dec64_floor             ; tail call

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_modulo;(dividend: dec64, divisor: dec64) returns modulus: dec64

; Modulo. It produces the same result as
;    dec64_subtract(
;        dividend,
;        dec64_multiply(
;            dec64_floor(
;                dec64_divide(
;                    dividend,
;                    divisor
;                )
;            ),
;            divisor
;        )
;    )

    asr     x4, x0, 8               ; x4 is dividend coefficient
    cbz     x4, modulo_zero
    asr     x6, x1, 8               ; x6 is divisor coefficient
    cbz     x6, return_null
    mov     x14, x0                 ; x14 is dividend
    mov     x15, x30                ; x15 is return address
    adr     x30, 8
    b       dec64_integer_divide
    adr     x30, 8
    b       dec64_multiply
    mov     x1, x0
    mov     x0, x14
    mov     x30, x15
    b       dec64_subtract

modulo_zero

    subs    xzr, x0, 0x80
    b.ne    return_zero
    b       return_null

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_signum;(number: dec64) returns signature: dec64

; If the number is nan, the result is nan.
; If the number is less than zero, the result is -1.
; If the number is zero, the result is 0.
; If the number is greater than zero, the result is 1.

    and     x5, x0, 0xFF            ; x5 is the exponent
    subs    xzr, x5, 0x80           ; is it nan?
    b.eq    return_null
    adds    xzr, xzr, x0, asr 8     ; is the coefficient zero?
    cset    x4, ne                  ; x4 is either 1 or 0
    asr     x0, x0, 63              ; x0 is either -1 or 0
    orr     x0, x0, x4              ; x0 is either -1, 0, or 1
    lsl     x0, x0, 8
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_abs;(number: dec64) returns absolution: dec64

; Find the absolute value of a number. If the number is negative, hand it off
; to dec64_neg. Otherwise, return the number unless it is nan or zero.

    tbnz    x0, 63, dec64_neg
    and     x5, x0, 0xFF            ; x5 is the exponent
    subs    xzr, x5, 0x80           ; is it nan?
    b.eq    return_null
    adds    x4, xzr, x0, asr 8      ; is the coefficient zero?
    csel    x0, x0, x4, ne          ; x0 is either the number or zero
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_neg;(number: dec64) returns negation: dec64

; Negate a number.

    sxtb    x11, w0                 ; x11 is the exponent
    subs    xzr, x11, -128
    b.eq    return_null
    sub     x0, xzr, x0, asr 8
    b       new

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_normal;(number: dec64) returns normalization: dec64

; Make the exponent as close to zero as possible without losing any signficance.
; Usually normalization is not needed since it does not materially change the
; value of a number.

; Clobbers: x4 thru x7

    sxtb    x5, w0                  ; x5 is the exponent
    tbnz    x0, 7, normal_micro     ; is the exponent negative?
    cbz     x5, return              ; is the number an integer?
    asr     x0, x0, 8               ; x0 is the coefficient
    cbz     x0, return_zero         ; is the coefficient zero?
    mov     x7, 10

normal_grow

; The exponent is greater than zero. If we subtract 1 from the exponent, we must
; multiply the coefficient by 10.

    mul     x4, x0, x7
    asr     x6, x4, 55
    adds    xzr, x6, x6, asr 63
    b.ne    normal_done
    mov     x0, x4
    subs    x5, x5, 1
    b.gt    normal_grow
    lsl     x0, x0, 8
    ret

normal_micro

; The exponent is negative. Is it nan?

    subs    xzr, x5, -128
    b.eq    return_null
    mov     x7, 10
    asr     x0, x0, 8               ; x0 is the coefficient

normal_shrink

; If we add 1 to the exponent, we must divide the coefficient by 10.

    cbz     x0, return_zero         ; is the coefficient zero?
    sdiv    x4, x0, x7
    mul     x6, x4, x7
    subs    xzr, x6, x0
    b.ne    normal_done
    mov     x0, x4
    adds    x5, x5, 1
    b.mi    normal_shrink

normal_done

    and     x5, x5, 0xFF
    add     x0, x5, x0, lsl 8
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return

; Return whatever is in x0.

    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_null

    mov     x0, 0x8000000000000000
    add     x0, x0, 0x80
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_one

    mov     x0, 256                ; one
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_zero

    mov     x0, xzr                 ; zero
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_equal;(comparahend: dec64, comparator: dec64) returns comparison: dec64

; Compare two dec64 numbers.

    subs    xzr, x0, x1             ; this is the trivial case
    b.eq    return_true

; If the comparator is nan, then we pass the problem to dec64_is_nan.

    and     x7, x1, 0xFF            ; x7 is the second exponent
    subs    xzr, x7, 0x80
    b.eq    dec64_is_nan

; If the exponents are equal, then the numbers are not.
; If the signs are not equal, then the numbers are not.

    eor     x4, x0, x1
    asr     x4, x4, 63              ; x4 is 0 if the signs match
    and     x5, x0, 0xFF            ; x5 is the first exponent
    subs    xzr, x5, x7             ; do the exponents match?
    csinv   x5, x4, xzr, ne
    cbnz    x5, return_false

; Do it the hard way.

    mov     x15, x30
    adr     x30, is_equal_subtract_return
    b       dec64_subtract

is_equal_subtract_return

    mov     x30, x15
    cbz     x0, return_true
    b       return_false

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_false;(boolean: dec64) returns notation: dec64

; If the boolean is false, the result is true. Otherwise the result is false.

    mov     x4, 0x8000000000000000  ; the most significant bit
    add     x4, x4, 0x280           ; x4 is false
    add     x5, x4, 0x100           ; x5 is true
    subs    xzr, x0, x4
    csel    x0, x4, x5, ne
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_integer;(number: dec64) returns comparison: dec64

; If the number contains a non-zero fractional part or if it is nan, return
; false. Otherwise, return true.

    tbz     x0, 7, return_true
    sxtb    x5, w0                  ; x5 is the exponent
    sub     x5, xzr, x5
    subs    xzr, x5, 17
    b.gt    return_false
    adr     x7, power
    ldr     x7, [x7, x5, lsl 3]
    asr     x0, x0, 8
    sdiv    x4, x0, x7
    mul     x4, x4, x7
    subs    xzr, x0, x4
    b.eq    return_true
    b       return_false

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_less;(comparahend: dec64, comparator: dec64) returns comparison: dec64

; Compare two dec64 numbers. If the first is less than the second, return true,
; otherwise return false.
; Clobbers: x4 thru x11

; The other 3 comparison functions are easily implemented with dec64_is_less:

;    dec64_is_greater(a, b)          => dec64_is_less(b, a)
;    dec64_is_greater_or_equal(a, b) => dec64_is_false(dec64_is_less(a, b))
;    dec64_is_less_or_equal(a, b)    => dec64_is_false(dec64_is_less(b, a))

; If the exponents are the same, then do a simple compare. If the exponents
; are both nan, then the result must be false because nan is not less than nan.

    sxtb    x5, w0                  ; x5 is the first exponent
    sxtb    x7, w1                  ; x7 is the second exponent
    eor     x10, x0, x1             ; x10 is negative if the signs mismatch
    tbnz    x10, 63, less_sign
    subs    x9, x5, x7              ; x9 is the exponent difference
    b.ne    less_hard

    subs    xzr, x0, x1             ; compare the numbers
    b.ge    return_false

    subs    xzr, x5, -128           ; is the first argument nan?
    b.ne    return_true
    b       return_false            ; nan is never less than nan

less_sign

    subs    xzr, x5, -128           ; is the first argument nan?
    b.eq    return_false
    subs    xzr, x7, -128           ; is the second argument nan?
    b.eq    return_true
    tbnz    x0, 63, return_true
    b       return_false

less_hard

    subs    xzr, x5, -128           ; is the first argument nan?
    b.eq    return_false
    subs    xzr, x7, -128           ; is the second argument nan?
    b.eq    return_true
    asr     x6, x1, 8               ; x6 is the second coefficient
    cbz     x6, return_false
    adds    x4, xzr, x0, asr 8      ; x4 is the first coefficient
    b.eq    return_true
    cneg    x4, x4, mi              ; x4 is abs(first coefficient)
    cneg    x6, x6, mi              ; x6 is abs(second coefficient)
    mov     x0, 0x8000000000000000
    add     x0, x0, 0x280           ; x0 is false
    lsr     x8, x1, 63              ; x8 is 1 if numbers are negative
    eor     x0, x0, x8, lsl 8       ; flip x0 if the inputs are negative

; Swap and flip if the difference of the exponents is negative.

    ands    xzr, x9, x9
    b.pl    less_hard_scale
    mov     x11, x4
    mov     x4, x6
    mov     x6, x11
    sub     x9, xzr, x9
    eor     x0, x0, 0x100           ; flip x0

less_hard_scale

; The maximum possible coefficient is 36028797018963967. 10**18 is more than
; that. If the exponential difference is large, then the coefficients do not
; matter.

    subs    xzr, x9, 18
    b.ge    return

; Amplify the first coefficient. This will make their exponents the same,
; allowing a simple comparison.

    adr     x10, power
    ldr     x10, [x10, x9 lsl 3]    ; x10 is a power of ten
    umulh   x8, x4, x10             ; x8 is the high extension of the product
    cbnz    x8, return              ; if it overflowed, it can not be less
    mul     x4, x4, x10             ; x4 is the scaled coefficient
    subs    xzr, x4, x6             ; now we can compare
    b.eq    return_false
    b.hi    return
    eor     x0, x0, 0x100           ; flip one more time
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_nan;(number: dec64) returns comparison: dec64

    and     x5, x0, 0xFF
    subs    xzr, x5, 0x80
    cset    x5, eq
    mov     x0, 0x8000000000000000
    add     x0, x0, 0x280           ; x0 is false
    add     x0, x0, x5, lsl 8
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_zero;(number: dec64) returns comparison: dec64

    ands    x4, x0, 0xFFFFFFFFFFFFFF00
    cset    x4, eq
    and     x5, x0, 0xFF
    subs    xzr, x5, 0x80
    csel    x4, xzr, x4, eq
    mov     x0, 0x8000000000000000
    add     x0, x0, 0x280           ; x0 is false
    add     x0, x0, x4, lsl 8
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_false

    mov     x0, 0x8000000000000000
    add     x0, x0, 0x280           ; x0 is false
    ret

; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_true

    mov     x0, 0x8000000000000000
    add     x0, x0, 0x380           ; x0 is true
    ret

    end
