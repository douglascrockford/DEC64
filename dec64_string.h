/* dec64_string.h

The dec64_string header file. This is the companion to dec64_string.c.

dec64.com
2014-04-12
Public Domain

No warranty.
*/

enum dec64_string_mode {
    engineering_mode,
    scientific_mode,
    standard_mode
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

extern dec64_string_char dec64_string_decimal_point(
    dec64_string_state state,
    dec64_string_char decimal_point
);

extern void dec64_string_engineering(
    dec64_string_state state
);

extern int dec64_string_places(
    dec64_string_state state,
    dec64_string_char places
);

extern void dec64_string_scientific(
    dec64_string_state state
);

extern int dec64_string_separation(
    dec64_string_state state,
    int separation
);

extern dec64_string_char dec64_string_separator(
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
