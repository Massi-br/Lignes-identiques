//  Affiche sur la sortie standard la liste des différents lignes lus sur
//    le flux associé à f, chaque ligne étant précédé soit : 
//     -- de la liste des numéros dans lesquels elle apparait
//     -- ....


//  Limitations :
//  - les mots sont obtenus par lecture sur l'entrée des suites consécutives
//    de longueur maximale mais majorée WORD_LENGTH_MAX de caractères qui ne
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

#include "hashtable.h"
#include "holdall.h"
#include "option.h"


typedef struct val val;
struct val{
    size_t frequency;
    long int *lines;
    size_t capacity;
};

#define LINES__CAPACITY_MIN 1
#define LINES__CAPACITY_MUL 2

//  str_hashfun : l'une des fonctions de pré-hachage conseillées par Kernighan
//    et Pike pour les chaines de caractères.
static size_t str_hashfun(const char *s);


int scptr_display(int *n, const char *s, val *v);

//  rfree : libère la zone mémoire pointée par ptr et renvoie zéro.
static int rfree(void *ptr);

// 
// static int rfree__(val *v);

// #define STR(s)  #s
// #define XSTR(s) STR(s)

#define LINE_LENGTH_MAX 85

int main(int argc, char *argv[]) {

  bool filter = false;
  char filter_class[256];
  bool sort = false;
  char sort_word[256];
  bool upper = false;
  int  n = 0;
  int optind = 0;

  option(argc, argv, &n, &filter, filter_class, &sort, sort_word, &upper, &optind);

  int r = EXIT_SUCCESS;
  hashtable *ht = hashtable_empty((int (*)(const void *, const void *))strcmp,
      (size_t (*)(const void *))str_hashfun);
  holdall *has = holdall_empty();
  holdall *hacptr = holdall_empty();
  if (ht == NULL
      || has == NULL
      || hacptr == NULL) {
    goto error_capacity;
  }
  // printf("%d\n",n);
  // printf("%s\n",argv[optind]);

  if (n == 1) {
  
    char *filename = argv[optind];
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
      goto error_read;
    }

    long int n_line = 1;
    char line[LINE_LENGTH_MAX + 1];
    printf("%s\n",argv[optind]);
    while(fgets(line, sizeof(line), f) != NULL) {
      size_t len = strlen(line);
      if (len > 0 && line[len-1] == '\n') {
        line[len-1] = '\0';
        --len;
      }

      if (len > LINE_LENGTH_MAX) {
          line[LINE_LENGTH_MAX] = '\0';
          len = LINE_LENGTH_MAX;
      }
      
      val *value =  hashtable_search(ht, line);
      // TRACK
      if (value != NULL) {
        if (value->frequency == value->capacity) {
          if (value->capacity * sizeof (long int) >
              SIZE_MAX / LINES__CAPACITY_MUL) {
                goto error_capacity;
          }
          long int *a =
              realloc(value->lines, value->capacity * 
              LINES__CAPACITY_MUL * sizeof (long int));
          if (a == NULL) {
            goto error_capacity;
          }
          value->lines = a;
          value->capacity *= LINES__CAPACITY_MUL;
        }
        // TRACK
        value->lines[value->frequency] = n_line;
        value->frequency += 1;

      }else {
        if (len != 0) { 
          char *s = malloc(len + 1);
          if (s == NULL) {
            goto error_capacity;
          }
          strcpy(s, line);
          
          if (holdall_put(has, s) != 0) {
            free(s);
            goto error_capacity;
          }
          value = malloc(sizeof *value);
          if (value == NULL) {
            goto error_capacity;
          }
          value->lines = malloc(LINES__CAPACITY_MIN *
          sizeof *(value->lines));
          if (value->lines == NULL) {
            free(value);
            goto error_capacity;
          }
          value->frequency = 1;
          value->lines[0] = n_line;
          value->capacity = LINES__CAPACITY_MIN;

          if (holdall_put(hacptr, value) != 0) {
            free(value->lines);
            free(value);
            goto error_capacity;
          }
          if (hashtable_add(ht, s, value) == NULL) {
            goto error_capacity;
          }
        }
      }
      ++n_line;
    }
    if (fclose(f) != 0) {
      goto error_write;
    }

  }else if (n >= 2) {
    for(int i = optind; i < n; ++i) {
            char *filename = argv[i];
            FILE *f = fopen(filename, "r");
            if (f == NULL) {
                goto error_read;
            }
        
        bool is_in = true;
        char line[LINE_LENGTH_MAX + 1];
        while(fgets(line, sizeof(line), f) != NULL) {
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
                --len;
            }

            if (len > LINE_LENGTH_MAX) {
                line[LINE_LENGTH_MAX] = '\0';
                len = LINE_LENGTH_MAX;
            }
            val *value =  hashtable_search(ht, line);
            // TRACK
            if (value != NULL) {
              if (value->frequency == value->capacity) {
                if (value->capacity * sizeof (long int) >
                    SIZE_MAX / LINES__CAPACITY_MUL) { 
                      goto error_capacity;
                }
              long int *a =
                realloc(value->lines, value->capacity * 
              LINES__CAPACITY_MUL * sizeof (long int));
              if (a == NULL) {
                goto error_capacity;
              }
              value->lines = a;
              value->capacity *= LINES__CAPACITY_MUL;
            }
            if (is_in) {
              value->frequency++;
              is_in = false;
            }
            value->lines[value->frequency - 1]++;

      }else {
        if (len != 0) {
          char *s = malloc(len + 1);
          if (s == NULL) {
            goto error_capacity;
          }
          strcpy(s, line);
          if (holdall_put(has, s) != 0) {
            free(s);
            goto error_capacity;
          }
          value = malloc(sizeof *value);
          if (value == NULL) {
            free(s);
            goto error_capacity;
          }
          value->lines = malloc(LINES__CAPACITY_MIN *
          sizeof *(value->lines));
          if (value->lines == NULL) {
            free(s);
            free(value);
            goto error_capacity;
          }
          value->frequency = 1;
          value->capacity = LINES__CAPACITY_MIN;
          value->lines[value->frequency - 1] = 1;
          is_in = false;

          if (holdall_put(hacptr, value) != 0) {
            free(value);
            goto error_capacity;
          }
          if (hashtable_add(ht, s, value) == NULL) {
            goto error_capacity;
          }
      }
    }
            // to be continued ...
    }
    if (fclose(f) != 0) {
      goto error_write;
    }
    }
  }

  #if defined HOLDALL_WANT_EXT && HOLDALL_WANT_EXT != 0
    holdall_sort(has, (int (*)(const void *, const void *))strcmp);
  #endif
  if (holdall_apply_context2(has,
      ht, (void *(*)(void *, void *))hashtable_search,&n,
      (int (*)(void *, void *, void *))scptr_display) != 0) {
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
  error:
    r = EXIT_FAILURE;
    goto dispose;
  dispose:
    hashtable_dispose(&ht);
    if (has != NULL) {
      holdall_apply(has, rfree);
    }
    holdall_dispose(&has);
    // desalouer le tableau allouer dans la structure;
    if (hacptr != NULL) {
      holdall_apply(hacptr, rfree);
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

int scptr_display(int *n, const char *s, val *v) {
  if (*n == 1) {
    // printf("%s\n",argv[*op]);
    if (v->frequency > 1) {
      for(size_t i = 0; i < v->frequency - 1; ++i) {
        printf("%ld, ",v->lines[i]);
      }
      printf("%ld\t",v->lines[v->frequency - 1]);
      printf("%s\n",s);
    }
  }else if (*n > 1) {
    // for(int i = *op; i < *n; ++i) {
    //   printf("%s\t",argv[i]);
    // }
    if (v->frequency == (size_t)*n) {
       for(size_t i = 0; i < v->frequency; ++i) {
        printf("%ld, ",v->lines[i]);
      }
      printf("\t");
      printf("%s\n",s);
    }
  }
  return 0;
}

int rfree(void *ptr) {
  free(ptr);
  return 0;
}
