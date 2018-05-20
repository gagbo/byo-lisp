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

struct lval* builtin_def(struct lenv* e, struct lval* x);
struct lval* builtin_put(struct lenv* e, struct lval* x);
struct lval* builtin_exit(struct lenv* e, struct lval* x);

struct lval* builtin_gt(struct lenv* e, struct lval* a);
struct lval* builtin_ge(struct lenv* e, struct lval* a);
struct lval* builtin_lt(struct lenv* e, struct lval* a);
struct lval* builtin_le(struct lenv* e, struct lval* a);

struct lval* builtin_eq(struct lenv* e, struct lval* a);
struct lval* builtin_ne(struct lenv* e, struct lval* a);

struct lval* builtin_lambda(struct lenv* e, struct lval* x);
/** Emulate def {fun} (\ {args body} {def (head args) (\ (tail args) body)})
 *
 * This allows to define functions like this :
 *
 * fun {add_together x y} {+ x y}
 * fun {example x & xs} {* x (eval(len xs))}
 *
 * The function manually constructs a lambda lval using lval_lambda, and then
 * uses lval_add to build a proper struct lval to pass to builtin_def. The
 * addition of the binding to the env is deferred to builtin_def
 *
 * This means that fun is not checking for redefinition of builtins, only
 * builtin_def is.
 */
struct lval* builtin_fun(struct lenv* e, struct lval* x);

#endif  // EVALUATION_H_
