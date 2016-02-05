/*
dec64_string.c
Conversion between DEC64 and strings.

dec64.com
2016-02-04
Public Domain

No warranty.

This file contains dec64_to_string and dec64_from_string, and dec64_string-*
configuration functions.
*/

#include <stdlib.h>
#include "dec64.h"
#include "dec64_string.h"

static const int e = 'e';
static const int64 confirmed = 0xFFDEADFACEC0DECELL;

static const int64 power[17] = {
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
};

/* functions in service to dec64_to_string */

static void digitize(dec64_string_state state) {
    int64 coefficient;
    int digit;
    int place;

    coefficient = dec64_coefficient(state->number);
    if (coefficient < 0) {
        coefficient = -coefficient;
    }
    state->nr_digits = 0;
    state->nr_zeros = 0;
    for (place = 16; place >= 0; place -= 1) {
        digit = (int)(coefficient / power[place]);
        state->digits[state->nr_digits] = digit + '0';
        if (digit == 0) {
            if (state->nr_digits != 0) {
                state->nr_digits += 1;
            }
            state->nr_zeros += 1;
        } else {
            state->nr_digits += 1;
            state->nr_zeros = 0;
        }
        coefficient -= digit * power[place];
    }
}

static emit(dec64_string_state state, int c) {
    if (state->string != NULL) {
        if (c > 0) {
            state->string[state->length] = (dec64_string_char)c;
            state->length += 1;
        }
    }
}

static void emit_at(dec64_string_state state, int64 at) {
    emit(
        state, 
        (at >= state->nr_digits || at < 0)
            ? '0' 
            : state->digits[at]
    );
}

static void emit_decimal_point(dec64_string_state state) {
    emit(state, state->decimal_point);
}

static void emit_digits(
    dec64_string_state state,
    int from,
    int to
) {
    int at;
    for (at = from; at < to; at += 1) {
        emit_at(state, at);
    }
}

static void emit_digits_separated(
    dec64_string_state state,
    int from, int to
) {
    int sep;
    if (state->separation <= 0) {
        emit_digits(state, from, to);
    }
    sep = to % state->separation;
    if (sep <= 0) {
        sep = state->separation;
    }
    while (1) {
        emit_digits(state, from, sep);
        from = sep;
        if (from >= to) {
            break;
        }
        emit(state, state->separator);
        sep += state->separation;
    }
}

static void emit_end(dec64_string_state state) {
    if (state->string != NULL) {
        state->string[state->length] = 0;
    }
}

static emit_exponent(
    dec64_string_state state,
    int64 exponent
) {
    int go = 0;
    if (exponent != 0) {
        emit(state, e);
        if (exponent < 0) {
            exponent = -exponent;
            emit(state, '-');
        }
        if (exponent >= 100) {
            emit(state, '1');
            exponent -= 100;
            go = 1;
        }
        if (exponent >= 10 || go) {
            emit(state, '0' + (dec64_string_char)(exponent / 10));
        }
        emit(state, '0' + exponent % 10);
    }
}

static emit_separator(dec64_string_state state) {
    emit(state, state->separator);
}

static void engineering(dec64_string_state state) {
    int64 exponent = dec64_exponent(state->number) + state->nr_digits;
    int to = state->nr_digits - state->nr_zeros;
    int fudge = (int)exponent % 3;
    if (fudge <= 0) {
        fudge += 3;
    }
    emit_digits(state, 0, fudge);
    if (fudge < to) {
        emit_decimal_point(state);
        emit_digits(state, fudge, to);
    }
    emit_exponent(state, exponent - fudge);
}

static void scientific(dec64_string_state state) {
    int64 exponent = dec64_exponent(state->number) + state->nr_digits;
    int nr_digits = (state->nr_digits - state->nr_zeros);
    int at = 1;
    emit_at(state, 0);
    if (at < nr_digits) {
        emit_decimal_point(state);
        emit_digits(state, 1, nr_digits);
    }
    emit_exponent(state, exponent - 1);
}

static void standard(dec64_string_state state) {
    int from = 0;
    int to;
    int places;
    int sep = 0;
    int64 exponent = dec64_exponent(state->number);
    if (exponent >= 0) {
        to = state->nr_digits + (int)exponent;
        if (to + state->places > 20) {
            scientific(state);
        } else {
            emit_digits_separated(state, 0, to);
            if (state->places > 0) {
                emit_decimal_point(state);
                emit_digits(state, to, state->places + to);
            }
        }
    } else {
        from = (int)exponent + state->nr_digits;
        to = state->nr_digits - state->nr_zeros;
        if (from <= 0) {
            places = to - from;
            if (places > 18) {
                scientific(state);
            } else {
                emit(state, '0');
                emit_decimal_point(state);
                if (places < state->places) {
                    to = state->places + from;
                }
                emit_digits(state, from, to);
            }
        } else {
            emit_digits_separated(state, 0, from);
            emit_decimal_point(state);
            if (to - from < state->places) {
                to = state->places + from;
            }
            emit_digits(state, from, to);
        }
    }
}

/* creation */

dec64_string_state dec64_string_begin() {
/*
    Create a state object. State objects are passed as the first argument to 
    the other public functions. It holds state so that this module is reentrant
    and thread safe. Do not manipulate this object directly. Use the functions.
    It can return NULL if memory allocation fails.
*/
    dec64_string_state state =
            (dec64_string_state)malloc(sizeof (struct dec64_string_state));
    if (state) {
        state->decimal_point = '.';
        state->mode = standard_mode;
        state->nr_digits = 0;
        state->nr_zeros = 0;
        state->number = DEC64_NAN;
        state->places = 0;
        state->separation = 3;
        state->separator = 0;
        state->string = 0;
        state->valid = confirmed;
    }
    return state;
}

/* destruction */

void dec64_string_end(dec64_string_state state) {
/*
    Dispose of the state object.
*/
    if (state->valid == confirmed) {
        state->valid = 0;
        free(state);
    }
}

/*
    There are three output modes: standard, scientific, and engineering.
    The modes are used by dec64_to_string and ignored by dec64_from_string.
*/

void dec64_string_engineering(dec64_string_state state) {
/*
    Put dec64_to_string into engineering mode, which is like scientific mode
    except that the exponent is constrained to be a multiple of 3. There can
    be up to three digits before the decimal point.
*/
    state->mode = engineering_mode;
}

void dec64_string_scientific(dec64_string_state state) {
/*
    Put dec64_to_string into scientific mode, in which the coefficient is
    displayed with one digit before the decimal point, and an exponent 
    prefixed with 'e' is appended if necessary.
*/
    state->mode = scientific_mode;
}

void dec64_string_standard(dec64_string_state state) {
/*
    Put dec64_to_string into standard mode. Separators can be inserted between
    some digits if requested. If the number might be too long, then scientific
    mode might be used instead.
*/
    state->mode = standard_mode;
}

/* There are several formatting options. */

dec64_string_char dec64_string_decimal_point(
    dec64_string_state state,
    dec64_string_char decimal_point
) {
/*
    Specify the decimal point character. The default is '.'. If it is '\0',
    then the decimal point will be suppressed. This is used by both
    dec64_to_string and dec64_from_string.

    It returns the previous value.
*/
    int old_decimal_point = state->decimal_point;
    state->decimal_point = decimal_point;
    return old_decimal_point;
}

int dec64_string_places(
    dec64_string_state state,
    dec64_string_char places
) {
/*
    Specify the minimum number of decimal places output by dec64_to_string in
    standard mode. This is commonly used to format money values.

    It returns the previous value.
*/
    int old_places = state->places;
    state->places = places;
    return old_places;
}

int dec64_string_separation(
    dec64_string_state state,
    int separation
) {
/*
    Specify the number of digits output between separators by dec64_to_string
    in standard mode. The default is 3. If separation is 1 or less, then
    separation is suppressed.

    It returns the previous value.
*/
    int old_separation = state->separation;
    if (separation < 2) {
        separation = 0;
    }
    state->separation = separation;
    return old_separation;
}

dec64_string_char dec64_string_separator(
    dec64_string_state state,
    dec64_string_char separator
) {
/*
    Specify the separation character, which is used by dec64_to_string in
    standard mode to improve the readability of digits before the decimal
    point. Typical values include ',', ' ', '_', and '.'. If it is '\0',
    then separation will be supressed. The default is '\0'.

    dec64_from_string will ignore this character.

    It returns the previous value.
*/
    int old_separator = state->separator;
    state->separator = separator;
    return old_separator;
}

/* Action. */

dec64 dec64_from_string(
    dec64_string_state state, 
    dec64_string_char string[]
) {
/*
    Convert a string into a dec64. If conversion is not possible for any
    reason, the result will be nan.
*/
    int at;
    int c;
    int digits;
    int leading;
    int ok;
    int point;

    int64 coefficient;
    int64 exp;
    int64 exponent;
    int64 sign;
    int64 sign_exp;

    if (state == NULL || state->valid != confirmed || string == NULL) {
        return 128;
    }
/*
    Get the first character.
*/
    c = string[0];
    coefficient = 0;
    digits = 0;
    exponent = 0;
    leading = 1;
    ok = 0;
    point = 0;
/*
    Minus sign.
*/
    if (c == '-') {
        c = string[1];
        at = 1;
        sign = -1;
    } else {
        at = 0;
        sign = 1;
    }
/*
    Loop through the string to the end.
*/
    while (c != 0) {
/*
    Skip the separator character.
*/
        if (c != state->separator) {
/*
    Is the character a zero?
*/
            if (c == '0') {
                ok = 1;
/*
    If this is a leading zero, then decrement the exponent if
    it appears after a decimal point.
*/
                if (leading) {
                    exponent -= point;
                } else {
/*
    This is not a leading zero. We will only accumulate the
    first 18 digits. An excess digit isn't accumlated, but
    it can influence the exponent.
*/
                    digits += 1;
                    if (digits > 18) {
                        exponent += 1 - point;
                    } else {
                        coefficient *= 10;
                        exponent -= point;
                    }
                }
/*
    Is the character another fine digit?
*/
            } else if (c >= '1' && c <= '9') {
/*
    A number must include at least 1 digit. Stop ignoring leading zeros.
*/
                ok = 1;
                leading = 0;
/*
    Count the number of digits. Only accumulate the first 18 digits. The most
    we can use is 17. We take one more for rounding.
*/
                digits += 1;
                if (digits > 18) {
/*
    If this is an excess character, and we haven't seen a
    decimal point, then increment the exponent.
*/
                    exponent += 1 - point;
/*
    If we see a significant digit, stop skipping leading zeros, acccumulate the
    digit into the coefficient, and decrement the exponent if we have seen a
    decimal point.
*/
                } else {
                    coefficient = coefficient * 10 + (c - '0');
                    exponent -= point;
                }
/*
    There is a decimal point. If there is more than one decimal
    point, return nan.
*/
            } else if (c == state->decimal_point) {
                if (point) {
                    return DEC64_NAN;
                }
                point = 1;
/*
    Handle the E component.
*/
            } else {
                if (c == 'e' || c == 'E') {
                    if (ok) {
                        ok = 0;
                        exp = 0;
                        sign_exp = 1;
                        at += 1;
                        c = string[at];
/*
    Optional minus or plus
*/
                        if (c == '-') {
                            sign_exp = -1;
                            at += 1;
                            c = string[at];
                        } else if (c == '+') {
                            at += 1;
                            c = string[at];
                        }
/*
    The exponent itself.
*/
                        while (c != 0) {
                            if (c >= '0' && c <= '9') {
                                ok = 1;
                                exp = exp * 10 + (c - '0');
                                if (exp < 0) {
                                    return DEC64_NAN;
                                }
                            } else {
                                return DEC64_NAN;
                            }
                            at += 1;
                            c = string[at];
                        }
                    }
/*
    If everything is ok, incorporate the exponent in the new number.
*/
                    if (ok) {
                        return dec64_new(
                            sign * coefficient,
                            (sign_exp * exp) + exponent
                        );
                    }
/*
    If any other character is seen, return nan.
*/
                }
                return DEC64_NAN;
            }
        }
/*
    Get the next charcter.
*/
        at += 1;
        c = string[at];
    }
/*
    If everything is ok, return the number.
*/
    return (ok)
        ? dec64_new(sign * coefficient, exponent)
        : DEC64_NAN;
}

int dec64_to_string(
    dec64_string_state state,
    dec64 number,
    dec64_string_char string[]
) {
/*
    dec64_to_string converts a dec64 number into a string. The caller provides
    the memory in which to deposit the string. The string must have sufficient
    capacity to hold 32 characters. If NULL is passed in as the string, then
    no characters will be deposited, but a character count will be returned.

    dec64_to_string returns the number of characters actually deposited in the
    string (not including the trailing \0). If the number is nan, then it
    returns 0 indicating an empty string.

    In standard mode, the number will be formatted conventionally unless it
    would require more than 17 digits, which would be due to excessive
    trailing zeros or zeros immediately after the decimal point. In that
    case scientific notation will be used instead.
*/
    if (state == NULL || state->valid != confirmed) {
        return 0;
    }

    state->length = 0;
    state->string = string;
    if (dec64_is_any_nan(number) != DEC64_TRUE) {
        if (dec64_is_zero(number) == DEC64_TRUE) {
            emit(state, '0');
        } else {
            if (number != state->number) {
                state->number = number;
                digitize(state);
            }
            if (number < 0) {
                emit(state, '-');
            }
            switch (state->mode) {
            case engineering_mode:
                engineering(state);
                break;
            case scientific_mode:
                scientific(state);
                break;
            case standard_mode:
                standard(state);
                break;
            }
        }
    }
    emit_end(state);
    state->string = NULL;
    return state->length;
}
