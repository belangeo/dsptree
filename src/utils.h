#ifndef __UTILS_H
#define __UTILS_H

/*
 * Reads the content of a text file.
 * The user must free the returned char buffer when done with it.
 */
char * readfile(char *filename);

/* 
 * What should be printed to the console.
 * 0 = nothing, 1 = pre-processing, 2 = tokens, 3 = ast, 4 = all.
 */
int dsptree_print_level;

#endif /* __UTILS_H */
