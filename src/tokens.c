#include <stdio.h>
#include <string.h>
#include "tokens.h"

static char *TokenName[NumTokenType] = {"OPEN BRACE", "CLOSE BRACE",
                                        "NUMBER", "SYMBOL", "STRING"};

void get_token(char *str, char **start, char **end) {
    static const char *whitespaces = " \t\r\n";
    static const char *delimiters = "() \t\r\n";
    static const char *braces = "()";
    static const char *string = "\"";
    static const char *comment = ";";
    static const char *endofline = "\r\n\0";

    /* Ignore white spaces. */
    str += strspn(str, whitespaces);

    /* Ignore comments. */
    if (strchr(comment, str[0]) != NULL)
        str += strcspn(str, endofline) + 1;

    /* All bytes consumed. */
    if (str[-1] == '\0') {
        *start = *end = NULL;
        return;
    }

    /* Set a pointer to the beginning of the token. */
    *start = str;

    /* Find the end of the token. Will be starting point for the next token. */
    if (strchr(braces, str[0]) != NULL)             /* parenthesis */
        *end = *start + 1;
    else if (strchr(string, str[0]) != NULL)        /* string */
        *end = *start + strcspn(str+1, string) + 2;
    else                                            /* number or symbol */
        *end = *start + strcspn(str, delimiters);
}

void print_token(Token t) {
    switch (t.type) {
        case T_OpenBrace:
        case T_CloseBrace:
            printf("%s\n", TokenName[t.type]);
            break;
        case T_Number:
            printf("%s: %f\n", TokenName[t.type], t.data.number);
            break;
        case T_Symbol:
        case T_String:
            printf("%s: %s\n", TokenName[t.type], t.data.string);
            break;
    }
}

