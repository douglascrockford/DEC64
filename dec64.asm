title   dec64.asm for x64.

; dec64.com
; 2016-02-26
; Public Domain

; No warranty expressed or implied. Use at your own risk. You have been warned.

; This file implements the elementary arithmetic operations for DEC64, a decimal
; floating point type. DEC64 uses 64 bits to represent a number. The low order
; 8 bits contain an exponent that ranges from -127 to 127. The exponent is not
; biased. The exponent -128 is reserved for nan (not a number). The remaining
; 56 bits, including the sign bit, are the coefficient in the range
; -36028797018963968 thru 36028797018963967. The exponent and the coefficient
; are both two's complement signed numbers.
;
; The value of any non-nan DEC64 number is coefficient * (10 ** exponent).
;
; Rounding is to the nearest value. Ties are rounded away from zero. Integer
; division is floored. The result of modulo has the sign of the divisor.
; There is no negative zero.
;
; These operations will produce a result of nan:
;
;   dec64_abs(nan)
;   dec64_ceiling(nan)
;   dec64_dec(nan)
;   dec64_floor(nan)
;   dec64_half(nan)
;   dec64_inc(nan)
;   dec64_int(nan)
;   dec64_neg(nan)
;   dec64_normal(nan)
;   dec64_not(nan)
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
; These operations will produce a result of nan for all values of n
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
; These operations will produce a result of nan for all values of n:
;
;   dec64_add(n, nan)
;   dec64_add(nan, n)
;   dec64_divide(nan, n)
;   dec64_integer_divide(nan, n)
;   dec64_less(n, nan)
;   dec64_less(nan, n)
;   dec64_modulo(nan, n)
;   dec64_round(n, nan)
;   dec64_round(nan, n)
;   dec64_subtract(n, nan)
;   dec64_subtract(nan, n)
;
; This file can be processed with Microsoft's ML64.exe. There might be other
; assemblers that can process this file, but that has not been tested.
;
;   You know what goes great with those defective pentium chips?
;   Defective pentium salsa! (David Letterman)

; All public symbols have a dec64_ prefix. All other symbols are private.

; When these functions return nan, they will always return 0x80, the normal
; nan.

; There are 72057594037927936 possible nan values. Two are reserved to
; represent true and false. The comparison functions will return a boolean
; encoded as a DEC64 nan. Be careful when using these comparisons with C
; because C thinks that the DEC64 false is truthy.

true    equ 380h
false   equ 280h

; Sometimes multiplication by a magic number can be faster than and as
; accurate as division.

eight_over_ten equ -3689348814741910323

public dec64_abs;(number: dec64)
;   returns absolution: dec64

public dec64_add;(augend: dec64, addend: dec64)
;   returns sum: dec64

public dec64_ceiling;(number: dec64)
;   returns integer: dec64

public dec64_coefficient;(number: dec64)
;   returns coefficient: int64

public dec64_dec;(minuend: dec64)
;   returns decrementation: dec64

public dec64_divide;(dividend: dec64, divisor: dec64)
;   returns quotient: dec64

public dec64_equal;(comparahend: dec64, comparator: dec64)
;   returns comparison: dec64

public dec64_exponent;(number: dec64)
;   returns exponent: int64

public dec64_floor;(number: dec64)
;   returns integer: dec64

public dec64_half;(dividend: dec64)
;   returns quotient: dec64

public dec64_inc;(augend: dec64)
;   returns incrementation: dec64

public dec64_int;(number: dec64)
;   returns integer: dec64

public dec64_integer_divide;(dividend: dec64, divisor: dec64)
;   returns quotient: dec64

public dec64_is_any_nan;(number: dec64)
;   returns comparison: dec64

public dec64_is_integer;(number: dec64)
;   returns comparison: dec64

public dec64_is_zero;(number: dec64)
;   returns comparison: dec64

public dec64_less;(comparahend: dec64, comparator: dec64)
;   returns comparison: dec64

public dec64_modulo;(dividend: dec64, divisor: dec64)
;   returns modulus: dec64

public dec64_multiply;(multiplicand: dec64, multiplier: dec64)
;   returns product: dec64

public dec64_neg;(number: dec64)
;   returns negation: dec64

public dec64_new;(coefficient: int64, exponent: int64)
;   returns number: dec64

public dec64_normal;(number: dec64)
;   returns normalization: dec64

public dec64_not;(boolean: dec64)
;   returns notation: dec64

public dec64_round;(number: dec64, place: dec64)
;   returns quantization: dec64

public dec64_signum;(number: dec64)
;   returns signature: dec64

public dec64_subtract;(minuend: dec64, subtrahend: dec64)
;   returns difference: dec64

;  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

; Repair the register names. Over the long and twisted evolution of x86, the
; register names have picked up some weird, inconsistent conventions. We can
; simplify, naming them r0 thru r15, with a _b suffix to indicate the low
; order byte, an _h suffix to indicate the so-called high byte and a _w suffix
; to indicate the lower 16 bits. We would use _d to indicate the lower 32 bits,
; but that was not needed here.

r0      equ rax
r1      equ rcx
r2      equ rdx
r6      equ rsi
r7      equ rdi

r0_b    equ al
r1_b    equ cl
r2_b    equ dl
r8_b    equ r8b
r9_b    equ r9b
r10_b   equ r10b
r11_b   equ r11b

r0_h    equ ah
r1_h    equ ch
r2_h    equ dh

r0_w    equ ax
r1_w    equ cx

; All of the public functions in this file accept up to two arguments, which
; are passed in registers (either r1, r2 or r7, r6), returning a result in r0.

; Registers r1, r2, r8, r9, r10, and r11 are clobbered. Register r0 is the
; return value. The other registers are not disturbed.

; There is painfully inadequate standardization around x64 calling conventions.
; On Win64, the first two arguments are passed in r1 and r2. On Unix, the first
; two arguments are passed in r7 and r6. We try to hide this behind macros. The
; two systems also have different conventions about which registers may be
; clobbered and which must be preserved. This code lives in the intersection.

; This has not yet been tested on Unix.

UNIX    equ 0                   ; calling convention: 0 for Windows, 1 for Unix

function_with_one_parameter macro
    if UNIX
    mov     r1,r7               ;; UNIX
    endif
    endm

function_with_two_parameters macro
    if UNIX
    mov     r1,r7               ;; UNIX
    mov     r2,r6               ;; UNIX
    endif
    endm

call_with_one_parameter macro function
    if UNIX
    mov     r7,r1               ;; UNIX
    endif
    call    function
    endm

call_with_two_parameters macro function
    if UNIX
    mov     r7,r1               ;; UNIX
    mov     r6,r2               ;; UNIX
    endif
    call    function
    endm

tail_with_one_parameter macro function
    if UNIX
    mov     r7,r1               ;; UNIX
    endif
    jmp     function
    endm

tail_with_two_parameters macro function
    if UNIX
    mov     r7,r1               ;; UNIX
    mov     r6,r2               ;; UNIX
    endif
    jmp     function
    endm

; There may be a performance benefit in padding programs so that most jump
; destinations are aligned on 16 byte boundaries.

pad macro
    align   16
    endm

;  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_data segment para read

power:                              ; the powers of 10

    qword   1                       ; 0
    qword   10                      ; 1
    qword   100                     ; 2
    qword   1000                    ; 3
    qword   10000                   ; 4
    qword   100000                  ; 5
    qword   1000000                 ; 6
    qword   10000000                ; 7
    qword   100000000               ; 8
    qword   1000000000              ; 9
    qword   10000000000             ; 10
    qword   100000000000            ; 11
    qword   1000000000000           ; 12
    qword   10000000000000          ; 13
    qword   100000000000000         ; 14
    qword   1000000000000000        ; 15
    qword   10000000000000000       ; 16
    qword   100000000000000000      ; 17
    qword   1000000000000000000     ; 18
    qword   10000000000000000000    ; 19

dec64_data ends

;  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_code segment para execute

dec64_coefficient: function_with_one_parameter
;(number: dec64) returns coefficient: int64

; Return the coefficient part from a dec64 number.

    mov     r0,r1
    sar     r0,8
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_exponent: function_with_one_parameter
;(number: dec64) returns exponent: int64

; Return the exponent part, sign extended to 64 bits, from a dec64 number.
; dec64_exponent(nan) returns -128.

    movsx   r0,r1_b             ; r0 is the exponent
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_new: function_with_two_parameters
;(coefficient: int64, exponent: int64) returns number: dec64

; Construct a new dec64 number with a coefficient and an exponent.

    mov     r0,r1               ; r0 is the coefficient
    test    r0,r0               ; is the coefficient zero?
    jz      return_zero
    mov     r8,r2               ; r8 is the exponent

; Fall into pack.

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

pack:

; The pack function will combine the coefficient and exponent into a dec64.
; Numbers that are too huge to be contained in this format become nan.
; Numbers that are too tiny to be contained in this format become zero.

; The coefficient is in r0.
; The exponent is in r8.

; If the exponent is greater than 127, then the number is too big.
; But it might still be possible to salvage a value.

    cmp     r8,127          ; compare exponent with 127
    jg      pack_decrease   ; it might be possible to decrease it

; If the exponent is too small, or if the coefficient is too large, then some
; division is necessary. The absolute value of the coefficient is off by one
; for the negative because
;    negative_extreme_coefficent = -(extreme_coefficent + 1)

    mov     r10,r0          ; r10 is the coefficient
    mov     r1,3602879701896396800 ; the ultimate coefficient * 100
    not     r10             ; r10 is -coefficient
    xor     r11,r11         ; r11 is zero
    test    r10,r10         ; look at the sign bit
    cmovs   r10,r0          ; r10 is the absolute value of the coefficient
    cmp     r10,r1          ; compare with the actual coefficient
    jae     pack_large      ; deal with the very large coefficient
    mov     r1,36028797018963967 ; the ultimate coefficient - 1
    mov     r9,-127         ; r9 is the ultimate exponent
    cmp     r1,r10          ; compare with the actual coefficient
    adc     r11,0           ; add 1 to r11 if 1 digit too big
    mov     r1,360287970189639679 ; the ultimate coefficient * 10 - 1
    sub     r9,r8           ; r9 is the difference from the actual exponent
    cmp     r1,r10          ; compare with the actual coefficient
    adc     r11,0           ; add 1 to r11 if 2 digits too big
    cmp     r9,r11          ; which excess is larger?
    cmovl   r9,r11          ; take the max
    test    r9,r9           ; if neither was zero
    jnz     pack_increase   ; then increase the exponent by the excess
    shl     r0,8            ; shift the exponent into position
    cmovz   r8,r0           ; if the coefficient is zero, also zero the exp
    movzx   r8,r8_b         ; zero out all but the bottom 8 bits of the exp
    or      r0,r8           ; mix in the exponent
    ret                     ; whew
    pad

pack_large:

    mov     r1,r0           ; r1 is the coefficient
    sar     r1,63           ; r1 is -1 if negative, or 0 if positive
    mov     r11,eight_over_ten ; magic number
    mov     r9,r1           ; r9 is -1 or 0
    xor     r0,r1           ; complement the coefficient if negative
    and     r9,1            ; r9 is 1 or 0
    add     r0,r9           ; r0 is absolute value of coefficient
    add     r8,1            ; add 1 to the exponent
    mul     r11             ; multiply abs(coefficient) by magic number
    mov     r0,r2           ; r0 is the product shift 64 bits
    shr     r0,3            ; r0 is divided by 8: the abs(coefficient) / 10
    xor     r0,r1           ; complement the coefficient if it was negative
    add     r0,r9           ; coefficient's sign is restored
    jmp     pack            ; start over
    pad

pack_increase:

    mov     r10,power
    mov     r10,[r10][r9*8] ; r10 is 10^r9
    cmp     r9,20           ; is the difference more than 20?
    jae     return_zero     ; if so, the result is zero (rare)
    mov     r11,r10         ; r11 is the power of ten
    neg     r11             ; r11 is the negation of the power of ten
    test    r0,r0           ; examine the sign of the coefficient
    cmovns  r11,r10         ; r11 has the sign of the coefficient
    sar     r11,1           ; r11 is half the signed power of ten
    add     r0,r11          ; r0 is the coefficient plus the rounding fudge
    cqo                     ; sign extend r0 into r2
    idiv    r10             ; divide by the power of ten
    add     r8,r9           ; increase the exponent
    jmp     pack            ; start over
    pad

pack_decrease:

; The exponent is too big. We can attempt to reduce it by scaling back.
; This can salvage values in a small set of cases.

    imul    r0,10           ; try multiplying the coefficient by 10
    jo      return_nan      ; if it overflows, we failed to salvage
    sub     r8,1            ; decrement the exponent
    test    r8,-128         ; is the exponent still over 127?
    jnz     pack_decrease   ; until the exponent is less than 127
    mov     r9,r0           ; r9 is the coefficient
    sar     r9,56           ; r9 is top 8 bits of the coefficient
    adc     r9,0            ; add the ninth bit
    jnz     return_nan      ; the number is still too large
    shl     r0,8            ; shift the exponent into position
    cmovz   r8,r0           ; if the coefficient is zero, also zero the exponent
    movzx   r8,r8_b         ; zero out all but the bottom 8 bits of the exponent
    or      r0,r8           ; mix in the exponent
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_round: function_with_two_parameters
;(number: dec64, place: dec64) returns quantization: dec64

; The place argument indicates at what decimal place to round.
;    -2        nearest cent
;     0        nearest integer
;     3        nearest thousand
;     6        nearest million
;     9        nearest billion

; The place should be between -16 and 16.

    mov     r11,r1          ; preserve the number
    mov     r1,r2           ; pass the place
    call_with_one_parameter dec64_int
    mov     r9,r0           ; r9 is the normalized place
    cmp     r11_b,128       ; is the first operand not nan?
    setne   r1_b
    sar     r9,8            ; r9 is the place as int64
    cmp     r0_b,128        ; is the second operand nan?
    setne   r1_h
    movsx   r8,r11_b        ; r8 is the current exponent
    mov     r0,r11          ; r0 is the number
    test    r1_b,r1_h       ; is either nan?
    jz      return_nan      ; if so, the result is nan

    sar     r0,8            ; r0 is the coefficient
    jz      return          ; if the coefficient is zero, the result is zero
    cmp     r8,r9           ; compare the exponents
    jge     pack            ; no rounding required
    mov     r2,r0           ; r2 is the coefficient
    neg     r2              ; rs is the coefficient negated
    cmovns  r0,r2           ; r0 is absolute value of the coefficient
    mov     r10,eight_over_ten ; magic
    pad

round_loop:

; Increment the exponent and divide the coefficient by 10 until the target
; exponent is reached.

    mul     r10             ; r2 is the coefficient * 8 / 10
    mov     r0,r2           ; r0 is the coefficient * 8 / 10
    shr     r0,3            ; r0 is the coefficient / 10

    add     r8,1            ; increment the exponent
    cmp     r8,r9           ; compare the exponents
    jne     round_loop      ; loop if the exponent has not reached the target

; Round if necessary and return the result.

    shr     r2,2            ; Isolate the carry bit
    and     r2,1            ; r2 is 1 if rounding is needed
    add     r0,r2           ; r0 is rounded
    mov     r2,r0           ; r2 is the result
    neg     r2              ; r2 is the result negated
    test    r11,r11         ; was the original number negative
    cmovs   r0,r2           ; if so, use the negated result
    jmp     pack            ; pack it up

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_add: function_with_two_parameters
;(augend: dec64, addend: dec64) returns sum: dec64

; Add two dec64 numbers together.

; Registers:
; r0        result, imul/idiv
; r1        first argument
; r2        second argument, imul/idiv
; r8        exponent of first argument
; r9        exponent of second argument
; r10       coefficient of first argument
; r11       coefficient of second argument

; If the two exponents are both zero (which is usually the case for integers)
; we can take the fast path. Since the exponents are both zero, we can simply
; add the numbers together and check for overflow.

    pad

add_begin:

    mov     r0,r1           ; r0 is the first number
    or      r1_b,r2_b       ; r1_b is the two exponents or'd together
    jnz     add_slow        ; if either exponent is not zero, take the slow path
    add     r0,r2           ; add the shifted coefficients together
    jo      add_overflow    ; if there was no overflow, we are done
    ret                     ; no need to pack
    pad

add_overflow:

; If there was an overflow (extremely unlikely) then we must make it fit.
; pack knows how to do that.

    rcr     r0,1            ; divide the sum by 2 and repair its sign
    movsx   r8,r1_b         ; r8 is the exponent
    sar     r0,7            ; r0 is the coefficient of the sum
    jmp     pack            ; pack it up
    pad

add_slow:

; The slow path is taken if the two operands do not both have zero exponents.

    mov     r1,r0           ; restore r1
    cmp     r0_b,128        ; is the first operand nan?
    je      return_nan      ; if nan, get out

; Are the two exponents the same? This will happen often, especially with
; money values.

    cmp     r1_b,r2_b       ; compare the two exponents
    jne     add_slower      ; if not equal, take the slower path

; The exponents match so we may add now. Zero out the exponents so there
; will be no carry into the coefficients when the coefficients are added.
; If the result is zero, then return the normal zero.

    and     r0,-256         ; remove the exponent
    and     r2,-256         ; remove the other exponent
    add     r0,r2           ; add the shifted coefficients
    jo      add_overflow    ; if it overflows, it must be repaired
    cmovz   r1,r0           ; if the coefficient is zero, the exponent is zero
    movzx   r1,r1_b         ; mask the exponent
    or      r0,r1           ; mix in the exponent
    ret                     ; no need to pack
    pad

add_slower:

; The slower path is taken when neither operand is nan, and their
; exponents are different. Before addition can take place, the exponents
; must be made to match. Swap the numbers if the second exponent is greater
; than the first.

    cmp     r2_b,128        ; Is the second operand nan?
    je      return_nan
    cmp     r1_b,r2_b       ; compare the exponents
    mov     r0,r1           ; r0 is the first number
    cmovl   r1,r2           ; r1 is the number with the larger exponent
    cmovl   r2,r0           ; r2 is the number with the smaller exponent

; Shift the coefficients of r1 and r2 into r10 and r11 and unpack the exponents.

    mov     r10,r1          ; r10 is the first number
    mov     r11,r2          ; r11 is the second number
    movsx   r8,r1_b         ; r8 is the first exponent
    movsx   r9,r2_b         ; r9 is the second exponent
    sar     r10,8           ; r10 is the first coefficient
    sar     r11,8           ; r11 is the second coefficient
    mov     r0,r10          ; r0 is the first coefficient
    pad

add_slower_decrease:

; The coefficients are not the same. Before we can add, they must be the same.
; We will try to decrease the first exponent. When we decrease the exponent
; by 1, we must also multiply the coefficient by 10. We can do this as long as
; there is no overflow. We have 8 extra bits to work with, so we can do this
; at least twice, possibly more.

    imul    r0,10           ; before decrementing the exponent, multiply
    jo      add_slower_increase
    sub     r8,1            ; decrease the first exponent
    mov     r10,r0          ; r10 is the enlarged first coefficient
    cmp     r8,r9           ; are the exponents equal yet?
    jg      add_slower_decrease
    mov     r0,r11          ; r0 is the second coefficient

; The exponents are now equal, so the coefficients may be added.

    add     r0,r10          ; add the two coefficients
    jmp     pack            ; pack it up
    pad

add_slower_increase:

; We cannot decrease the first exponent any more, so we must instead try to
; increase the second exponent, which will result in a loss of significance.
; That is the heartbreak of floating point.

; Determine how many places need to be shifted. If it is more than 17, there is
; nothing more to add.

    mov     r2,r8           ; r2 is the first exponent
    sub     r2,r9           ; r2 is the remaining exponent difference
    mov     r0,r11          ; r0 is the second coefficient
    cmp     r2,17           ; 17 is the max digits in a packed coefficient
    ja      return_r1       ; too small to matter
    mov     r9,power
    mov     r9,[r9][r2*8]   ; r9 is the power of ten
    cqo                     ; sign extend r0 into r2
    idiv    r9              ; divide the second coefficient by the power of 10
    test    r0,r0           ; examine the scaled coefficient
    jz      return_r1       ; too insignificant to add?

; The exponents are now equal, so the coefficients may be added.

    add     r0,r10          ; add the two coefficients
    jmp     pack
    pad

return_r1:

    mov     r0,r1           ; r0 is the original number
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_inc: function_with_one_parameter
;(augend: dec64) returns incrementation: dec64

; Increment a number. In most cases, this will be a faster way to add one than
; dec64_add.

    test    r1_b,r1_b       ; what is the exponent?
    jnz     inc_not_integer

; The number is an integer. This might be easy.

    mov     r0,100h         ; r0 is one
    add     r0,r1           ; r0 is the incrementation
    jo      inc_hardway     ; overflow (very rare)
    ret
    pad

inc_not_integer:

    js      inc_negative_exponent

    test    r1,-256         ; is the coefficient zero?
    jz      return_one      ; if so, the result is one
    cmp     r1_b,17         ; is the number too enormous to increment?
    jge     return_r1       ; then return the number

inc_hardway:

    mov     r2,100h         ; r2 is one
    tail_with_two_parameters dec64_add
    pad

inc_negative_exponent:

    cmp     r1_b,128        ; is the number nan?
    je      return_nan
    cmp     r1_b,-17        ; is the number too small to increment?
    jle     return_one      ; then return one

    movsx   r8,r1_b         ; r8 is the negative exponent
    neg     r8              ; flip the sign
    mov     r9,power
    mov     r0,[r9][r8*8]   ; r0 is 10^(-exponent)
    shl     r0,8            ; convert to dec64
    add     r0,r1           ; now we add
    jo      inc_hardway     ; if it overflows, do it the hard way
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_dec: function_with_one_parameter
;(minuend: dec64) returns decrementation: dec64

; Increment a number. In most cases, this will be a faster way to subtract one
; than dec64_subtract.

    test    r1_b,r1_b       ; what is the exponent?
    jnz     dec_not_integer


; The number is an integer. This might be easy.

    mov     r0,-256         ; r0 is negative one
    add     r0,r1           ; r0 is the decrementation
    jo      dec_hardway     ; overflow (very rare)
    ret
    pad

dec_not_integer:

    js      dec_negative_exponent

    test    r1,-256         ; is the coefficient zero?
    jz      dec_neg_one     ; if so, the result is negative one
    cmp     r1_b,17         ; is the number too enormous to decrement?
    jge     return_r1       ; then return the number

dec_hardway:

    mov     r2,100h         ; r2 is one
    tail_with_two_parameters dec64_subtract
    pad

dec_negative_exponent:

    cmp     r1_b,128        ; is the number nan?
    je      return_nan
    cmp     r1_b,-17        ; is the number too small to decrement?
    jle     dec_neg_one     ; then return negative one

    movsx   r8,r1_b         ; r8 is the negative exponent
    neg     r8              ; flip the sign
    mov     r9,power
    mov     r0,[r9][r8*8]   ; r0 is 10^(-exponent)
    neg     r0              ; go negative
    shl     r0,8            ; convert to dec64
    add     r0,r1           ; now we subtract
    jo      dec_hardway     ; if it overflows, do it the hard way
    ret
    pad

dec_neg_one:

    mov     r0,-256         ; r0 is -1
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_ceiling: function_with_one_parameter
;(number: dec64) returns integer: dec64

; Produce the smallest integer that is greater than or equal to the number. In
; the result, the exponent will be greater than or equal to zero unless it is
; nan. Numbers with positive exponents will not be modified, even if the
; numbers are outside of the safe integer range.

; Preserved: r11.

    mov     r9,1            ; r9 is the round up flag
    jmp     floor_begin

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_floor: function_with_one_parameter
;(number: dec64) returns integer: dec64

; Produce the largest integer that is less than or equal to the number. This
; is sometimes called the entier function. In the result, the exponent will be
; greater than or equal to zero unless it is nan. Numbers with positive
; exponents will not be modified, even if the numbers are outside of the safe
; integer range.

; Preserved: r11.

    mov     r9,-1           ; r9 is the round down flag
    pad

floor_begin:

    cmp     r1_b,128        ; compare the exponent to nan
    je      return_nan       ; if the exponent is nan, the result is nan
    mov     r0,r1           ; r0 is the number
    movsx   r8,r1_b         ; r8 is the exponent
    sar     r0,8            ; r0 is the coefficient
    cmovz   r1,r0           ; if the coefficient is zero, the number is zero
    neg     r8              ; r8 is the negated exponent
    test    r1_b,r1_b       ; examine the exponent
    jns     return_r1       ; nothing to do unless the exponent was negative
    cmp     r8,17           ; is the exponent is too extreme?
    jae     floor_micro     ; deal with a micro number
    mov     r10,power
    mov     r10,[r10][r8*8] ; r10 is the power of ten
    cqo                     ; sign extend r0 into r2
    idiv    r10             ; divide r2:r0 by 10
    test    r2,r2           ; examine the remainder
    jnz     floor_remains   ; deal with the remainder
    shl     r0,8            ; pack the coefficient
    ret
    pad

floor_micro:

    mov     r2,r0           ; r2 is the coefficient
    xor     r0,r0           ; r0 is zero
    pad

floor_remains:

; If the remainder is negative and the rounding flag is negative, then we need
; to decrement r0. But if the remainder and the rounding flag are both
; positive, then we need to increment r0.

    xor     r10,r10         ; r10 is zero
    xor     r2,r9           ; xor the remainder and the rounding
    cmovs   r9,r10          ; if they had different signs, clear the rounding
    add     r0,r9           ; add the rounding to the coefficient
    shl     r0,8            ; pack the coefficient
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_subtract: function_with_two_parameters
;(minuend: dec64, subtrahend: dec64) returns difference: dec64

; Subtract the dec64 number in r2 from the dec64 number in r1.
; The result is in r0.

; This is the same as dec64_add, except that the operand in r2 has its
; coefficient complemented first.

    xor     r2,-256         ; not the coefficient only
    add     r2,256          ; the two's complement increment of the coefficient
    jno     add_begin       ; if there is no overflow, begin the beguine

; The subtrahend coefficient is -36028797018963968. This value cannot easily be
; complemented, so take the slower path. This should be extremely rare.

    cmp     r1_b,128        ; is the first operand nan
    sete    r0_b
    cmp     r2_b,128        ; is the second operand nan?
    sete    r0_h
    or      r0_b,r0_h       ; is either nan?
    jnz     return_nan
    mov     r10,r1          ; r10 is the first coefficient
    movsx   r8,r1_b         ; r8 is the first exponent
    sar     r10,8
    movsx   r9,r2_b         ; r9 is the second exponent
    mov     r11,80000000000000H ; r11 is 36028797018963968
    mov     r0,r10          ; r0 is the first coefficient
    cmp     r8,r9           ; if the second exponent is larger, swap
    jge     subtract_slower_decrease_compare
    mov     r0,r11          ; r0 is the second coefficient
    xchg    r8,r9           ; swap the exponents
    xchg    r10,r11         ; swap the coefficients
    jmp     subtract_slower_decrease_compare
    pad

subtract_slower_decrease:

; The coefficients are not the same. Before we can add, they must be the same.
; We will try to decrease the first exponent. When we decrease the exponent
; by 1, we must also multiply the coefficient by 10. We can do this as long as
; there is no overflow. We have 8 extra bits to work with, so we can do this
; at least twice, possibly more.

    imul    r0,10           ; before decrementing the exponent, multiply
    jo      subtract_slower_increase
    sub     r8,1            ; decrease the first exponent
    mov     r10,r0          ; r10 is the enlarged first coefficient
    pad

subtract_slower_decrease_compare:

    cmp     r8,r9           ; are the exponents equal yet?
    jg      subtract_slower_decrease
    mov     r0,r11          ; r0 is the second coefficient

; The exponents are now equal, so the coefficients may be added.

    add     r0,r10          ; add the two coefficients
    jmp     pack            ; pack it up
    pad

subtract_slower_increase:

; We cannot decrease the first exponent any more, so we must instead try to
; increase the second exponent, which will result in a loss of significance.
; That is the heartbreak of floating point.

; Determine how many places need to be shifted. If it is more than 17, there is
; nothing more to add.

    mov     r2,r8           ; r2 is the first exponent
    sub     r2,r9           ; r2 is the remaining exponent difference
    mov     r0,r11          ; r0 is the second coefficient
    cmp     r2,17           ; 17 is the max digits in a packed coefficient
    ja      subtract_underflow ; too small to matter
    mov     r9,power
    mov     r9,[r9][r2*8]   ; r9 is the power of ten
    cqo                     ; sign extend r0 into r2
    idiv    r9              ; divide the second coefficient by the power of 10

; The exponents are now equal, so the coefficients may be added.

    add     r0,r10          ; add the two coefficients
    jmp     pack
    pad

subtract_underflow:

    mov     r0,r10          ; r0 is the first coefficient
    jmp     pack

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_multiply: function_with_two_parameters
;(multiplicand: dec64, multiplier: dec64) returns product: dec64

; Multiply two dec64 numbers together.

; Unpack the exponents into r8 and r9.

    movsx   r8,r1_b         ; r8 is the first exponent
    movsx   r9,r2_b         ; r9 is the second exponent

; Set flags in r0 if either operand is nan.

    cmp     r1_b,128        ; is the first operand nan?
    sete    r0_b            ; r0_b is 1 if the first operand is nan
    cmp     r2_b,128        ; is the second operand nan?
    sete    r0_h            ; r0_h is 1 if the second operand is nan

; Unpack the coefficients. Set flags in r1 if either is not zero.

    sar     r1,8            ; r1 is the first coefficient
    mov     r10,r1          ; r10 is the first coefficient
    setnz   r1_b            ; r1_b is 1 if the first coefficient is not zero
    sar     r2,8            ; r2 is the second coefficient
    setnz   r1_h            ; r1_h is 1 if the second coefficient is not zero

; The result is nan if one or both of the operands is nan and neither of the
; operands is zero.

    or      r1_w,r0_w       ; is either coefficient zero and not nan?
    xchg    r1_b,r1_h
    test    r0_w,r1_w
    jnz     return_nan

    mov     r0,r10          ; r0 is the first coefficient
    add     r8,r9           ; r8 is the product exponent
    imul    r2              ; r2:r0 is r1 * r2
    jno     pack            ; if the product fits in 64 bits, start packing

; There was overflow.

; Make the 110 bit coefficient in r2:r0Er8 all fit. Estimate the number of
; digits of excess, and increase the exponent by that many digits.
; We use 77/256 to convert log2 to log10.

    mov     r9,r2           ; r9 is the excess significance
    xor     r1,r1           ; r1 is zero anticipating bsr
    neg     r9
    cmovs   r9,r2           ; r9 is absolute value of the excess significance

    bsr     r1,r9           ; find the position of the most significant bit
    imul    r1,77           ; multiply the bit number by 77/256 to
    shr     r1,8            ;     convert a bit number to a digit number
    add     r1,2            ; add two extra digits to the scale
    add     r8,r1           ; increase the exponent
    mov     r9,power
    idiv    qword ptr [r9][r1*8] ; divide by the power of ten
    jmp     pack

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_divide: function_with_two_parameters
;(dividend: dec64, divisor: dec64) returns quotient: dec64

; Divide a dec64 number by another.

; Begin unpacking the components.

    movsx   r8,r1_b         ; r8 is the first exponent
    movsx   r9,r2_b         ; r9 is the second exponent
    mov     r10,r1          ; r10 is the first number
    mov     r11,r2          ; r11 is the second number

; Set nan flags in r0.

    cmp     r1_b,128        ; is the first operand nan?
    sete    r0_b            ; r0_b is 1 if the first operand is nan
    cmp     r2_b,128        ; is the second operand nan?
    sete    r0_h            ; r0_h is 1 if the second operand is nan

    sar     r10,8           ; r10 is the dividend coefficient
    setnz   r1_b            ; r1_b is 1 if the dividend coefficient is zero
    sar     r11,8           ; r11 is the divisor coefficient
    setz    r1_h            ; r1_h is 1 if dividing by zero
    or      r0_h,r0_b       ; r0_h is 1 if either is nan
    or      r1_b,r0_b       ; r1_b is zero if the dividend is zero and not nan
    jz      return_zero     ; if the dividend is zero, the quotient is zero
    sub     r8,r9           ; r8 is the quotient exponent
    or      r0_b,r1_h       ; r0_b is 1 if either is nan or the divisor is zero
    jnz     return_nan
    pad

divide_measure:

; We want to get as many bits into the quotient as possible in order to capture
; enough significance. But if the quotient has more than 64 bits, then there
; will be a hardware fault. To avoid that, we compare the magnitudes of the
; dividend coefficient and divisor coefficient, and use that to scale the
; dividend to give us a good quotient.

    mov     r0,r10          ; r0 is the first coefficient
    mov     r1,r11          ; r1 is the second coefficient
    neg     r0              ; r0 is negated
    cmovs   r0,r10          ; r0 is abs of dividend coefficient
    neg     r1              ; r1 is negated
    cmovs   r1,r11          ; r1 is abs of divisor coefficient
    bsr     r0,r0           ; r0 is the dividend most significant bit
    bsr     r1,r1           ; r1 is the divisor most significant bit

; Scale up the dividend to be approximately 58 bits longer than the divisor.
; Scaling uses factors of 10, so we must convert from a bit count to a digit
; count by multiplication by 77/256 (approximately LN2/LN10).

    add     r1,58           ; we want approximately 58 bits in the raw quotient
    sub     r1,r0           ; r1 is the number of bits to add to the dividend
    imul    r1,77           ; multiply by 77/256 to convert bits to digits
    shr     r1,8            ; r1 is the number of digits to scale the dividend

; The largest power of 10 that can be held in an int64 is 1e18.

    cmp     r1,18           ; prescale the dividend if 10**r1 won't fit
    jg      divide_prescale

; Multiply the dividend by the scale factor, and divide that 128 bit result by
; the divisor.  Because of the scaling, the quotient is guaranteed to use most
; of the 64 bits in r0, and never more. Reduce the final exponent by the number
; of digits scaled.

    mov     r0,r10          ; r0 is the dividend coefficient
    mov     r9,power
    imul    qword ptr [r9][r1*8] ; r2:r0 is the dividend coefficient * 10**r1
    idiv    r11             ; r0 is the quotient
    sub     r8,r1           ; r8 is the exponent
    jmp     pack            ; pack it up
    pad

divide_prescale:

; If the number of scaling digits is larger than 18, then we will have to
; scale in two steps: first prescaling the dividend to fill a register, and
; then repeating to fill a second register. This happens when the divisor
; coefficient is much larger than the dividend coefficient.

    mov     r1,58           ; we want 58 bits or so in the dividend
    sub     r1,r0           ; r1 is the number of additional bits needed
    imul    r1,77           ; convert bits to digits
    shr     r1,8            ; shift 8 is cheaper than div 256
    mov     r9,power
    imul    r10,qword ptr [r9][r1*8] ; multiply the dividend by power of ten
    sub     r8,r1           ; reduce the exponent
    jmp     divide_measure  ; try again

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_integer_divide: function_with_two_parameters
;(dividend: dec64, divisor: dec64) returns quotient: dec64

; Divide, with a floored integer result. It produces the same result as
;    dec64_floor(dec64_divide(dividend, divisor))
; but can sometimes produce that result more quickly.

    cmp     r1_b,r2_b       ; are the exponents equal?
    jne     integer_divide_slow

    mov     r0,r1           ; r0 is the dividend
    mov     r11,r2          ; r11 is the divisor
    sar     r1,8            ; r1 is the dividend coefficient
    setnz   r2_h            ; r2_h is 1 if the dividend coefficient is not zero
    cmp     r0_b,128        ; are the operands nan?
    sete    r2_b            ; r2_b is 1 if the operands are nan
    and     r0,-256         ; zero the dividend's exponent
    or      r2_h,r2_b       ; r2_h is zero if the dividend is zero and not nan
    jz      return_zero     ; the quotient is zero if the dividend is zero
    sar     r11,8           ; r11 is the divisor coefficient
    setz    r2_h            ; r2_h is 1 if the divisor coefficient is zero
    or      r2_b,r2_h       ; r2_b is 1 if the result is nan
    jnz     return_nan
    cqo                     ; sign extend r0 into r2
    idiv    r11             ; r0 is the quotient
    and     r0,-256         ; zero the exponent again
    ret                     ; no need to pack
    pad

integer_divide_slow:

; The exponents are not the same, so do it the hard way.

    call_with_two_parameters dec64_divide
    mov     r1,r0
    tail_with_one_parameter dec64_floor

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_modulo: function_with_one_parameter
;(dividend: dec64, divisor: dec64) returns modulus: dec64

; Modulo. It produces the same result as
;    dec64_subtract(
;        dividend,
;        dec64_multiply(
;            dec64_integer_divide(dividend, divisor),
;            divisor
;        )
;    )

    cmp     r1_b,r2_b       ; are the two exponents the same?
    jnz     modulo_slow     ; if not take the slow path
    mov     r0,r1           ; r0 is the dividend
    mov     r11,r2          ; r11 is the divisor
    cmp     r1_b,128        ; is the first operand nan?
    setne   r2_h            ; r2_h is 1 if the operands are not nan
    sar     r0,8            ; r0 is the dividend coefficient
    setz    r2_b            ; r2_b is 1 if the dividend coefficient is zero
    test    r2_b,r2_h       ; is the dividend is zero and not nan?
    jnz     return_zero     ; the quotient is zero if the dividend is zero
    sar     r11,8           ; r11 is the divisor coefficient
    setnz   r2_b            ; r2_b is 1 if the divisor coefficient is not zero
    test    r2_h,r2_b       ; is either operand nan or is the divisor zero?
    jz      return_nan
    cqo                     ; sign extend r0 into r2
    idiv    r11             ; divide r2:r0 by the divisor

; If the signs of the divisor and remainder are different and the remainder is
; not zero, add the divisor to the remainder.

    xor     r0,r0           ; r0 is zero
    mov     r10,r2          ; r10 is the remainder
    test    r2,r2           ; examine the remainder
    cmovz   r11,r0          ; r11 is zero if the remainder is zero
    xor     r10,r11         ; r10 is remainder xor divisor
    cmovs   r0,r11          ; r0 is the divisor if the signs were different
    add     r0,r2           ; r0 is the corrected result
    cmovz   r1,r0           ; if r0 is zero, so is the exponent
    shl     r0,8            ; position the coefficient
    mov     r0_b,r1_b       ; mix in the exponent
    ret
    pad

modulo_slow:

; The exponents are not the same, so do it the hard way.

    push    r1              ; save the dividend
    push    r2              ; save the divisor

    call_with_two_parameters dec64_integer_divide
    pop     r2
    mov     r1,r0
    call_with_two_parameters dec64_multiply
    pop     r1
    mov     r2,r0
    tail_with_two_parameters dec64_subtract

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_half: function_with_one_parameter
;(dividend: dec64) returns quotient: dec64

; Divide a dec64 number by two. This will always be faster than dec64_divide.

    cmp     r1_b,128
    je      return_nan
    test    r1_h,1
    jz      half_fast

; Unpack the components into r8 and r1, multiply by 5 and divide by 10.

    movsx   r8,r1_b         ; r8 is the exponent
    sar     r1,8            ; r1 is the coefficient
    sub     r8,1            ; bump down the exponent
    lea     r0,[r1+r1*4]    ; r0 is the coefficient * 5
    jmp     pack
    pad

half_fast:

; If the least significant bit of the coefficient is 0, then we can do this
; the fast way. Shift the coefficient by 1 bit and restore the exponent. If
; the shift produces zero, even easier.

    mov     r0,-256         ; r0 is the coefficient mask
    and     r0,r1           ; r0 is the coefficient shifted 8 bits
    jz      return
    sar     r0,1            ; r0 is divided by 2
    movzx   r1,r1_b         ; zero out r1 except lowest 8 bits
    or      r0,r1           ; mix in the exponent
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_signum: function_with_one_parameter
;(number: dec64) returns signature: dec64

; If the number is nan, the result is nan.
; If the number is less than zero, the result is -1.
; If the number is zero, the result is 0.
; If the number is greater than zero, the result is 1.

    mov     r0,r1           ; r0 is the number
    sar     r0,8            ; r0 is the coefficient
    mov     r8,r0           ; r8 is the coefficient too
    neg     r0              ; r0 is -coefficient
    sar     r8,63           ; r8 is the extended sign (-1 if negative else 0)
    shr     r0,63           ; r0 is 1 if the number was greater than 0
    mov     r2,128          ; r2 is nan
    or      r0,r8           ; r0 is -1, 0, or 1
    shl     r0,8            ; package the number with a zero exponent
    cmp     r1_b,r2_b       ; is the number nan?
    cmove   r0,r2           ; if so, replace the answer
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_not: function_with_one_parameter
;(boolean: dec64) returns notation: dec64

; If the number is true, the result is false.
; If the number is false, the result is true.
; Otherwise, the result is nan.

    mov     r0,r1           ; r0 is the argument

; Turn off the bit that distinguishes true from false.

    and     r1,NOT(true XOR false)
    cmp     r1,false        ; if the argument was not a boolean
    jne     return_nan
    xor     r0,true XOR false ; return the not boolean
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_neg: function_with_one_parameter
;(number: dec64) returns negation: dec64

; Negate a number. We need to negate the coefficient without changing the
; exponent.

    cmp     r1_b,128        ; compare the exponent to nan
    je      return_nan      ; is the number nan?
    mov     r2,r1           ; r2 is the number
    mov     r0,-256         ; r0 is the coefficient mask
    sar     r2,8            ; r2 is the coefficient
    cmovz   r1,r2           ; if the coefficient is zero, then the exponent too
    xor     r0,r1           ; r0 has the exponent and complemented coefficient
    add     r0,256          ; r0 has the exponent and negated coefficient
    jo      neg_overflow    ; nice day if it don't overflow
    ret

neg_overflow:

; The coefficient is -36028797018963968, which is the only coefficient that
; cannot be trivially negated. So we do this the hard way.

    mov     r0,r2
    movsx   r8,r1_b         ; r8 is the exponent
    neg     r0
    jmp     pack

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_abs: function_with_one_parameter
;(number: dec64) returns absolution: dec64

; Find the absolute value of a number. If the number is negative, hand it off
; to dec64_neg. Otherwise, return the number unless it is nan or zero.

    test    r1,r1           ; examine r1
    js      dec64_neg       ; is the number negative?
    mov     r10,r1          ; r10 is the number
    mov     r0,r1           ; r0 is the number
    mov     r2,128          ; r2 is nan
    and     r10,-256        ; r10 is the coefficient without an exponent
    cmovz   r0,r10          ; if the coefficient is zero, the number is zero
    cmp     r1_b,r2_b       ; compare r1 to nan
    cmovz   r0,r2           ; is the number nan?
    ret                     ; return the number

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_equal: function_with_two_parameters
;(comparahend: dec64, comparator: dec64) returns comparison: dec64

; Compare two dec64 numbers. If they are equal, return true, otherwise return
; false. Denormal zeroes are equal but denormal nans are not.

; If the numbers are trivally equal, then return true.

    cmp     r1,r2           ; compare the two numbers
    je      return_true

; If the exponents match or if their signs are different, then return false.

    mov     r0,r1           ; r0 is the first number
    xor     r0,r2           ; r0 the xor of the two numbers
    sets    r0_h            ; r0_h is 1 if the signs are different
    cmp     r1_b,r2_b       ; compare the two exponents
    sete    r0_b            ; r0_b is 1 if the exponents are the same
    or      r0_b,r0_h
    jnz     return_false

; Do it the hard way by subtraction. Is the difference zero?

    call_with_two_parameters dec64_subtract ; r0 is r1 - r2
    cmp     r0_b,128        ; is the difference nan?
    je      return_false
    or      r0,r0           ; examine the difference
    setz    r0_b            ; r0 is 1 if the numbers are equal
    movzx   r0,r0_b
    shl     r0,8
    add     r0,false
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_less: function_with_two_parameters
;(comparahend: dec64, comparator: dec64) returns comparison: dec64

; Compare two dec64 numbers. If either argument is any nan, then the result is
; nan. If the first is less than the second, return true, otherwise return
; false.

; The other 3 comparison functions are easily implemented with dec64_less:

;    dec64_greater(a, b)          => dec64_less(b, a)
;    dec64_greater_or_equal(a, b) => dec64_not(dec64_less(a, b))
;    dec64_less_or_equal(a, b)    => dec64_not(dec64_less(b, a))

    cmp     r1_b,128        ; is the first argument nan?
    setne   r8_b            ; r8_b is 1 if first is not nan
    cmp     r2_b,128        ; is the second argument nan?
    setne   r9_b            ; r9_b is 1 if second is not nan
    test    r8_b,r9_b       ; is either nan?
    jz      return_nan

; If the exponents are the same, or the coefficient signs are different, then
; do a simple compare.

    mov     r10,r1          ; r10 is the first number
    cmp     r1_b,r2_b       ; are the two exponents equal?
    setne   r8_b            ; r8_b is 0 if the exponents are equal
    xor     r10,r2          ; r10 is the two numbers xor together
    setns   r10_b           ; r10_b is 0 if the sign bits were different
    test    r10_b,r8_b      ; exponents equal or sign bits different
    jnz     less_slow       ; do it another way
    cmp     r1,r2           ; compare the numbers
    setl    r0_b            ; r0_b is 1 if the first number is less
    movzx   r0,r0_b         ; r0 is 1 if the first number is less
    shl     r0,8            ; that bit distinguishes true from false
    add     r0,false        ; convert to boolean
    ret
    pad

less_slow:

; Do it the hard way with a subtraction.

    call_with_two_parameters dec64_subtract ; r0 is r1 - r2
    cmp     r0_b,128        ; is the difference nan?
    je      return_nan
    shr     r0,63           ; r0 is 1 if the first number is less
    shl     r0,8            ; that bit distinguishes true from false
    add     r0,false        ; convert to boolean
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_integer: function_with_one_parameter
;(number: dec64) returns comparison: dec64

; If the number contains a non-zero fractional part or if it is nan, return
; false. Otherwise, return true.

    cmp     r1_b,128        ; nan exponent?
    je      return_false    ; nan is not an integer
    mov     r0,r1           ; r0 is the number
    sar     r0,8            ; r0 is the coefficient
    cmovz   r1,r0           ; if the coefficient is zero, so is the exponent
    movsx   r8,r1_b         ; r8 is the exponent
    test    r1_b,r1_b       ; examine the exponent
    js      is_integer_frac ; if the exponent is not negative, then frac
    pad

return_true:

    mov     r0,true         ; return true
    ret
    pad

is_integer_frac:

    neg     r8              ; negate the exponent
    cmp     r8_b,17         ; extreme negative exponents can never be integer
    jae     return_false
    mov     r9,power
    mov     r10,[r9][r8*8]  ; r10 is 10^-exponent
    cqo                     ; sign extend r0 into r2
    idiv    r10             ; divide r2:r0 by the power of ten
    xor     r0,r0           ; r0 is zero
    test    r2,r2           ; examine the remainder
    setz    r0_b            ; if the remainder is zero, then return one
    movzx   r0,r0_b
    shl     r0,8
    add     r0,false
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_any_nan: function_with_one_parameter
;(number: dec64) returns comparison: dec64

    cmp     r1_b,128        ; is r1 nan?
    sete    r0_b            ; r0 is 1 if r1 is nan
    movzx   r0,r0_b
    shl     r0,8
    add     r0,false
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_is_zero: function_with_one_parameter
;(number: dec64) returns comparison: dec64

    xor     r0,r0           ; r0 is zero
    test    r1,-256         ; coefficient mask
    setz    r0_b            ; r0 is one if the coefficient is zero
    cmp     r1_b,128        ; is the number nan?
    setnz   r0_h            ; r0_h is 1 if the number is not nan
    and     r0_b,r0_h       ; r0 is one if the coefficient is zero and not nan
    movzx   r0,r0_b
    shl     r0,8
    add     r0,false
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_int: function_with_one_parameter
;(number: dec64) returns integer: dec64

; Convert the number such that the exponent will be zero, discarding the
; fraction part. It will produce nan if the result cannot be represented in
; 56 signed bits. This is used to extract an int56 from a dec64 for bitwise
; operations. It accepts a broader range than the safe integer range:
; -36028797018963968 thru 72057594037927935.

; Preserved: r11

    mov     r0,r1           ; r0 is the number
    test    r1_b,r1_b       ; examine the exponent
    jz      return          ; nothing to do it the exponent is zero
    js      dec64_floor     ; shed the fraction part if exponent is negative
    movzx   r8,r1_b         ; r8 is the exponent
    mov     r2,18           ; r2 is 18
    cmp     r1_b,18         ; is the exponent too enormous?
    cmovae  r8,r2           ; r8 is min(exponent, 18)
    and     r0,-256         ; r0 is the coefficient, shifted 8
    mov     r9,power
    mov     r10,[r9][r8*8]  ; r10 is 10^exponent
    imul    r10             ; r0 is coefficient * 10^exponent
    sar     r2,1            ; shift the lsb of the overflow into carry
    adc     r2,0            ; if r2 was 0 or -1, it is now 0
    jnz     return_nan      ; was the coefficient too enormous?
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dec64_normal: function_with_one_parameter
;(number: dec64) returns normalization: dec64

; Make the exponent as close to zero as possible without losing any signficance.
; Usually normalization is not needed since it does not materially change the
; value of a number.

    mov     r0,r1           ; r0 is the number
    cmp     r1_b,128        ; compare the exponent to nan
    jz      return_nan      ; if exponent is nan, the result is nan
    and     r0,-256         ; r0 is the coefficient shifted 8 bits
    mov     r8,10           ; r8 is the divisor
    cmovz   r1,r0           ; r1 is zero if r0 is zero
    mov     r10,r0          ; r10 is the coefficient shifted 8 bits
    test    r1_b,r1_b       ; examine the exponent
    jz      return          ; if the exponent is zero, return r0
    jns     normal_multiply ; if the exponent is positive
    sar     r0,8            ; r0 is the coefficient
    sar     r10,8           ; r10 is the coefficient
    pad

normal_divide:

; While the exponent is less than zero, divide the coefficient by 10 and
; increment the exponent.

    cqo                     ; sign extend r0 into r2
    idiv    r8              ; divide r2:r0 by 10
    test    r2,r2           ; examine the remainder
    jnz     normal_divide_done ; if r2 is not zero, we are done
    mov     r10,r0          ; r10 is the coefficient
    add     r1_b,1          ; increment the exponent
    jnz     normal_divide   ; until the exponent is zero
    pad

normal_divide_done:

    mov     r0,r10          ; r0 is the finished coefficient
    shl     r0,8            ; put it in position
    mov     r0_b,r1_b       ; mix in the exponent
    ret
    pad

normal_multiply:

; While the exponent is greater than zero, multiply the coefficient by 10 and
; decrement the exponent. If the coefficient gets too large, wrap it up.

    imul    r0,10           ; r0 is r0 * 10
    jo      normal_multiply_done ; return zero if overflow
    mov     r10,r0          ; r10 is the coefficient
    sub     r1_b,1          ; decrement the exponent
    jnz     normal_multiply ; until the exponent is zero
    ret
    pad

normal_multiply_done:

    mov     r0,r10          ; r0 is the finished positioned coefficient
    mov     r0_b,r1_b       ; mix in the exponent
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return:

; Return whatever is in r0.

    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_nan:

; All of the dec64_ functions return only this form of nan.

    mov     r0,128          ; nan
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_one:

    mov     r0,256          ; one
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_zero:

    xor     r0,r0           ; zero
    ret

    pad; -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

return_false:

    mov     r0,false
    ret

dec64_code ends
    end
