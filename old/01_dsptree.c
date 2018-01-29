#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <editline.h>

enum TokenType {OpenBrace, CloseBrace, Number, Symbol, String};

typedef struct val {
    int type;
    union {
        double value;
        char *symbol;
    };
} val;

val
make_number(double value) {
    val v;
    v.type = Number;
    v.value = value;
    return v;
}

val
make_symbol(char *symbol) {
    val v;
    v.type = Symbol;
    v.symbol = symbol;
    return v;
}

void 
get_token(char *str, char **start, char **end) {

    static const char *whitespaces = " \t\r\n";
    static const char *delimiters = "() \t\r\n";
    static const char *prefix = "()";
    static const char *string = "\"";

    /* Ignore white spaces. */
    str += strspn(str, whitespaces);

    /* All bytes consumed. */
    if (str[0] == '\0') {
        *start = *end = NULL;
        return;
    }

    /* Set a pointer to the beginning of the token. */
    *start = str;

    /* Find the end of the token. This pointer will be the 
       starting point for the next token. */
    if (strchr(prefix, str[0]) != NULL)             /* parenthesis */
        *end = *start + 1;
    else if (strchr(string, str[0]) != NULL)        /* string */
        *end = *start + strcspn(str+1, string) + 2;
    else                                            /* number or symbol */
        *end = *start + strcspn(str, delimiters);
}

int 
parse(char *input) {

    char token[64];
    char *start;
    char *remain = input;
    int level = 0;

    get_token(remain, &start, &remain);

    while(start != NULL) {
        errno = 0;
        snprintf(token, 63, "%.*s", (int)(remain - start), start);
        
        char *check = token;
        double number = strtod(token, &check);
        if (errno != 0 || token == check || *check != 0) {
            if (strcmp(token, "(") == 0)
                level += 2; /* open brace */
            else if (strcmp(token, ")") == 0)
                level -= 2; /* close brace */
            else {
                val v = make_symbol(token);
                printf("%*s", level, "");
                printf("%s\n", v.symbol);
            }
        }
        else {
            val v = make_number(number);
            printf("%*s", level, "");
            printf("%f\n", v.value);
        }

        get_token(remain, &start, &remain);
    }

    return 0;
}

int 
main(int argc, char **argv) {

    char *input;

    while ((input = readline("> ")) != NULL) {
        add_history(input);
        parse(input);
        free(input);
    }

    return 0;
}
