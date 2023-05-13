// Partie interface du module option pour la gestion
// des arguments sur la ligne de commande...

#ifndef OPTION__H
#define OPTION__H

#include <stdlib.h>

#define ARG__OPT_SHORT    "-"

// option_arg : Traite les arguments passés sur la ligne de commande
// selon le module getopt et affecte aux chaînes pointées par “filter” et “sort”
// les
// valeurs correspondantes aux arguments des options, si ces derniers sont
// spécifiés
// en ligne de commande tout en mettant à true les valeurs pointées par f , s et
// upper.
// Stocke dans la variable pointée par op l'indice du premier fichier spécifié
// sur la
// ligne de commande ou à 1 si aucun fichier n'est spécifié en ligne de
// commande, et dans
// la variable pointée par n le nombre de fichiers spécifiés en ligne de
// commande.
// si aucun argument n'a été fourni , la fonction suppose qu'un seul fichier
// doit être
// traiter et affecte la valeur 1 à *n et 0 à *op. le nombre d'arguments est
// spécifié
// par argc, et les valeurs des arguments par argv.
extern void option_arg(int argc, char *argv[], int *n, bool *f, char *filter,
    bool *s, char *sort, bool *upper, int *op);

#endif
