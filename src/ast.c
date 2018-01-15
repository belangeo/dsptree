#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "ast.h"

void
delete_node(Node *node) {
    int i;
    for (i=0; i<node->count; i++) {
        delete_node(node->childs[i]);
    }
    if (node != NULL)
        free(node);
}

Node *
make_node(int type) {
    Node *node;
    node = malloc(sizeof(Node));
    node->type = type;
    node->parent = NULL;
    node->childs = NULL;
    node->count = 0;
    return node;
}

int is_node_childs_numbers(Node *node) {
    int i, ok = 1;
    for (i=1; i<node->count; i++) {
        if (node->childs[i]->type != N_Number) {
            ok = 0;
            break;
        }
    }
    return ok;
}

void reduce_node(Node *node) {
    int i;
    dt_float_t result;
    result = node->childs[1]->data.number;
    if (strcmp(node->childs[0]->data.string, "*") == 0) {
        for (i=2; i<node->count; i++) {
            result *= node->childs[i]->data.number;
        }
    }
    else if (strcmp(node->childs[0]->data.string, "/") == 0) {
        for (i=2; i<node->count; i++) {
            result /= node->childs[i]->data.number;
        }
    }
    else if (strcmp(node->childs[0]->data.string, "+") == 0) {
        for (i=2; i<node->count; i++) {
            result += node->childs[i]->data.number;
        }
    }
    else if (strcmp(node->childs[0]->data.string, "-") == 0) {
        for (i=2; i<node->count; i++) {
            result -= node->childs[i]->data.number;
        }
    }
    node->type = N_Number;
    for (i=0; i<node->count; i++) {
        delete_node(node->childs[i]);
    }
    node->count = 0;
    node->data.number = result;
}

void reduce_ast(Node *node) {
    int i;
    switch (node->type) {
        case N_Root:
            if (node->count > 0) {
                for (i=0; i<node->count; i++) {
                    reduce_ast(node->childs[i]);
                }
            }
            break;
        case N_Expr:
            if (node->count > 0) {
                if (node->childs[0]->type == N_Operator) {
                    if (is_node_childs_numbers(node)) {
                        reduce_node(node);
                        reduce_ast(node->parent);
                    } else {
                        for (i=0; i<node->count; i++) {
                            reduce_ast(node->childs[i]);
                        }
                    }
                } else {
                    for (i=0; i<node->count; i++) {
                        reduce_ast(node->childs[i]);
                    }
                }
            }
            break;
        case N_Operator:
        case N_Number:
        case N_String:
        case N_Symbol:
            break;
    }
}

void print_ast(Node *node, int tab) {
    int i;
    switch (node->type) {
        case N_Root:
            if (node->count > 0) {
                for (i=0; i<node->count; i++) {
                    print_ast(node->childs[i], tab);
                }
            }
            break;
        case N_Expr:
            if (node->count > 0) {
                for (i=0; i<node->count; i++) {
                    print_ast(node->childs[i], tab+4);
                }
            }
            break;
        case N_Operator:
            printf("%*s%s\n", tab-4, "", node->data.string);
            break;
        case N_Number:
            printf("%*s%f\n", tab, "", node->data.number);
            break;
        case N_String:
        case N_Symbol:
            printf("%*s%s\n", tab, "", node->data.string);
            break;
    }
}

Node * add_node_from_token(Token t, Node *node) {
    int type = 0;
    Node *child = NULL;
    switch (t.type) {
        case T_OpenBrace:
            child = make_node(N_Expr);
            child->parent = node;
            node->count++;
            node->childs = realloc(node->childs, node->count * sizeof(Node));
            node->childs[node->count-1] = child;
            return child;
        case T_CloseBrace:
            return node->parent;
        case T_Number:
            child = make_node(N_Number);
            child->data.number = t.data.number;
            break;
        case T_Symbol:
            type = N_Symbol;
            if (node->type == N_Expr && node->count == 0)
                type = N_Operator;
            child = make_node(type);
            strcpy(child->data.string, t.data.string);
            break;
        case T_String:
            child = make_node(N_String);
            strcpy(child->data.string, t.data.string);
            break;
    }
    node->count++;
    node->childs = realloc(node->childs, node->count * sizeof(Node));
    node->childs[node->count-1] = child;
    return node;
}


