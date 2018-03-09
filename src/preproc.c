#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "preproc.h"
#include "utils.h"

static char * 
load_libs(char *input) {
    int i, ok, pos, commented, psize, size = 0, which = 0, loaded = 0;
    char filename[64];
    char *ptr, *content, *buffer;
    char *next = input;

    ptr = strstr(input, "(load ");
    while (ptr) {
        ok = 0; pos = 0, commented = 0;
        ptr += 6;
        /* Check if commented. */
        next = ptr;
        while (*next != '\n' && *next != '\r') {
            next--;
            if (*next == ';') {
                commented = 1;
                break;
            }
            if (next == input)
                break;
        }

        /* Find librairie name. */
        next = ptr;
        while (*next != '\n' && *next != '\0') {
            if (*next == '\"' && ok == 0) {
                ok = 1;
            } else if (*next == '\"') {
                filename[pos++] = '\0';
                break;
            } else if (ok) {
                filename[pos++] = *next;
            }
            next++;
        }

        /* Load the librairie if not commented. */
        if (!commented) {
            strcat(filename, ".dtlib");

            content = readfile(filename);
            psize = strlen(content);
            if (which == 0)
                buffer = (char *)malloc(sizeof(char) * (size + psize));
            else
                buffer = (char *)realloc(buffer, sizeof(char) * (size + psize));
            which++;

            for (i=0; i<psize; i++) {
                buffer[size+i] = content[i];
            }

            size += psize;

            free(content);
            loaded = 1;
        }

        ptr = strstr(ptr, "(load ");
    }

    if (next != input && loaded == 0) {
        next += 2;
        psize = strlen(next);
        buffer = (char *)realloc(buffer, sizeof(char) * (size + psize + 1));
        for (i=0; i<psize; i++) {
            buffer[size+i] = next[i];
        }
        size += psize;
    } else {
        size = strlen(input);
        buffer = (char *)malloc(sizeof(char) * (size + 1));
        snprintf(buffer, size, "%s", input);
    }
    buffer[size] = '\0';

    return buffer;
}

static char * 
remove_comments(char *input) {
    int in = 0, out = 0, inside = 0;
    int size = strlen(input);

    while (input[in] != '\0') {
        if (input[in] == ';' && inside == 0)
            inside = 1;
        else if (inside == 1 && (input[in] == '\n' || input[in] == '\r'))
            inside = 0;

        if (!inside)
            input[out++] = input[in];

        in++;
    }

    for ( ; out < size; out++)
        input[out] = '\0';

    return input;
}

char * 
process(char *input) {
    char *buf = load_libs(input);
    buf = remove_comments(buf);
    if (dsptree_print_level == 1 || dsptree_print_level == 4) {
        printf("\nPRE-PROCESSING ======================\n");
        printf("%s\n", buf);
    }
    return buf;
}
