#ifndef EVALUATION_H_
#define EVALUATION_H_

#include "lval.h"
#include "mpc.h"

struct lenv;

struct lval* builtin_add(struct lenv* e, struct lval* x);
struct lval* builtin_sub(struct lenv* e, struct lval* x);
struct lval* builtin_mul(struct lenv* e, struct lval* x);
struct lval* builtin_div(struct lenv* e, struct lval* x);
struct lval* builtin_mod(struct lenv* e, struct lval* x);
struct lval* builtin_floor(struct lenv* e, struct lval* x);
struct lval* builtin_head(struct lenv* e, struct lval* x);
struct lval* builtin_tail(struct lenv* e, struct lval* x);
struct lval* builtin_join(struct lenv* e, struct lval* x);
struct lval* builtin_eval(struct lenv* e, struct lval* x);
struct lval* builtin_list(struct lenv* e, struct lval* x);
struct lval* builtin_len(struct lenv* e, struct lval* x);
struct lval* builtin_cons(struct lenv* e, struct lval* x);
struct lval* builtin_init(struct lenv* e, struct lval* x);

#endif  // EVALUATION_H_
