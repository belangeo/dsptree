#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "preproc.h"
#include "parser.h"

#ifdef __APPLE__
#include <editline/readline.h>
#else
#include <editline.h>
#endif

int main(int argc, char **argv) {
    char *input=NULL, *preproc=NULL;

    dsptree_print_level = 0;

    if (argc == 2) {
        input = readfile(argv[1]);
    } else if (argc == 4) {
        if (strcmp(argv[1], "-p") == 0)
            dsptree_print_level = atoi(argv[2]);
        input = readfile(argv[3]);
    }

    if (input) {
        preproc = process(input);
        parse(preproc);
        free(input);
        free(preproc);
    }

    while ((input = readline("> ")) != NULL) {
        add_history(input);
        preproc = process(input);
        parse(preproc);
        free(input);
        free(preproc);
    }

    return 0;
}
