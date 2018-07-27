/* dec64_string.h

The dec64_string header file. This is the companion to dec64_string.c.

dec64.com
2014-04-12
Public Domain

No warranty.
*/


#ifndef DEC64_STRING
#define DEC64_STRING

#ifdef __cplusplus
extern "C" {
#endif

static const int e_chr = 'e';
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

dec64_string_state dec64_string_begin();

/*
    destruction
*/

void dec64_string_end(
    dec64_string_state state
);

/*
    configuration
*/

dec64_string_char dec64_string_decimal_point(
    dec64_string_state state,
    dec64_string_char decimal_point
);

void dec64_string_engineering(
    dec64_string_state state
);

int dec64_string_places(
    dec64_string_state state,
    dec64_string_char places
);

void dec64_string_scientific(
    dec64_string_state state
);

int dec64_string_separation(
    dec64_string_state state,
    int separation
);

dec64_string_char dec64_string_separator(
    dec64_string_state state,
    dec64_string_char separator
);

void dec64_string_standard(
    dec64_string_state state
);

/*
    action
*/

dec64 dec64_from_string(
    dec64_string_state state,
    dec64_string_char string[]
);

int dec64_to_string(
    dec64_string_state state,
    dec64 number,
    dec64_string_char string[]
);



#ifdef __cplusplus
}
#endif

#endif //DEC64_STRING
