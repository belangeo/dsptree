#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "numbers.h"
#include "tokens.h"
#include "ast.h"
#include "parser.h"
#include "utils.h"

int parse(char *input) {
    char token[64];
    char *start, *remain = input;
    dt_float_t number;
    Token t;
    Node *root = make_node(N_Root), *curnode = root;

    if (dsptree_print_level == 2 || dsptree_print_level == 4)
        printf("\nTOKENS ==============================\n");

    get_token(remain, &start, &remain);

    while(start != NULL) {
        snprintf(token, 63, "%.*s", (int)(remain - start), start);

        if (strchr("0123456789.-+", token[0]) != NULL) {
            if (get_number(token, &number)) {
                t.type = T_Number;
                t.data.number = number;
            } else {
                if (token[0] == '-' || token[0] == '+') {
                    t.type = T_Symbol;
                    strcpy(t.data.string, token);
                } else {
                    printf("ERROR: %s is not a valid number!\n", token);
                    return -1;
                }
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
        if (dsptree_print_level == 2 || dsptree_print_level == 4)
            print_token(t);

        curnode = add_node_from_token(t, curnode);
        get_token(remain, &start, &remain);
    }

    /* Reduce number-only expressions. */
    reduce_ast(root);

    if (dsptree_print_level > 2) {
        printf("\nABSTRACT SYNTAX TREE ================\n");
        print_ast(root, 0);
    }

    delete_node(root);

    return 0;
}
