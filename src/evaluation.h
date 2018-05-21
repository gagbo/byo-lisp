/** This file contains all the builtin functions of the lisp implementation */
#ifndef EVALUATION_H_
#define EVALUATION_H_

#include "lval.h"
#include "mpc.h"

struct lenv;

/** Computes (+ x...) where x is the list of arguments */
struct lval* builtin_add(struct lenv* e, struct lval* x);

/** Computes (- x...) where x is the list of arguments
 * If there's only one argument, then it return -x
 */
struct lval* builtin_sub(struct lenv* e, struct lval* x);

/** Computes (* x...) where x is the list of arguments */
struct lval* builtin_mul(struct lenv* e, struct lval* x);

/** Computes (/ x...) where x is the list of arguments
 * The function returns an error if one of the dividers is 0
 */
struct lval* builtin_div(struct lenv* e, struct lval* x);

/** Computes (% x...) where x is the list of arguments */
struct lval* builtin_mod(struct lenv* e, struct lval* x);

/** Computes (floor x) where x is one number */
struct lval* builtin_floor(struct lenv* e, struct lval* x);

/** Computes (head x) where x is one list given as Q-Expression
 * It returns a one-element Q-Expression with the first element of x
 * Returns an error if an empty list is given
 */
struct lval* builtin_head(struct lenv* e, struct lval* x);

/** Computes (tail x) where x is one list given as Q-Expression
 * It returns a list with all the last elements of x.
 * Returns an error is an empty list is given
 */
struct lval* builtin_tail(struct lenv* e, struct lval* x);

/** Computes (join x...) where x is a list of arguments
 * All the lists are joined together in one expression
 */
struct lval* builtin_join(struct lenv* e, struct lval* x);

/** Computes (eval x) where x is one Expression
 * Change type of x to S-Expr so it can be evaluated
 */
struct lval* builtin_eval(struct lenv* e, struct lval* x);

/** Computes (list x...) where x is a list of arguments
 * Creates a Q-Expression from a list of arguments
 */
struct lval* builtin_list(struct lenv* e, struct lval* x);

/** Computes (len x) where x is a list given as Q-Expression
 * Returns the length of the list enclosed in a Q-Expression
 * Returns 0 for an empty list
 */
struct lval* builtin_len(struct lenv* e, struct lval* x);

/** Computes (cons x y) where x and y are one lists
 * Prepends x to the list y, and returns the Q-Expression we obtain
 * y must be a Q-Expression
 */
struct lval* builtin_cons(struct lenv* e, struct lval* x);

/** Computes (init x) where x is a list given as Q-Expression
 * Returns all but the last element of x
 * Returns an error for an empty list
 */
struct lval* builtin_init(struct lenv* e, struct lval* x);

/** Computes (def {symbols} {values}) where symbols and values are lists
 * Defines new values for the given symbols in the global environment
 * Returns an error if trying to redefine one builtin
 * Returns an error if there are not as many elements in symbols as in values
 */
struct lval* builtin_def(struct lenv* e, struct lval* x);

/** Computes (= {symbols} {values}) where symbols and values are lists
 * Defines new values for the given symbols in the innermost environment
 * Returns an error if trying to redefine at least one builtin
 * Returns an error if there are not as many elements in symbols as in values
 */
struct lval* builtin_put(struct lenv* e, struct lval* x);

/** Computes (> x y) where x, y are evaluable elements
 * Returns t if x > y, f otherwise
 */
struct lval* builtin_gt(struct lenv* e, struct lval* a);

/** Computes (>= x y) where x, y are evaluable elements
 * Returns t if x >= y, f otherwise
 */
struct lval* builtin_ge(struct lenv* e, struct lval* a);

/** Computes (< x y) where x, y are evaluable elements
 * Returns t if x < y, f otherwise
 */
struct lval* builtin_lt(struct lenv* e, struct lval* a);

/** Computes (<= x y) where x, y are evaluable elements
 * Returns t if x <= y, f otherwise
 */
struct lval* builtin_le(struct lenv* e, struct lval* a);

/** Computes (== x y) where x, y are values
 * Returns t if x == y (element-wise in case of lists or strings), f otherwise
 */
struct lval* builtin_eq(struct lenv* e, struct lval* a);

/** Computes (!= x y) where x, y are evaluable elements
 * Returns t if x != y (element-wise in case of lists or strings), f otherwise
 */
struct lval* builtin_ne(struct lenv* e, struct lval* a);

/** Computes (|| x) where x is a list of bool
 * Returns t if at least one member of x is t, f otherwise
 */
struct lval* builtin_or(struct lenv* e, struct lval* a);

/** Computes (&& x) where x is a list of bool
 * Returns t if all elements of x are t, f otherwise
 */
struct lval* builtin_and(struct lenv* e, struct lval* a);

/** Computes (! x) where x is one bool
 * Returns t if x is f, f otherwise
 */
struct lval* builtin_not(struct lenv* e, struct lval* a);

/** Computes (cond {val1} {val_t} {val_f}) where val1 is a bool, val_t and val_f
 * are evaluable
 * Returns val_t if val1 == t, val_f otherwise
 */
struct lval* builtin_cond(struct lenv* e, struct lval* a);

/** Computes (\ {args} {body}) where args is a list of symbols and body is
 * evaluable
 * Returns a function objects which will map the arguments in the body before
 * returning the value of evaluated body
 */
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

/** Load a file and evaluate it
 * Returns an error with the message if the parsing went wrong
 */
struct lval* builtin_load(struct lenv* e, struct lval* a);

/** Print a value on stdout */
struct lval* builtin_print(struct lenv* e, struct lval* a);

/** Return an error message from a string */
struct lval* builtin_error(struct lenv* e, struct lval* a);

#endif  // EVALUATION_H_
