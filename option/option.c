#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include "option.h"


#define INITIALIZE(x) \
  { .label = #x, .isclass = is ## x }

struct {
  const char *label;
  int (*isclass)(int);
} class[] = {
  INITIALIZE(alnum),
  INITIALIZE(alpha),
  INITIALIZE(blank),
  INITIALIZE(cntrl),
  INITIALIZE(digit),
  INITIALIZE(graph),
  INITIALIZE(lower),
  INITIALIZE(print),
  INITIALIZE(punct),
  INITIALIZE(space),
  INITIALIZE(upper),
  INITIALIZE(xdigit),
};

#define TRACK fprintf(stderr, "*** %s:%d\n", __func__, __LINE__);

#define CLASS__NBR  (sizeof(class) / sizeof(*class))


#define ARG__OPT_LONG     "--"
#define ARG__OPT_SHORT    "-"
#define ARG_HELP          "help"
#define FILTER__OPT_LONG  "filter"
#define FILTER__OPT_SHORT "f"
#define SORT__OPT_LONG    "sort"
#define SORT__OPT_SHORT   "s"
#define UPPER__OPT_LONG   "uppercasing"
#define UPPER__OPT_SHORT  "u"

struct option long_options[] = {
    {ARG_HELP, no_argument, 0, 'h'},
    {FILTER__OPT_LONG, required_argument, 0, 'f'},
    {SORT__OPT_LONG, required_argument, 0, 's'},
    {UPPER__OPT_LONG, required_argument, 0, 'u'},
    {0, 0, 0, 0}
};

void option (int argc, char *argv[], int *take, bool *f, char *filter,
             bool *s, char *sort, bool *upper, int *op) {

    int opt;

    while ((opt = getopt_long(argc, argv, "hf:s:u:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                usage(argv);
                exit(EXIT_SUCCESS);
                break;
            case 'f':
                if (optarg == NULL){
                    fprintf(stderr, "Option -f requires an argument.\n");
                    exit(EXIT_FAILURE);
                }
                *f = true; 
                memcpy(filter, optarg, strlen(optarg) + 1); 
                break;
            case 's':
                if (optarg == NULL){
                    fprintf(stderr, "Option -s requires an argument.\n");
                    exit(EXIT_FAILURE);
                }
                *s = true;
                memcpy(sort, optarg, strlen(optarg) + 1);
                break;
            case 'u':
                if (optarg == NULL){
                    fprintf(stderr, "Option -u requires an argument.\n");
                    exit(EXIT_FAILURE);
                }
                *upper = true;
                break;
            default:
                usage(argv);
                exit(EXIT_FAILURE);
                break;
        }
    }
    if (optind < argc) {
        printf("%s", argv[optind]);
        for (int i = optind + 1; i < argc; i++) {
            printf("\t%s", argv[i]);
        }
        printf("\n");
        *op = optind;
        *take = argc - optind;
    }
        if (optind < argc) {

    }
    if (optind == argc){
        printf("No file name specified.\n");
    }  
}

void usage(char *argv[]) {
    printf("Usage: %s [OPTIONS] [FILES]\n", argv[0]);
    printf("Options:\n");
    printf("  --help, -h                Display this help and exit\n");
    printf("  --filter, -f CLASS        Retain only lines containing characters in CLASS\n");
    printf("  --sort, -s WORD           Sort lines according to WORD\n");
    printf("  --uppercasing, -u TOUPPER Converts all characters to uppercase characters\n");
    printf("\n");
    exit(EXIT_SUCCESS);
}


static int ch_keep(const char *filename, int (*charcond)(int)) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        return -1;
    }
    FILE *tmpfile = fopen("tmpfile", "wbx");
    if (tmpfile == NULL) {
        fclose(f);
        return -1;
    }
    int c = fgetc(f);
    while (c != EOF
           && ((charcond(c) && fputc(c, tmpfile) != EOF)
           || !charcond(c))) {
            if (c == '\n') { 
                fputc('\n', tmpfile);
            }
            c = fgetc(f);
    }

    if (ferror(f)) {
        return -1;
    }

    if (fclose(f) != 0) {
        return -1;
    }
    if (fclose(tmpfile) != 0) {
        return -1;
    }
    remove(filename);
    rename("tmpfile", filename);
    return 0;
}


int apply_filter(int optind, int argc, char *argv[], char *className) {
    int (*charcond)(int) = NULL;
    size_t k = 0;
    while (k < CLASS__NBR && strcmp(className, class[k].label) != 0) {
      ++k;
    }
    if (k == CLASS__NBR) {
      return -1;
    }
    charcond = class[k].isclass;

    for(int k = optind; k < argc; ++k) {
        const char * const a = argv[k];
        if (ch_keep(a, charcond) != 0) {
            return -1;
    }
  }
  return 0;
}