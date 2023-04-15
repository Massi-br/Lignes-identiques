//  Affiche sur la sortie standard la liste des différentes lignes lues sur
//    le flux associé à f, chaque ligne étant précédé soit :
//     -- de la liste des numéros dans lesquels elle apparait
//     -- ....

//  Limitations :
//  - les mots sont obtenus par lecture sur l'entrée des suites consécutives
//    de longueur maximale mais majorée LINE_LENGTH_MAX de caractères qui ne
//    sont pas de la catégorie isspace ;
//  - toute suite de tels caractères de longueur strictement supérieure à
//    WORD_LENGTH_MAX se retrouve ainsi découpée en plusieurs mots.
//  Attention ! Le point suivant est à retravailler. Le laisser en l'état est
//    contraire aux exigences prônées :
//  - l'avertissement qui figure aux lignes 50-51 est une nuisance si le mot lu
//    a exactement la longueur WORD_LENGTH_MAX.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <locale.h>

#include "hashtable.h"
#include "holdall.h"
#include "option.h"

typedef struct val val;
struct val
{
	size_t frequency;
	size_t capacity;
	long int *lines;
};

#define INITIALIZE(x)               \
	{                               \
		.arg = #x, .isclass = is##x \
	}

struct
{
	const char *arg;
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

#define CLASS__NBR (sizeof(class) / sizeof(*class))

#define LINES__CAPACITY_MIN 2
#define LINES__CAPACITY_MUL 2

//  str_hashfun : l'une des fonctions de pré-hachage conseillées par Kernighan
//  et Pike pour les chaines de caractères.
static size_t str_hashfun(const char *s);

// fun_toupper : converti la chaine associée par s tout carcatère
// lu correspondant à une lettre minuscule en le caractère majuscule
// associé.
static void fun_toupper(char *s);

// apply_filter : stocke dans la chaine de caractère associée à s, tous les
// caractères de la chaine associée à str de longueur len satisfaisant
// la condition décrits par la chaine associée à className qui est l'un
// des suffixes ... des douze test d’appartenance à une catégorie de
// caractères is... de l’en-tête standard <ctype.h>.
// La longueur de s est supposée supérieure à len.
static int apply_filter(const char *str, char *s, size_t len, char *className);

// scptr_display : affiche sur la sortie standard ...
// Renvoie zéro en cas de succès, une valeur non nulle en cas d'échec.
int scptr_display(int *n, const char *s, val *v);
#define TRACK fprintf(stderr, "*** %s:%d\n", __func__, __LINE__);
//  rfree, rfree_ : libère la zone mémoire pointée par ptr et renvoie zéro.
static int rfree(void *ptr);
static int rfree_(void *ptr);
// afree_ : libère la zone mémoire la zone mémoire pointée par ptr->lines
// et renvoie zéro.
static int afree_(val *ptr);

#define LINE_LENGTH_MAX 85

int main(int argc, char *argv[])
{
	bool filter = false;
	char filter_class[256];
	bool sort = false;
	char sort_word[256];
	bool upper = false;
	int n = 0;
	int optind = 0;

	int r = EXIT_SUCCESS;
	hashtable *ht = hashtable_empty((int (*)(const void *, const void *))strcmp,
									(size_t(*)(const void *))str_hashfun);
	holdall *has = holdall_empty();
	holdall *hacptr = holdall_empty();

	if (ht == NULL || has == NULL || hacptr == NULL)
		goto error_capacity;

	printf("Avant option: n = %d\n", n);
	if (option(argc, argv, &n, &filter, filter_class,
			   &sort, sort_word, &upper, &optind) != 0)
		goto error_read;
	printf("Après option: n = %d\n", n);
	if (n == 1)
	{
		FILE *f;
		char *filename = argv[optind];
		if (strcmp(filename, "-") == 0)
		{
			f = stdin;
		}
		else
		{
			f = fopen(filename, "r");
			if (f == NULL)
				goto error_read;
		}
		long int n_line = 1;
		char line[LINE_LENGTH_MAX + 1];
		printf("%s\n", filename);
		while (fgets(line, sizeof(line), f) != NULL)
		{
			size_t len = strlen(line);
			if (len > 0 && line[len - 1] == '\n')
			{
				line[len - 1] = '\0';
				--len;
			}

			if (len > LINE_LENGTH_MAX)
			{
				line[LINE_LENGTH_MAX] = '\0';
				len = LINE_LENGTH_MAX;
			}
			if (upper)
				fun_toupper(line);
			char p[len + 1];
			if (filter)
				if (apply_filter(line, p, len, filter_class) != 0)
					goto error;
			val *value = hashtable_search(ht, filter ? p : line);
			if (value != NULL)
			{
				if (value->frequency == value->capacity)
				{
					if (value->frequency * sizeof(long int) >
						SIZE_MAX / LINES__CAPACITY_MUL)
						goto error_capacity;
					long int *a =
						realloc(value->lines, value->frequency *
												  LINES__CAPACITY_MUL * sizeof(long int));
					if (a == NULL)
						goto error_capacity;
					value->lines = a;
					value->capacity *= LINES__CAPACITY_MUL;
				}
				value->lines[value->frequency] = n_line;
				value->frequency += 1;
			}
			else
			{
				if (len != 0)
				{
					char *s = malloc(len + 1);
					if (s == NULL)
						goto error_capacity;
					strcpy(s, (filter && (strlen(p) != 0)) ? p : line);
					if (holdall_put(has, s) != 0)
					{
						free(s);
						goto error_capacity;
					}
					value = malloc(sizeof *value);
					if (value == NULL)
						goto error_capacity;
					value->lines = malloc(LINES__CAPACITY_MIN *
										  sizeof *(value->lines));
					if (value->lines == NULL)
					{
						free(value);
						goto error_capacity;
					}
					value->frequency = 1;
					value->lines[0] = n_line;
					value->capacity = LINES__CAPACITY_MIN;

					if (holdall_put(hacptr, value) != 0)
					{
						free(value->lines);
						free(value);
						goto error_capacity;
					}
					if (hashtable_add(ht, s, value) == NULL)
						goto error_capacity;
				}
			}
			++n_line;
		}
		if (strcmp(filename, "stdin") != 0)
		{
			if (fclose(f) != 0)
			{
				goto error_write;
			}
		}
	}
	else if (n >= 2)
	{
		size_t ind = 1;
		int j = optind;
		for (int i = optind; i < argc; ++i)
		{
			FILE *f;
			char *filename = argv[i];
			if (strcmp(filename, "-") == 0)
			{
				f = stdin;
			}
			else
			{
				f = fopen(filename, "r");
				if (f == NULL)
				{
					goto error_read;
				}
			}

			printf("%s\t", filename);
			char line[LINE_LENGTH_MAX + 1];
			while (fgets(line, sizeof(line), f) != NULL)
			{
				size_t len = strlen(line);
				if (len != 0 && line[len - 1] == '\n')
				{
					line[len - 1] = '\0';
					--len;
				}

				if (len > LINE_LENGTH_MAX)
				{
					line[LINE_LENGTH_MAX] = '\0';
					len = LINE_LENGTH_MAX;
				}
				if (upper)
				{
					fun_toupper(line);
				}
				char p[len + 1];
				if (filter)
				{
					if (apply_filter(line, p, len, filter_class) != 0)
					{
						goto error;
					}
				}
				val *value = hashtable_search(ht, filter ? p : line);
				if (value != NULL)
				{
					if (value->frequency + 1 == value->capacity)
					{
						if (value->frequency * sizeof(long int) >
							SIZE_MAX / LINES__CAPACITY_MUL)
						{
							goto error_capacity;
						}
						long int *a =
							realloc(value->lines, value->frequency *
													  LINES__CAPACITY_MUL * sizeof(long int));
						if (a == NULL)
						{
							goto error_capacity;
						}
						value->lines = a;
						value->capacity *= LINES__CAPACITY_MUL;
					}
					if (value->lines[ind - 1] == 0)
					{
						value->frequency++;
						value->lines[ind] = 0;
					}
					value->lines[ind - 1]++;
				}
				else
				{
					if (i == j && len != 0)
					{
						char *s = malloc(len + 1);
						if (s == NULL)
						{
							goto error_capacity;
						}
						strcpy(s, (filter && (strlen(p) != 0)) ? p : line);
						if (holdall_put(has, s) != 0)
						{
							free(s);
							goto error_capacity;
						}
						value = malloc(sizeof *value);
						if (value == NULL)
						{
							free(s);
							goto error_capacity;
						}
						value->lines = malloc(LINES__CAPACITY_MIN *
											  sizeof *(value->lines));
						if (value->lines == NULL)
						{
							goto error_capacity;
						}
						value->frequency = 1;
						value->capacity = LINES__CAPACITY_MIN;
						value->lines[0] = 1;
						value->lines[1] = 0;

						if (holdall_put(hacptr, value) != 0)
						{
							free(value->lines);
							free(value);
							goto error_capacity;
						}
						if (hashtable_add(ht, s, value) == NULL)
						{
							goto error_capacity;
						}
					}
				}
			}
			if (strcmp(filename, "-") != 0)
			{
				if (fclose(f) != 0)
				{
					goto error_write;
				}
			}

			++ind;
		}
		printf("\n");
	}

	if (sort)
	{
		if (strcmp("locale", sort_word) == 0)
		{
			setlocale(LC_COLLATE, "");
			holdall_sort(has, (int (*)(const void *, const void *))strcoll);
		}
		else if (strcmp("standard", sort_word) == 0)
		{
			holdall_sort(has, (int (*)(const void *, const void *))strcmp);
		}
	}

	if (holdall_apply_context2(has,
							   ht, (void *(*)(void *, void *))hashtable_search, &n,
							   (int (*)(void *, void *, void *))scptr_display) != 0)
		goto error_write;

#if defined HASHTABLE_STATS && HASHTABLE_STATS != 0
	hashtable_fprint_stats(ht, stderr);
#endif

	goto dispose;
error_capacity:
	fprintf(stderr, "*** Error: Not enough memory\n");
	goto error;
error_read:
	fprintf(stderr, "*** Error: A read error occurs\n");
	goto error;
error_write:
	fprintf(stderr, "*** Error: A write error occurs\n");
	goto error;
error:
	r = EXIT_FAILURE;
	goto dispose;
dispose:
	hashtable_dispose(&ht);
	if (has != NULL)
		holdall_apply(has, rfree);
	holdall_dispose(&has);
	if (hacptr != NULL)
		holdall_apply(hacptr, rfree_);
	holdall_dispose(&hacptr);
	return r;
}

size_t str_hashfun(const char *s)
{
	size_t h = 0;

	for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; ++p)
		h = 37 * h + *p;
	return h;
}

int scptr_display(int *n, const char *s, val *v)
{
	if (*n == 1)
	{
		if (v->frequency > 1)
		{
			for (size_t i = 0; i < v->frequency - 1; ++i)
				printf("%ld, ", v->lines[i]);
			printf("%ld\t", v->lines[v->frequency - 1]);
			printf("%s\n", s);
		}
	}
	else if (*n > 1)
	{
		if (v->frequency == (size_t)(*n))
		{
			for (size_t i = 0; i < v->frequency; ++i)
				printf("%ld\t", v->lines[i]);
			printf("\t");
			printf("%s\n", s);
		}
	}
	return 0;
}

int apply_filter(const char *str, char *s, size_t len, char *className)
{
	int (*charcond)(int) = NULL;
	size_t k = 0;

	while (k < CLASS__NBR && strcmp(className, class[k].arg) != 0)
		++k;
	if (k == CLASS__NBR)
		return -1;
	charcond = class[k].isclass;
	int j = 0;

	for (size_t i = 0; i < len; i++)
	{
		int c = str[i];
		if (charcond(c))
			s[j++] = (char)c;
	}
	s[j] = '\0';
	return 0;
}

void fun_toupper(char *str)
{
	int i = 0;

	while (str[i])
	{
		str[i] = (char)toupper(str[i]);
		++i;
	}
}

int rfree(void *ptr)
{
	free(ptr);
	return 0;
}

int afree_(val *ptr)
{
	free(ptr->lines);
	return 0;
}

int rfree_(void *ptr)
{
	afree_(ptr);
	free(ptr);
	return 0;
}
