/* dec64_string.h

The dec64_string header file. This is the companion to dec64_string.c.

dec64.com
2018-12-20
Public Domain

No warranty.
*/
#ifndef __DEC64_STRING_H__
#define __DEC64_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

enum dec64_string_mode {
    dec64_engineering_mode,
    dec64_scientific_mode,
    dec64_standard_mode
};

typedef char dec64_string_char;

typedef struct dec64_string_state {
/*
    For internal use only.
*/
    dec64 valid;
    dec64 number;
    dec64_string_char* string;
    int digits[32];
    int length;
    int nr_digits;
    int nr_zeros;
    int places;
    int separation;
    enum dec64_string_mode mode;
    dec64_string_char decimal_point;
    dec64_string_char separator;
}  * dec64_string_state;

/*
    creation
*/

extern dec64_string_state dec64_string_begin();

/*
    destruction
*/

extern void dec64_string_end(
    dec64_string_state state
);

/*
    configuration
*/

extern void dec64_string_decimal_point(
    dec64_string_state state,
    dec64_string_char decimal_point
);

extern void dec64_string_engineering(
    dec64_string_state state
);

extern void dec64_string_places(
    dec64_string_state state,
    dec64_string_char places
);

extern void dec64_string_scientific(
    dec64_string_state state
);

extern void dec64_string_separation(
    dec64_string_state state,
    int separation
);

extern void dec64_string_separator(
    dec64_string_state state,
    dec64_string_char separator
);

extern void dec64_string_standard(
    dec64_string_state state
);

/*
    action
*/

extern dec64 dec64_from_string(
    dec64_string_state state,
    dec64_string_char string[]
);

extern int dec64_to_string(
    dec64_string_state state,
    dec64 number,
    dec64_string_char string[]
);

extern char* dec64_dump(dec64 number);

#ifdef __cplusplus
}
#endif

#endif
