// Partie implantation du module option

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "option.h"

#define ARG__OPT_LONG     "--"
#define ARG_HELP          "help"
#define FILTER__OPT_LONG  "filter"
#define SORT__OPT_LONG    "sort"
#define UPPER__OPT_LONG   "uppercasing"

// is_help_requested : teste si l'un des paramètres de la ligne de commande est
// égal à la concaténation des chaines désignées par les macroconstantes
// ARG__OPT_LONG et ARG_HELP
static bool is_help_requested(int argc, char *argv[]);

// usage : affiche l'aide et termine le programme en notifiant un succès à
// l'environnement d'exécution
static void usage(char *argv[]);

// struct option : structure contenant une liste d'options longues qui
// peuvent être utilisées par la fonction "getopt_long" pour parser les
// arguments de la ligne de commande.
struct option long_options[] = {
  {FILTER__OPT_LONG, required_argument, 0, 'f'},
  {SORT__OPT_LONG, required_argument, 0, 's'},
  {UPPER__OPT_LONG, no_argument, 0, 'u'},
  {0, 0, 0, 0}
};

void option_arg(int argc, char *argv[], int *n, bool *f, char *filter,
    bool *s, char *sort, bool *upper, int *op) {
  if (is_help_requested(argc, argv)) {
    usage(argv);
  }
  int opt;
  while ((opt = getopt_long(argc, argv, "f:s:u", long_options, NULL)) != -1) {
    switch (opt) {
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
        fprintf(stderr, "Saisissez « %s --help » pour plus d'informations.\n",
            argv[0]);
        exit(EXIT_FAILURE);
        break;
    }
  }
  *op = optind;
  *n = argc - optind;
  if (*n == 0) {
    *n = 1;
    *op = 0;
  }
}

void usage(char *argv[]) {
  printf("Usage : %s [OPTION]... [FILE]...\n", argv[0]);
  printf("Affiche sur la sortie standard la liste des différentes"
      " ignes lues sur\n");
  printf("un ou plusieurs fichiers dont le(s) nom(s) figure(nt) sur"
      "la ligne de\n");
  printf("commande selon que :\n");
  printf("  -- si le nom d'un fichier figure sur la ligne de commande,"
      "affiche pour chaque ligne\n");
  printf("     de texte non vide apparaissant au moins deux fois dans"
      "le fichier, la liste des\n");
  printf("     numéros dans lesquels elle apparait dans le fichier ainsi"
      " que son contenu.\n");
  printf("  -- si au moins deux noms de fichiers figurent sur la ligne"
      " de commande,\n");
  printf("     pour chaque ligne de texte non vide apparaissant au"
      " moins une fois\n");
  printf("     d’afficher, dans tous les fichiers, le nombre d’occurrences"
      " de la\n");
  printf("     ligne dans chacun des fichiers et le contenu de la ligne.\n\n");
  printf("Sans FICHIER ou quand FICHIER est '-', lire l'entrée standard.\n\n");
  printf("OPTIONS:\n");
  printf("  --help                    Display this help and exit\n");
  printf("  --filter, -f CLASS        Retain only lines containing"
      "characters in CLASS\n");
  printf("  --sort, -s WORD           Sort lines according to WORD\n");
  printf("  --uppercasing, -u         Converts all characters to uppercase"
      " characters\n");
  printf("\n");
  printf("  The value of CLASS is one of the suffixes ... of the twelve"
      "tests for\n"
      "  belonging to a category of is... characters of standard header"
      " <ctype.h> : \n"
      " 'alnum', 'alpha', 'blank', 'upper', 'lower', 'cntrl', 'digit'\n"
      " 'space', 'graph', 'print', 'punct', 'xdigit'\n");
  printf("  The value of WORD is 'standard' for use of strcmp and 'locale'\n"
      "  for use of strcoll.\n\n");
  printf("Limitations\n");
  printf("   les lignes sont obtenus par lecture sur un ou des fichiers dont\n"
      "   le(s) nom(s) figure(nt) sur la ligne de commande de longueur\n"
      "   maximale mais majorée BUFSIZE de caractères.\n");
  printf("   Toute suite de tels caractères de longueur strictement\n"
      "   supérieure à BUFSIZE se retrouve ainsi ignorés.\n\n");
  printf("*** Attention ***\n");
  printf("Les fichiers dont les noms commencent les préfixes '-' ou '--'\n"
      "seront considerés comme des options et non comme des noms de "
      "fichiers !\n");
  exit(EXIT_SUCCESS);
}

bool is_help_requested(int argc, char *argv[]) {
  for (int k = 1; k < argc; ++k) {
    if (strcmp(argv[k], ARG__OPT_LONG ARG_HELP) == 0) {
      return true;
    }
  }
  return false;
}
