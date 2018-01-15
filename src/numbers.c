#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "numbers.h"

int compute_fractional(char *str, dt_float_t *number) {
    if (strpbrk(str, "/") == NULL)
        return 0;
    dt_float_t n1, n2;
    char *p = strtok(str, "/");
    if (!get_number(p, &n1))
        return 0;
    p = strtok(NULL, "/");
    if (!get_number(p, &n2))
        return 0;
    *number = n1 / n2;
    return 1;
}

int get_number(char *str, dt_float_t *number) {
    char *check = str;
    errno = 0;
    *number = strtod(str, &check);
    if (errno != 0 || str == check || *check != 0) {
        if (!compute_fractional(str, number))
            return 0;
    }
    return 1;
}


