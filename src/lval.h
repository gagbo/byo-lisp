#ifndef LVAL_H_
#define LVAL_H_

#include "evaluation.h"
#include "mpc.h"

struct lval;
struct lenv;
typedef struct lval*(*lbuiltin)(struct lenv*, struct lval*);

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

struct lval {
    int type;
    double num;

    char* err;
    char* sym;
    lbuiltin fun;

    int count;
    struct lval** cell;
};

/* Create a new lval from a number */
struct lval* lval_num(double x);

/* Create a new lval from an error */
struct lval* lval_err(char* message);

/* Create a new lval from a symbol */
struct lval* lval_sym(char* symbol);

/* Create a new lval from a function */
struct lval* lval_fun(lbuiltin fun);

/* Create a new lval from an empty sexpr */
struct lval* lval_sexpr();

/* Create a new lval from an empty qexpr */
struct lval* lval_qexpr();

/* Free an lval */
void lval_del(struct lval* v);

/* Copy an lval */
struct lval* lval_copy(struct lval* rhs);

/* Read a num from a tree */
struct lval* lval_read_num(mpc_ast_t* t);

/* Read a lval from a tree */
struct lval* lval_read(mpc_ast_t* t);

/* Add a lval to the Sexpr */
struct lval* lval_add(struct lval* v, struct lval* new_subexpr);

/* Take a sub expression in a Sexpr and delete the rest */
struct lval* lval_take(struct lval* v, int index);

/* Pop a sub expression in a Sexpr */
struct lval* lval_pop(struct lval* v, int index);

/* Return the eval expression, itself otherwise */
struct lval* lval_eval(struct lenv* e, struct lval* v);

/* Print an lval */
void lval_print(struct lval* v);

/* Print an expr lval */
void lval_expr_print(struct lval* v, char open, char close);

/* Print an lval followed by a newline */
void lval_println(struct lval* v);

#endif  // LVAL_H_
