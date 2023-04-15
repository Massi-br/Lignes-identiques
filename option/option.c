#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include "option.h"

#define TRACK fprintf(stderr, "*** %s:%d\n", __func__, __LINE__);

#define ARG__OPT_LONG "--"
#define ARG__OPT_SHORT "-"
#define ARG_HELP "help"
#define FILTER__OPT_LONG "filter"
#define FILTER__OPT_SHORT "f"
#define SORT__OPT_LONG "sort"
#define SORT__OPT_SHORT "s"
#define UPPER__OPT_LONG "uppercasing"
#define UPPER__OPT_SHORT "u"

struct option long_options[] = {
	{ ARG_HELP,	    no_argument,       0, 'h' },
	{ FILTER__OPT_LONG, required_argument, 0, 'f' },
	{ SORT__OPT_LONG,   required_argument, 0, 's' },
	{ UPPER__OPT_LONG,  no_argument,       0, 'u' },
	{ 0,		    0,		       0, 0   }
};
int option(int argc, char *argv[], int *n, bool *f, char *filter,
	   bool *s, char *sort, bool *upper, int *op)
{
	int opt;

	while ((opt = getopt_long(argc, argv, "hf:s:u", long_options, NULL)) != -1) {
		switch (opt) {
		case 'h':
			usage(argv);
			return 0;
			break;
		case 'f':
			*f = true;
			memcpy(filter, optarg, strlen(optarg) + 1);
			break;
		case 's':
			*s = true;
			memcpy(sort, optarg, strlen(optarg) + 1);
			break;
		case 'u':
			*upper = true;
			break;
		default:
			usage(argv);
			break;
		}
	}

	*op = optind;
	*n = argc - optind;

	if (*n == 0) {
		printf("No file name specified.\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

void usage(char *argv[])
{
	printf("Usage: %s [OPTIONS] [FILES]\n", argv[0]);
	printf("Options:\n");
	printf("  --help, -h                Display this help and exit\n");
	printf("  --filter, -f CLASS        Retain only lines containing characters in CLASS\n");
	printf("  --sort, -s WORD           Sort lines according to WORD\n");
	printf("  --uppercasing, -u TOUPPER Converts all characters to uppercase characters\n");
	printf("\n");
	exit(EXIT_SUCCESS);
}
