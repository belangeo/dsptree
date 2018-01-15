#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "numbers.h"
#include "tokens.h"
#include "ast.h"
#include "parser.h"

/*
 * Reads the content of a text file.
 * The user must free the returned char buffer when done with it.
 */
char * readfile(char *filename) {
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");

    if (handler) {
        fseek(handler, 0, SEEK_END);
        string_size = ftell(handler);
        rewind(handler);

        buffer = (char*)malloc(sizeof(char) * (string_size + 1));
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

int parse(char *input) {
    char token[64];
    char *start;
    char *remain = input;
    dt_float_t number;
    Token t;
    Node *root, *curnode;

    root = make_node(N_Root);
    curnode = root;

    get_token(remain, &start, &remain);

    while(start != NULL) {
        snprintf(token, 63, "%.*s", (int)(remain - start), start);

        if (strchr("0123456789.-+", token[0]) != NULL) {
            if (get_number(token, &number)) {
                t.type = T_Number;
                t.data.number = number;
            }
        }
        else if (strcmp(token, "(") == 0)
            t.type = T_OpenBrace;
        else if (strcmp(token, ")") == 0)
            t.type = T_CloseBrace;
        else if (token[0] == '\"') {
            t.type = T_String;
            strcpy(t.data.string, token);
        }
        else {
            t.type = T_Symbol;
            strcpy(t.data.string, token);
        }
        curnode = add_node_from_token(t, curnode);
        //print_token(t);
        get_token(remain, &start, &remain);
    }

    /* Reduce number-only expressions. */
    reduce_ast(root);

    print_ast(root, 0);

    delete_node(root);

    return 0;
}


