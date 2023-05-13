//  Partie implantation du module holdall.

#include "holdall.h"

//  struct holdall, holdall : implantation par liste dynamique simplement
//    chainée.

//  Si la macroconstante HOLDALL_PUT_TAIL est définie et que sa macro-évaluation
//    donne une entier non nul, l'insertion dans la liste a lieu en queue. Dans
//    le cas contraire, elle a lieu en tête.

typedef struct choldall choldall;

struct choldall {
  void *ref;
  choldall *next;
};

struct holdall {
  choldall *head;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  choldall **tailptr;
#endif
  size_t count;
};

holdall *holdall_empty(void) {
  holdall *ha = malloc(sizeof *ha);
  if (ha == NULL) {
    return NULL;
  }
  ha->head = NULL;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  ha->tailptr = &ha->head;
#endif
  ha->count = 0;
  return ha;
}

void holdall_dispose(holdall **haptr) {
  if (*haptr == NULL) {
    return;
  }
  choldall *p = (*haptr)->head;
  while (p != NULL) {
    choldall *t = p;
    p = p->next;
    free(t);
  }
  free(*haptr);
  *haptr = NULL;
}

int holdall_put(holdall *ha, void *ref) {
  choldall *p = malloc(sizeof *p);
  if (p == NULL) {
    return -1;
  }
  p->ref = ref;
#if defined HOLDALL_PUT_TAIL && HOLDALL_PUT_TAIL != 0
  p->next = NULL;
  *ha->tailptr = p;
  ha->tailptr = &p->next;
#else
  p->next = ha->head;
  ha->head = p;
#endif
  ha->count += 1;
  return 0;
}

size_t holdall_count(holdall *ha) {
  return ha->count;
}

int holdall_apply(holdall *ha,
    int (*fun)(void *)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun(p->ref);
    if (r != 0) {
      return r;
    }
  }
  return 0;
}


int holdall_apply_context(holdall *ha,
    void *context, void *(*fun1)(void *context, void *ptr),
    int (*fun2)(void *ptr, void *resultfun1)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun2(p->ref, fun1(context, p->ref));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context2(holdall *ha,
    void *context1, void *(*fun1)(void *context1, void *ptr),
    void *context2, int (*fun2)(void *context2, void *ptr, void *resultfun1)) {
  for (const choldall *p = ha->head; p != NULL; p = p->next) {
    int r = fun2(context2, p->ref, fun1(context1, p->ref));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

// MOVE_HEAD : copie le contenu du pointeur dont l'adressse est 
// spécifié par src vers le pointeur dont l'adresse est dst et 
// met à jour *src en pointant vers l'élement spécifié par next
//  de *src et dst en pointant vers le pointeur de l'élemetn next
// de dst.
#define MOVE_HEAD(dst, src, next)          \
    *dst = *src ;                          \
    *src = (*src)->next ;                  \
    dst  = &(*dst)->next ;                  

// holdall_split : divise la liste d'adresse ha en deux sous-listes 
// d'adresses respectives left et right. Elle itère sur les elements
// de la liste d'adresse ha en déplaçant l'élement de tête vers left
// et le suivant vers right.
static void holdall_split(choldall **ha, choldall **left, choldall **right) {
  while((*ha) != NULL) {
    MOVE_HEAD(left,ha,next) ;
    if ((*ha) != NULL) {
      MOVE_HEAD(right,ha,next) ;
    }
  }
  if (*left == NULL) {
    *right = NULL ;
  }else {
    *left = NULL ;
  }
}

// holdall_merger : fusionne deux sous-listes d'adresses respectives
// left et right en une seule liste d'adresse donnée par ha.
static void holdall_merger(choldall **ha, choldall **left, choldall **right, 
    int (*compar)(const void *, const void *) ) {
  while(*left != NULL || *right != NULL) {
    if (*right == NULL 
    || (*left != NULL && compar((*left)->ref,(*right)->ref) < 0)) {
      MOVE_HEAD(ha,left,next) ;
    }else {
      MOVE_HEAD(ha,right,next) ;
    }
  }
}

// holdall_sort_ : sans effet si la liste d'adresse ha contient
// un élement. tri sinon la liste d'adresse ha selon la fonction
// compar appliquée aux références qui y ont insérées avec succès.
static void holdall_sort_(choldall **ha,
    int (*compar)(const void *, const void *)) {
  if ((*ha)->next == NULL) {
    return ;
  }
  choldall *left ;
  choldall *right ;

  holdall_split(ha,&left,&right) ;
  holdall_sort_(&left,compar) ;
  holdall_sort_(&right,compar) ;
  holdall_merger(ha,&left,&right,compar) ;
}

void holdall_sort(holdall *ha,
    int (*compar)(const void *, const void *)) {
  holdall_sort_(&(ha)->head, compar) ;
}