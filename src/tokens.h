#ifndef __TOKENS_H
#define __TOKENS_H

#include "types.h"

enum TokenType {T_OpenBrace, T_CloseBrace, T_Number, T_Symbol, T_String,
                NumTokenType};

typedef struct {
    int type;
    union {
        dt_float_t number;
        char string[64];
    } data;
} Token;

void get_token(char *str, char **start, char **end);
void print_token(Token t);

#endif /* __TOKENS_H */
