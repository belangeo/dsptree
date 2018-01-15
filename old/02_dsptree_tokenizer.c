#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __APPLE__
#include <editline/readline.h>
#else
#include <editline.h>
#endif

/* Prototypes */
int get_number(char *str, double *number);

enum TokenType {OpenBrace, CloseBrace, Number, Symbol, String, NumTokenType};
char *TokenName[NumTokenType] = {"OPEN BRACE", "CLOSE BRACE", "NUMBER", "SYMBOL", 
                                 "STRING"};

typedef struct {
    int type;
    union {
        double number;
        char string[64];
    } data;
} Token;

void 
get_token(char *str, char **start, char **end) {
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
    if (str[0] == '\0') {
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

/*
* Reads the content of a text file.
* The user must free the returned char buffer when done with it.
*/
char *
readfile(char *filename) {
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");

    if (handler) {
        fseek(handler, 0, SEEK_END);
        string_size = ftell(handler);
        rewind(handler);

        buffer = (char*) malloc(sizeof(char) * (string_size + 1) );
        read_size = fread(buffer, sizeof(char), string_size, handler);
        buffer[string_size] = '\0';

        if (string_size != read_size) {
            free(buffer);
            buffer = NULL;
        }

        fclose(handler);
    }

    return buffer;
}

/*
*  Computes a fractional number and puts the result in `number` pointer.
*  Returns 0 on failure and 1 on success.
*/
int
compute_fractional(char *str, double *number) {
    if (strpbrk(str, "/") == NULL)
        return 0;
    double n1, n2;
    char *p = strtok(str, "/");
    if (get_number(p, &n1) == 0)
        return 0;
    p = strtok(NULL, "/");
    if (get_number(p, &n2) == 0)
        return 0;
    *number = n1 / n2;
    return 1;
}

/*
*  Converts a string to floating-point number and puts the result in 
*  `number` pointer.
*  Returns 0 on failure and 1 on success.
*/
int
get_number(char *str, double *number) {
    char *check = str;
    errno = 0;
    *number = strtod(str, &check);
    if (errno != 0 || str == check || *check != 0) {
        if (compute_fractional(str, number) == 0)
            return 0;
    }
    return 1;
}

void
print_token(Token t) {
    switch (t.type) {
        case OpenBrace:
        case CloseBrace:
            printf("%s\n", TokenName[t.type]);
            break;
        case Number:
            printf("%s: %f\n", TokenName[t.type], t.data.number);
            break;
        case Symbol:
        case String:
            printf("%s: %s\n", TokenName[t.type], t.data.string);
            break;
    }
}

int 
parse(char *input) {

    char token[64];
    char *start;
    char *remain = input;
    double number;
    Token t;

    get_token(remain, &start, &remain);

    while(start != NULL) {
        snprintf(token, 63, "%.*s", (int)(remain - start), start);

        if (strchr("0123456789.-+", token[0]) != NULL) {
            if (get_number(token, &number))
                t.type = Number;
                t.data.number = number;
        }
        else if (strcmp(token, "(") == 0)
            t.type = OpenBrace;
        else if (strcmp(token, ")") == 0)
            t.type = CloseBrace;
        else if (token[0] == '\"') {
            t.type = String;
            strcpy(t.data.string, token);
        }
        else {
            t.type = Symbol;
            strcpy(t.data.string, token);
        }
        print_token(t);
        get_token(remain, &start, &remain);
    }
    return 0;
}

int 
main(int argc, char **argv) {
    char *input;

    if (argc > 1) {
        input = readfile(argv[1]);
        if (input) {
            parse(input);
            free(input);
        }
    }

    while ((input = readline("> ")) != NULL) {
        add_history(input);
        parse(input);
        free(input);
    }

    return 0;
}
