#include "dec64_test.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    int nr_fail =
        dec64_test_all_base() +
        dec64_test_all_string() +
        dec64_test_all_math() +
        0;
    printf("Total: %d tests failed\n", nr_fail);
    return 0;
}
