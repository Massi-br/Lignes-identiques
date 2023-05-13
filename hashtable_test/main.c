//  Affiche sur la sortie standard la liste des différentes lignes lues sur
//  un ou plusieurs fichiers dont le(s) nom(s) figure(nt) sur la ligne de
//  commande selon que :
//     -- si le nom d'un fichier figure sur la ligne de commande, affiche pour
// chaque ligne
//        de texte non vide apparaissant au moins deux fois dans le fichier, la
// liste des
//        numéros dans lesquels elle apparait dans le fichier ainsi que son
// contenu.
//     -- si au moins deux noms de fichiers figurent sur la ligne de commande,
//        pour chaque ligne de texte non vide apparaissant au moins une fois
//        d’afficher, dans tous les fichiers, le nombre d’occurrences de la
//        ligne dans chacun des fichiers et le contenu de la ligne.

//  Limitations :
//  - les lignes sont obtenus par lecture sur un ou des fichiers dont le(s)
// nom(s)
//    figure(nt) sur la ligne de commande de longueur maximale mais majorée
//    BUFSIZE de caractères.
//  - toute suite de tels caractères de longueur strictement supérieure à
//    BUFSIZE se retrouve ainsi ignorés.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>

#include "hashtable.h"
#include "holdall.h"
#include "option.h"

typedef struct val val;
struct val {
  size_t frequency;
  size_t capacity;
  long int *lines;
};

#define INITIALIZE(x) {             \
    .arg = #x, .isclass = is ## x   \
}

struct {
  const char *arg;
  int (*isclass)(int);
} class[] = {
  INITIALIZE(alnum),
  INITIALIZE(alpha),
  INITIALIZE(blank),
  INITIALIZE(upper),
  INITIALIZE(lower),
  INITIALIZE(cntrl),
  INITIALIZE(digit),
  INITIALIZE(space),
  INITIALIZE(graph),
  INITIALIZE(print),
  INITIALIZE(punct),
  INITIALIZE(xdigit),
};

#define CLASS__NBR (sizeof(class) / sizeof(*class))

#define LINES__CAPACITY_MIN 2
#define LINES__CAPACITY_MUL 2

//  str_hashfun : l'une des fonctions de pré-hachage conseillées par Kernighan
//  et Pike pour les chaines de caractères.
static size_t str_hashfun(const char *s);

// apply_filter : return true si le caractère c satisfait
// la condition décrits par la chaine associée à className qui est l'un
// des suffixes ... des douze test d’appartenance à une catégorie de
// caractères is... de l’en-tête standard <ctype.h>. return false
// si la condition précedente n'est pas satisfaite ou si la chaine associée
// à className n'est pas l'un des suffixes des douze test d'appartenance
// à une cathégorie de caractères is... de l'en-tête standard <ctype.h>
// et met à true la variable pointée par is_arg_valid.
static bool apply_filter(int c, char *className, bool *is_arg_valid);

// lines_display : affiche sur la sortie standard les informations
// correspondant aux critères suivants :
// - Si le nombre d'occurrences de la valeur pointée par n
//   est égal à 1 et la fréquence de v est supérieure à 2,
//   affiche les numéros de ligne de toutes les occurrences
//   de v dont la fréquence est supérieure à 2, ainsi que
//   les lignes associées à s.
// - Sinon si le nombre d'occurrences de la valeur pointée par n
//   est supérieur à 2 et la fréquence de v est
//   égale à *n, affiche toutes les valeurs associés à v->lines
//   ainsi que les lignes associées à s dont la fréquence
//   d'apparition minimale de v est égale à *n.
// Renvoie zéro.
static int lines_display(int *n, const char *s, val *v);

// files_display : Parcourt le tableau associé à argv , de
// l'index spécifié par optind jusqu'à la variable spécifié
// par argc. Pour chaque élément du tableau de chaines de
// caractères argv, si la chaîne de caractères correspondante
// n'est pas égale à la macro ARG__OPT_SHORT et si optind est différent
// de zéro, la fonction affiche la chaîne  suivie d'une tabulation.
// affiche une nouvelle ligne avant de terminer l'exécution.
static void files_display(char *argv[], int argc, int optind);
//  rfree, rfree_ : libère la zone mémoire pointée par ptr et renvoie zéro.
static int rfree(void *ptr);
static int rfree_(void *ptr);
// afree_ : libère la zone mémoire la zone mémoire pointée par ptr->lines
// et renvoie zéro.
static int afree_(val *ptr);

int main(int argc, char *argv[]) {
  int r = EXIT_SUCCESS;
  hashtable *ht = hashtable_empty((int (*)(const void *, const void *))strcmp,
      (size_t (*)(const void *))str_hashfun);
  holdall *has = holdall_empty();
  holdall *hacptr = holdall_empty();
  if (ht == NULL || has == NULL || hacptr == NULL) {
    goto error_capacity;
  }
  bool filter = false;
  char filter_class[UCHAR_MAX];
  bool sort = false;
  char sort_word[UCHAR_MAX];
  bool upper = false;
  int n = 0;
  int optind = 0;
  option_arg(argc, argv, &n, &filter, filter_class,
      &sort, sort_word, &upper, &optind);
  if (n == 1) {
    char *filename = argv[optind];
    FILE *f;
    bool is_stdin = false;
    if (strcmp(filename, ARG__OPT_SHORT) == 0 || optind == 0) {
      is_stdin = true;
      f = stdin;
    } else {
      f = fopen(filename, "r");
      if (f == NULL) {
        goto error_opening;
      }
    }
    long int n_line = 1;
    int c;
    while ((c = fgetc(f)) != EOF) {
      char line[BUFSIZ];
      size_t len = 0;
      while (c != EOF && c != '\n') {
        if (len == BUFSIZ) {
          fprintf(stderr, "***warning : Only prefix of length %d"
              "of line number %ld is stored\n",
              BUFSIZ, n_line);
        } else {
          if (upper) {
            c = toupper(c);
          }
          if (filter) {
            bool is_arg_valid = true;
            if (apply_filter(c, filter_class, &is_arg_valid)) {
              line[len] = (char) c;
              ++len;
            }
            if (!is_arg_valid) {
              fprintf(stderr, "*** Error: invalid argument '%s'\n",
                  filter_class);
              if (!is_stdin) {
                fclose(f);
              }
              goto ask_help;
            }
          } else {
            line[len] = (char) c;
            ++len;
          }
        }
        c = fgetc(f);
      }
      line[len] = '\0';
      val *value = hashtable_search(ht, line);
      if (value != NULL) {
        if (value->frequency == value->capacity) {
          if (value->frequency * sizeof(long int)
              > SIZE_MAX / LINES__CAPACITY_MUL) {
            if (!is_stdin) {
              fclose(f);
            }
            goto error_capacity;
          }
          long int *a
            = realloc(value->lines, value->frequency
              * LINES__CAPACITY_MUL * sizeof(long int));
          if (a == NULL) {
            if (!is_stdin) {
              fclose(f);
            }
            goto error_capacity;
          }
          value->lines = a;
          value->capacity *= LINES__CAPACITY_MUL;
        }
        value->lines[value->frequency] = n_line;
        value->frequency += 1;
      } else {
        if (len != 0) {
          char *s = malloc(len + 1);
          if (s == NULL) {
            if (!is_stdin) {
              fclose(f);
            }
            goto error_capacity;
          }
          strcpy(s, line);
          if (holdall_put(has, s) != 0) {
            free(s);
            if (!is_stdin) {
              fclose(f);
            }
            goto error_capacity;
          }
          value = malloc(sizeof *value);
          if (value == NULL) {
            if (!is_stdin) {
              fclose(f);
            }
            goto error_capacity;
          }
          value->lines = malloc(LINES__CAPACITY_MIN
              * sizeof *(value->lines));
          if (value->lines == NULL) {
            free(value);
            if (!is_stdin) {
              fclose(f);
            }
            goto error_capacity;
          }
          value->frequency = 1;
          value->lines[0] = n_line;
          value->capacity = LINES__CAPACITY_MIN;
          if (holdall_put(hacptr, value) != 0) {
            free(value->lines);
            free(value);
            if (!is_stdin) {
              fclose(f);
            }
            goto error_capacity;
          }
          if (hashtable_add(ht, s, value) == NULL) {
            if (!is_stdin) {
              fclose(f);
            }
            goto error_capacity;
          }
        }
      }
      ++n_line;
    }
    if (!feof(f)) {
      goto error_read;
    }
    if (!is_stdin) {
      if (fclose(f) != 0) {
        goto error_closing;
      }
    }
  } else if (n >= 2) {
    size_t ind = 1;
    for (int i = optind; i < argc; ++i) {
      char *filename = argv[i];
      FILE *f;
      int k;
      if ((k = strcmp(filename, ARG__OPT_SHORT)) == 0) {
        f = stdin;
      } else {
        f = fopen(filename, "r");
        if (f == NULL) {
          goto error_opening;
        }
      }
      int c;
      while ((c = fgetc(f)) != EOF) {
        char line[BUFSIZ];
        size_t len = 0;
        while (c != EOF && c != '\n') {
          if (len == BUFSIZ) {
            fprintf(stderr,
                "***warning : Only prefix of length %d is stored\n",
                BUFSIZ);
          } else {
            if (upper) {
              c = toupper(c);
            }
            if (filter) {
              bool is_arg_valid = true;
              if (apply_filter(c, filter_class, &is_arg_valid)) {
                line[len] = (char) c;
                ++len;
              }
              if (!is_arg_valid) {
                fprintf(stderr, "*** Error: invalid argument '%s'\n",
                    filter_class);
                if (k != 0) {
                  fclose(f);
                }
                goto ask_help;
              }
            } else {
              line[len] = (char) c;
              ++len;
            }
          }
          c = fgetc(f);
        }
        line[len] = '\0';
        val *value = hashtable_search(ht, line);
        if (value != NULL) {
          if (value->frequency + 1 == value->capacity) {
            if (value->capacity * sizeof(long int)
                > SIZE_MAX / LINES__CAPACITY_MUL) {
              if (k != 0) {
                fclose(f);
              }
              goto error_capacity;
            }
            long int *a
              = realloc(value->lines, value->capacity
                * LINES__CAPACITY_MUL * sizeof(long int));
            if (a == NULL) {
              if (k != 0) {
                fclose(f);
              }
              goto error_capacity;
            }
            value->lines = a;
            value->capacity *= LINES__CAPACITY_MUL;
          }
          if (value->lines[ind - 1] == 0) {
            value->frequency++;
            value->lines[ind] = 0;
          }
          if (value->frequency==ind) {
            value->lines[ind - 1]++;
          }    
        } else {
          if (i == optind && len != 0) {
            char *s = malloc(len + 1);
            if (s == NULL) {
              if (k != 0) {
                fclose(f);
              }
              goto error_capacity;
            }
            strcpy(s, line);
            if (holdall_put(has, s) != 0) {
              free(s);
              if (k != 0) {
                fclose(f);
              }
              goto error_capacity;
            }
            value = malloc(sizeof *value);
            if (value == NULL) {
              if (k != 0) {
                fclose(f);
              }
              goto error_capacity;
            }
            value->lines = malloc(LINES__CAPACITY_MIN
                * sizeof *(value->lines));
            if (value->lines == NULL) {
              free(value);
              if (k != 0) {
                fclose(f);
              }
              goto error_capacity;
            }
            value->frequency = 1;
            value->capacity = LINES__CAPACITY_MIN;
            value->lines[0] = 1;
            value->lines[1] = 0;
            if (holdall_put(hacptr, value) != 0) {
              free(value->lines);
              free(value);
              if (k != 0) {
                fclose(f);
              }
              goto error_capacity;
            }
            if (hashtable_add(ht, s, value) == NULL) {
              if (k != 0) {
                fclose(f);
              }
              goto error_capacity;
            }
          }
        }
      }
      if (!feof(f)) {
        goto error_read;
      }
      if (k != 0) {
        if (fclose(f) != 0) {
          goto error_closing;
        }
      }
      ++ind;
    }
  }
  if (sort) {
    if (strcmp("locale", sort_word) == 0) {
      setlocale(LC_COLLATE, "");
      holdall_sort(has, (int (*)(const void *, const void *))strcoll);
    } else if (strcmp("standard", sort_word) == 0) {
      holdall_sort(has, (int (*)(const void *, const void *))strcmp);
    } else {
      fprintf(stderr, "*** Error: invalid argument '%s'\n", sort_word);
      goto ask_help;
    }
  }
  files_display(argv, argc, optind);
  if (holdall_apply_context2(has,
      ht, (void *(*)(void *, void *))hashtable_search, &n,
      (int (*)(void *, void *, void *))lines_display) != 0) {
    goto error_write;
  }
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
error_opening:
  fprintf(stderr, "*** Error: Error when opening file\n");
  goto error;
error_closing:
  fprintf(stderr, "*** Error: Error when closing file\n");
  goto error;
ask_help:
  fprintf(stderr, "Saisissez « %s --help » pour plus d'informations.\n",
      argv[0]);
  goto error;
error:
  r = EXIT_FAILURE;
  goto dispose;
dispose:
  hashtable_dispose(&ht);
  if (has != NULL) {
    holdall_apply(has, rfree);
  }
  holdall_dispose(&has);
  if (hacptr != NULL) {
    holdall_apply(hacptr, rfree_);
  }
  holdall_dispose(&hacptr);
  return r;
}

size_t str_hashfun(const char *s) {
  size_t h = 0;
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; ++p) {
    h = 37 * h + *p;
  }
  return h;
}

int lines_display(int *n, const char *s, val *v) {
  if (*n == 1) {
    if (v->frequency > 1) {
      for (size_t i = 0; i < v->frequency - 1; ++i) {
        printf("%ld, ", v->lines[i]);
      }
      printf("%ld\t", v->lines[v->frequency - 1]);
      printf("%s\n", s);
    }
  } else if (*n > 1) {
    if (v->frequency == (size_t) (*n)) {
      for (size_t i = 0; i < v->frequency; ++i) {
        printf("%ld\t", v->lines[i]);
      }
      printf("\t");
      printf("%s\n", s);
    }
  }
  return 0;
}

bool apply_filter(int c, char *className, bool *is_arg_valid) {
  int (*charcond)(int) = NULL;
  size_t k = 0;
  while (k < CLASS__NBR && strcmp(className, class[k].arg) != 0) {
    ++k;
  }
  if (k == CLASS__NBR) {
    *is_arg_valid = false;
    return false;
  }
  charcond = class[k].isclass;
  return charcond(c);
}

void files_display(char *argv[], int argc, int optind) {
  for (int i = optind; i < argc; ++i) {
    if (strcmp(argv[i], ARG__OPT_SHORT) != 0 && optind != 0) {
      printf("%s\t", argv[i]);
    }
  }
  printf("\n");
}

int rfree(void *ptr) {
  free(ptr);
  return 0;
}

int afree_(val *ptr) {
  free(ptr->lines);
  return 0;
}

int rfree_(void *ptr) {
  afree_(ptr);
  free(ptr);
  return 0;
}
