#include "evaluation.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpc.h"

#define LASSERT(args, cond, err) \
    if (!(cond)) {               \
        lval_del(args);          \
        return lval_err(err);    \
    }

#define LASSERT_NON_EMPTY(args)            \
    if (args->cell[0]->count == 0) {       \
        lval_del(args);                    \
        return lval_err("Empty argument"); \
    }

#define LASSERT_NUM_ARGS(args, c)                     \
    if (args->count != c) {                           \
        lval_del(args);                               \
        return lval_err("Wrong number of arguments"); \
    }

static struct lval* builtin_op(struct lval* a, char* op);
static struct lval* builtin_head(struct lval* a);
static struct lval* builtin_tail(struct lval* a);
static struct lval* builtin_join(struct lval* a);
static struct lval* lval_join(struct lval* lhs, struct lval* rhs);
static struct lval* builtin_eval(struct lval* a);
static struct lval* builtin_list(struct lval* a);
static struct lval* builtin_len(struct lval* a);
static struct lval* builtin_cons(struct lval* a);
static struct lval* builtin_init(struct lval* a);

struct lval*
builtin(struct lval* a, char* op) {
    if (strcmp(op, "list") == 0) {
        return builtin_list(a);
    }
    if (strcmp(op, "head") == 0) {
        return builtin_head(a);
    }
    if (strcmp(op, "join") == 0) {
        return builtin_join(a);
    }
    if (strcmp(op, "tail") == 0) {
        return builtin_tail(a);
    }
    if (strcmp(op, "eval") == 0) {
        return builtin_eval(a);
    }
    if (strcmp(op, "cons") == 0) {
        return builtin_cons(a);
    }
    if (strcmp(op, "len") == 0) {
        return builtin_len(a);
    }
    if (strcmp(op, "init") == 0) {
        return builtin_init(a);
    }
    if (strcmp(op, "floor") == 0 || strstr("%*/+-", op)) {
        return builtin_op(a, op);
    }

    return lval_err("Bad operation");
}

static struct lval*
builtin_head(struct lval* a) {
    LASSERT_NUM_ARGS(a, 1);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function 'head' passed incorrect type !");
    LASSERT_NON_EMPTY(a);

    struct lval* v = lval_take(a, 0);
    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }
    return v;
}

static struct lval*
builtin_tail(struct lval* a) {
    LASSERT_NUM_ARGS(a, 1);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function 'tail' passed incorrect type !");
    LASSERT_NON_EMPTY(a);

    struct lval* v = lval_take(a, 0); /* Here v is the actual {QEXPR} arg */
    lval_del(lval_pop(v, 0));
    return v;
}

static struct lval*
builtin_list(struct lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}

static struct lval*
builtin_eval(struct lval* a) {
    LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments !");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function 'eval' passed incorrect types !");

    struct lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

static struct lval*
builtin_join(struct lval* a) {
    for (int i = 0; i < a->count; ++i) {
        if (a->cell[i]->type != LVAL_QEXPR) {
            lval_del(a);
            return lval_err("Function 'join' passed incorrect types !");
        }
    }

    struct lval* x = lval_pop(a, 0);

    while (a->count > 0) {
        x = lval_join(x, lval_pop(a, 0));
    }

    lval_del(a);
    return x;
}

static struct lval*
lval_join(struct lval* lhs, struct lval* rhs) {
    while (rhs->count > 0) {
        lhs = lval_add(lhs, lval_pop(rhs, 0));
    }

    lval_del(rhs);
    return lhs;
}

static struct lval*
builtin_cons(struct lval* a) {
    LASSERT_NUM_ARGS(a, 2);
    struct lval* v = lval_eval(lval_pop(a, 0));
    LASSERT(v, v->type == LVAL_NUM, "First argument is not evaluable !");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Second argument is not a Q-Expr !");

    struct lval* ans = lval_qexpr();
    lval_add(ans, v);
    lval_join(ans, lval_take(a, 0));
    return ans;
}

static struct lval*
builtin_len(struct lval* a) {
    LASSERT_NUM_ARGS(a, 1);
    struct lval* ans = lval_qexpr();
    lval_add(ans, lval_num(a->cell[0]->count));

    return ans;
}

static struct lval*
builtin_init(struct lval* a) {
    LASSERT_NUM_ARGS(a, 1);
    LASSERT_NON_EMPTY(a);

    lval_del(lval_pop(a->cell[0], a->cell[0]->count - 1));
    return a->cell[0];
}

static struct lval*
builtin_op(struct lval* a, char* op) {
    if (a->type == LVAL_ERR) {
        return a;
    }

    struct lval* x = lval_pop(a, 0);
    if (strcmp(op, "-") == 0 && a->count == 0) {
        x->num = -x->num;
    }

    if (strcmp(op, "floor") == 0) {
        if (a->count == 0) {
            x->num = floor(x->num);
        } else {
            lval_del(a);
            return lval_err("floor expects one argument !");
        }
    }

    while (a->count > 0) {
        struct lval* y = lval_eval(lval_pop(a, 0));
        if (strcmp(op, "+") == 0) {
            x->num += y->num;
        }
        if (strcmp(op, "-") == 0) {
            x->num -= y->num;
        }
        if (strcmp(op, "*") == 0) {
            x->num *= y->num;
        }
        if (strcmp(op, "/") == 0) {
            if (y->num == 0) {
                lval_del(x);
                lval_del(y);
                x = lval_err("division by zero");
                break;
            }
            x->num /= y->num;
        }
        if (strcmp(op, "%") == 0) {
            if (y->num == 0) {
                lval_del(x);
                lval_del(y);
                x = lval_err("division by zero");
                break;
            }
            x->num = (int)round(x->num) % (int)round(y->num);
        }
        lval_del(y);
    }

    lval_del(a);
    return x;
}
