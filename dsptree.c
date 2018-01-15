#include <stdlib.h>
#include "parser.h"

#ifdef __APPLE__
#include <editline/readline.h>
#else
#include <editline.h>
#endif

int main(int argc, char **argv) {
    char *input;

    if (argc > 1) {
        input = readfile(argv[1]);
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
