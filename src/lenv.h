#ifndef LENV_H_
#define LENV_H_

#include "lval.h"

/** TODO : Reimplement as a hash table
 * This default implementation gets really bad as the dictionnary gets bigger
 * since every operation is O(count). Implementing a proper hash table will
 * raise performance, and allow to compare the 2 classic implementations of hash
 * tables.
 */
struct lenv {
    int count;
    char** syms;
    struct lval** vals;
};

/* Create an environment */
struct lenv* lenv_new();

/* Delete an environment */
void lenv_del(struct lenv* e);

/* Get a value in environment, return lval_err if not found */
struct lval* lenv_get(struct lenv* e, struct lval* k);

/* Put a value in environment */
void lenv_put(struct lenv* e, struct lval* k, struct lval* v);

/* Initialization with builtins */
void lenv_add_builtin(struct lenv* e, char* name, lbuiltin fun);

void lenv_add_builtins(struct lenv* e);

#endif /* LENV_H_ */
