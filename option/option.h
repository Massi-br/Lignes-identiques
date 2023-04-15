#ifndef OPTION__H
#define OPTION__H

#include <stdlib.h>




extern void usage(char *argv[]);

extern int option (int argc, char *argv[], int *take, bool *f, char *filter, 
        bool *s, char *sort, bool *upper, int *op);

#endif