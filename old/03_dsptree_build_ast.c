#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __APPLE__
#include <editline/readline.h>
#else
#include <editline.h>
#endif

#define dt_float_t float

/* NUMBERS */
/***********/

int get_number(char *str, dt_float_t *number);

/*
*  Computes a fractional number and puts the result in `number` pointer.
*  Returns 0 on failure and 1 on success.
*/
int
compute_fractional(char *str, dt_float_t *number) {
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

/*
*  Converts a string to floating-point number and puts the result in 
*  `number` pointer.
*  Returns 0 on failure and 1 on success.
*/
int
get_number(char *str, dt_float_t *number) {
    char *check = str;
    errno = 0;
    *number = strtod(str, &check);
    if (errno != 0 || str == check || *check != 0) {
        if (!compute_fractional(str, number))
            return 0;
    }
    return 1;
}

/* TOKENIZER */
/*************/

enum TokenType {T_OpenBrace, T_CloseBrace, T_Number, T_Symbol, T_String, NumTokenType};
char *TokenName[NumTokenType] = {"OPEN BRACE", "CLOSE BRACE", "NUMBER", "SYMBOL", 
                                 "STRING"};

typedef struct {
    int type;
    union {
        dt_float_t number;
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

void
print_token(Token t) {
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

/* ABSTRACT SYNTAX TREE */
/************************/

enum NodeType {N_Root, N_Expr, N_Operator, N_Number, N_Symbol, N_String, NumNodeType};

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

int is_node_childs_all_numbers(Node *node) {
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
    int i, ok;
    dt_float_t val;
    Node *new;
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
                    if (is_node_childs_all_numbers(node)) {
                        reduce_node(node);
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

void
print_ast(Node *node, int tab) {
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

Node *
eval_token(Token t, Node *node) {
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

/* PARSER */
/**********/

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

int 
parse(char *input) {
    int i, numpass=4;
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
        curnode = eval_token(t, curnode);
        //print_token(t);
        get_token(remain, &start, &remain);
    }

    /* Reduce number-only expressions. */
    for (i=0; i<numpass; i++) {
        reduce_ast(root);
    }

    print_ast(root, 0);

    delete_node(root);

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
    else {
        input = readfile("test_ast.dt");
        if (input) {
            parse(input);
            free(input);
        }
    }
    //while ((input = readline("> ")) != NULL) {
    //    add_history(input);
    //    parse(input);
    //    free(input);
    //}

    return 0;
}
