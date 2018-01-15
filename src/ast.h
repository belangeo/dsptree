#ifndef __AST_H
#define __AST_H

#include "tokens.h"

enum NodeType {N_Root, N_Expr, N_Operator, N_Number, N_Symbol,
               N_String, NumNodeType};

typedef struct Node {
    int type;
    union {
        dt_float_t number;
        char string[64];
    } data;
    struct Node *parent;
    struct Node **childs;
    int count;
} Node;

Node * make_node(int type);
void delete_node(Node *node);
int is_node_childs_numbers(Node *node);
void reduce_node(Node *node);
void reduce_ast(Node *node);
void print_ast(Node *node, int tab);
Node * add_node_from_token(Token t, Node *node);

#endif /* __AST_H */
