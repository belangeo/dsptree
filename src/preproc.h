#ifndef __PREPROC_H
#define __PREPROC_H

/*
 * Pre-processing function. Adds loaded librairies and removes comments.
 * The returned char buffer must be freed when done with it.
 */
 char * process(char *input);

#endif /* __PREPROC_H */
