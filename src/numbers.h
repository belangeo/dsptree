#ifndef __NUMBERS_H
#define __NUMBERS_H

#include "types.h"

/*
*  Computes a fractional number and puts the result in `number` pointer.
*  Returns 0 on failure and 1 on success.
*/
int compute_fractional(char *str, dt_float_t *number);

/*
*  Converts a string to floating-point number and puts the result in 
*  `number` pointer.
*  Returns 0 on failure and 1 on success.
*/
int get_number(char *str, dt_float_t *number);

#endif
