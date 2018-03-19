#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "preproc.h"
#include "utils.h"

/* Extra whitespaces are:
 * - whitespace or line break after opening brace
 * - whitespace before closing brace
 * - whitespace after another whitespace. */
static void
remove_extra_whitespaces(char *input) {
    char *read = input;
    char *write = input;
    int openbrace = 0, whitespace = 0;
    static const char *whitespaces = " \t\r\n";

    while (*read != '\0') {
        if (*read == ' ' || *read == '\t') {
            if (!openbrace && !whitespace) {
                *write++ = *read;
            }
        } else if (*read == '\n' || *read == '\r') {
            if (!openbrace) {
                *write++ = *read;
            }
        } else if (*read == ')' && whitespace) {
            write--;
            *write++ = *read;
        } else {
            *write++ = *read;
        }

        if (*read == '(') {
            openbrace = 1;
            whitespace = 0;
        } else if (strchr(whitespaces, *read)) {
            whitespace = 1;
        } else {
            openbrace = 0;
            whitespace = 0;
        }
        read++;
    }
    *write = '\0';
}

static void
remove_comments(char *input) {
    char *read = input;
    char *write = input;
    int inside = 0, newline = 0;

    while (*read != '\0') {
        if (*read == ';' && inside == 0) {
            inside = 1;
            if (newline)
                write--;
        }
        else if (inside == 1 && (*read == '\n' || *read == '\r'))
            inside = 0;
        if (*read == '\n' || *read == '\r')
            newline = 1;
        else
            newline = 0;

        if (!inside)
            *write++ = *read;

        read++;
    }
    *write = '\0';
}

/* Returned buffer must be freed (done inside replace_macros). */
static char *
replace_macro(char *input, char *start, char *end, char *name, char *value) {
    int i = 0, count = 0, size;
    int namelen = strlen(name);
    int vallen = strlen(value);
    char *buffer, *ptr, *val;

    ptr = end;
    ptr = strstr(ptr, name);
    while (ptr) {
        count += 1;
        ptr += namelen;
        ptr = strstr(ptr, name);
    }

    size = strlen(input) - (end - start) - namelen * count + vallen * count + 1;
    buffer = (char *)malloc(size * sizeof(char));

    while (input != start) {
        buffer[i++] = *input++;
    }
    input = end;
    ptr = strstr(input, name);
    while (ptr) {
        while (input != ptr) {
            buffer[i++] = *input++;
        }
        val = value;
        while (*val != '\0') {
            buffer[i++] = *val++;
        }
        input += namelen;
        ptr = strstr(input, name);
    }
    while (*input != '\0') {
        buffer[i++] = *input++;
    }
    buffer[i] = '\0';
    return buffer;
}

static char *
expand_macros(char *input) {
    int i, open, close;
    char *ptr, *start, *new = NULL;
    char name[64], value[1024];

    static const char *whitespaces = " \t\r\n";

    ptr = strstr(input, "(macro ");
    while (ptr) {
        if (ptr == input) /* Start of the file. */
            start = ptr;
        else if (strchr(whitespaces, ptr[-1]))
            start = ptr - 1;
        else
            start = ptr;

        // fill new buffer with content up to ptr.
        ptr += 7;

        /* Ignore white spaces. */
        ptr += strspn(ptr, whitespaces);

        /* Get macro name. */
        i = 0;
        while (strchr(whitespaces, ptr[0]) == NULL) {
            name[i++] = *ptr++;
        }
        name[i] = '\0';
        //printf("macro name: %s\n", name);

        /* Ignore white spaces. */
        ptr += strspn(ptr, whitespaces);

        /* Get macro value. */
        i = open = close = 0;
        while (*ptr != '\0') {
            value[i++] = *ptr;
            if (ptr[0] == '(')
                open += 1;
            ptr++;
            if (ptr[0] == ')')
                close += 1;
            if (close > open)
                break;
        }
        value[i] = '\0';
        //printf("macro value: %s\n", value);

        new = replace_macro(input, start, ptr+1, name, value);
        input = (char *)realloc(input, (strlen(new) + 1) * sizeof(char));
        strcpy(input, new);
        strcat(input, "\0");
        free(new);

        ptr = strstr(input, "(macro ");
    }
    return input;
}

static char *
load_libs(char *input) {
    int i, ok, pos, psize, size = 0, which = 0, loaded = 0;
    char filename[64];
    char *ptr, *content, *buffer = NULL;
    char *next = input;

    ptr = strstr(input, "(load ");
    while (ptr) {
        ok = 0; pos = 0;
        ptr += 6;

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

        /* Load the librairie. */
        strcat(filename, ".dtlib");

        content = readfile(filename);
        if (content) {
            /* Library file pre-processing. */
            remove_extra_whitespaces(content);
            remove_comments(content);
            content = expand_macros(content);

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
        } else {
            printf("ERROR: file \"%s\" does not exist!", filename);
        }

        ptr = strstr(ptr, "(load ");
    }

    if (next != input || loaded == 1) {
        next += 2;
        psize = strlen(next);
        if (buffer) {
            buffer = (char *)realloc(buffer, sizeof(char) * (size + psize + 1));
        } else {
            buffer = (char *)malloc(sizeof(char) * (size + psize + 1));
        }
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

    input = (char *)realloc(input, (strlen(buffer) + 1) * sizeof(char));
    strcpy(input, buffer);
    strcat(input, "\0");
    free(buffer);

    return input;
}

char *
process(char *input) {
    remove_extra_whitespaces(input);
    remove_comments(input);
    input = expand_macros(input);
    input = load_libs(input);

    if (dsptree_print_level == 1 || dsptree_print_level == 4) {
        printf("\nPRE-PROCESSING ======================\n");
        printf("%s\n", input);
    }
    return input;
}