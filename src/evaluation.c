#include "evaluation.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lenv.h"
#include "mpc.h"

#define LASSERT(args, cond, fmt, ...)                    \
    if (!(cond)) {                                       \
        struct lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args);                                  \
        return err;                                      \
    }

#define LASSERT_NON_EMPTY(name, args)                 \
    if (args->cell[0]->count == 0) {                  \
        lval_del(args);                               \
        return lval_err("%s : Empty argument", name); \
    }

#define LASSERT_NUM_ARGS(name, args, c)                                   \
    if (args->count != c) {                                               \
        int saved_count = args->count;                                    \
        lval_del(args);                                                   \
        return lval_err(                                                  \
            "%s : Wrong number of arguments. Got %i, Expected %i.", name, \
            saved_count, c);                                              \
    }

#define LASSERT_TYPE(name, args, i, wanted_type)                              \
    if (args->cell[i]->type != wanted_type) {                                 \
        int saved_type = args->cell[i]->type;                                 \
        lval_del(args);                                                       \
        return lval_err(                                                      \
            "%s : Wrong type for argument %i. Got %s, Expected %s.", name, i, \
            ltype_name(saved_type), ltype_name(wanted_type));                 \
    }

static struct lval* builtin_op(struct lenv* e, struct lval* a, char* op);
static struct lval* lval_join(struct lval* lhs, struct lval* rhs);
static struct lval* builtin_var(struct lenv* e, struct lval* a, char* func);
static struct lval* builtin_ord(struct lenv* e, struct lval* a, char* op);
static struct lval* builtin_cmp(struct lenv* e, struct lval* a, char* op);

struct lval*
builtin_add(struct lenv* e, struct lval* x) {
    return builtin_op(e, x, "+");
}

struct lval*
builtin_sub(struct lenv* e, struct lval* x) {
    return builtin_op(e, x, "-");
}

struct lval*
builtin_mul(struct lenv* e, struct lval* x) {
    return builtin_op(e, x, "*");
}

struct lval*
builtin_div(struct lenv* e, struct lval* x) {
    return builtin_op(e, x, "/");
}

struct lval*
builtin_mod(struct lenv* e, struct lval* x) {
    return builtin_op(e, x, "%");
}

struct lval*
builtin_floor(struct lenv* e, struct lval* x) {
    return builtin_op(e, x, "floor");
}

struct lval*
builtin_head(struct lenv* e, struct lval* a) {
    LASSERT_NUM_ARGS("head", a, 1);
    LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
    LASSERT_NON_EMPTY("head", a);
    (void)e;

    struct lval* v = lval_take(a, 0);
    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }
    return v;
}

struct lval*
builtin_tail(struct lenv* e, struct lval* a) {
    LASSERT_NUM_ARGS("tail", a, 1);
    LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
    LASSERT_NON_EMPTY("tail", a);
    (void)e;

    struct lval* v = lval_take(a, 0); /* Here v is the actual {QEXPR} arg */
    lval_del(lval_pop(v, 0));
    return v;
}

struct lval*
builtin_list(struct lenv* e, struct lval* a) {
    (void)e;
    a->type = LVAL_QEXPR;
    return a;
}

struct lval*
builtin_eval(struct lenv* e, struct lval* a) {
    LASSERT_NUM_ARGS("eval", a, 1);
    LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);

    struct lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

struct lval*
builtin_join(struct lenv* e, struct lval* a) {
    for (int i = 0; i < a->count; ++i) {
        LASSERT_TYPE("join", a, i, LVAL_QEXPR);
    }
    (void)e;

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

struct lval*
builtin_cons(struct lenv* e, struct lval* a) {
    LASSERT_NUM_ARGS("cons", a, 2);
    struct lval* v = lval_eval(e, lval_pop(a, 0));
    LASSERT(v, v->type == LVAL_NUM, "First argument is not evaluable !");
    LASSERT_TYPE("cons", a, 0, LVAL_QEXPR);

    struct lval* ans = lval_qexpr();
    lval_add(ans, v);
    lval_join(ans, lval_take(a, 0));
    return ans;
}

struct lval*
builtin_len(struct lenv* e, struct lval* a) {
    LASSERT_NUM_ARGS("len", a, 1);
    LASSERT_TYPE("len", a, 0, LVAL_QEXPR);
    (void)e;
    struct lval* ans = lval_qexpr();
    lval_add(ans, lval_num(a->cell[0]->count));

    return ans;
}

struct lval*
builtin_init(struct lenv* e, struct lval* a) {
    LASSERT_NUM_ARGS("init", a, 1);
    LASSERT_NON_EMPTY("init", a);
    (void)e;

    lval_del(lval_pop(a->cell[0], a->cell[0]->count - 1));
    return a->cell[0];
}

static struct lval*
builtin_op(struct lenv* e, struct lval* a, char* op) {
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
        LASSERT_TYPE(op, a, 0, LVAL_NUM);
        struct lval* y = lval_eval(e, lval_pop(a, 0));
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

struct lval*
builtin_def(struct lenv* e, struct lval* a) {
    return builtin_var(e, a, "def");
}

struct lval*
builtin_put(struct lenv* e, struct lval* a) {
    return builtin_var(e, a, "=");
}

static struct lval*
builtin_var(struct lenv* e, struct lval* x, char* func) {
    LASSERT_TYPE(func, x, 0, LVAL_QEXPR);

    struct lval* syms = x->cell[0];

    for (int i = 0; i < syms->count; ++i) {
        LASSERT(x, syms->cell[i]->type == LVAL_SYM,
                "Function '%s' cannot define non-symbol. Got %s, Expected %s.",
                func, ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
        if (lenv_is_builtin(e, syms->cell[i])) {
            struct lval* err =
                lval_err("def/fun/= : %s is already a builtin function !",
                         syms->cell[i]->sym);
            lval_del(x);
            return err;
        }
    }

    LASSERT(x, syms->count == x->count - 1,
            "Function '%s' passed incorrect number of values to symbols. Got "
            "%i, Expected%i",
            func, syms->count, x->count - 1);

    for (int i = 0; i < syms->count; ++i) {
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], x->cell[i + 1]);
        }
        if (strcmp(func, "=") == 0) {
            lenv_put(e, syms->cell[i], x->cell[i + 1]);
        }
    }

    lval_del(x);
    return lval_sexpr();
}

struct lval*
builtin_exit(struct lenv* e, struct lval* x) {
    (void)e;
    lval_del(x);
    return lval_exit_req("exit command");
}

struct lval*
builtin_lambda(struct lenv* e, struct lval* a) {
    (void)e;
    LASSERT_NUM_ARGS("\\", a, 2);
    LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

    for (int i = 0; i < a->cell[0]->count; ++i) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
                "Cannot define non-symbol. Got %s, Expected %s.",
                ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    struct lval* formals = lval_pop(a, 0);
    struct lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

struct lval*
builtin_fun(struct lenv* e, struct lval* a) {
    (void)e;
    LASSERT_NUM_ARGS("fun", a, 2);
    LASSERT_TYPE("fun", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("fun", a, 1, LVAL_QEXPR);

    for (int i = 0; i < a->cell[0]->count; ++i) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
                "Cannot define non-symbol. Got %s, Expected %s.",
                ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    struct lval* formals = lval_pop(a, 0);
    /* If the first argument of fun was {}, sym will by an error, therefore
     * builtin_def will catch it, and prevent the binding from happening */
    struct lval* sym = lval_pop(formals, 0);
    struct lval* body = lval_pop(a, 0);

    /* Put the function name back into a Q_Expr */
    struct lval* q_expr_fun_name = lval_add(lval_qexpr(), sym);

    /* Construct the struct lval we usually pass to builtin_def */
    struct lval* def_args = lval_add(lval_qexpr(), q_expr_fun_name);
    lval_add(def_args, lval_lambda(formals, body));

    lval_del(a);

    return builtin_def(e, def_args);
}

struct lval*
builtin_gt(struct lenv* e, struct lval* a) {
    return builtin_ord(e, a, ">");
}

struct lval*
builtin_ge(struct lenv* e, struct lval* a) {
    return builtin_ord(e, a, ">=");
}

struct lval*
builtin_lt(struct lenv* e, struct lval* a) {
    return builtin_ord(e, a, "<");
}

struct lval*
builtin_le(struct lenv* e, struct lval* a) {
    return builtin_ord(e, a, "<=");
}

static struct lval*
builtin_ord(struct lenv* e, struct lval* a, char* op) {
    LASSERT_NUM_ARGS(op, a, 2);
    LASSERT_TYPE(op, a, 0, LVAL_NUM);
    LASSERT_TYPE(op, a, 1, LVAL_NUM);
    double left_val = a->cell[0]->num;
    double right_val = a->cell[1]->num;
    lval_del(a);

    if (strcmp(op, ">") == 0) {
        return (left_val > right_val) ? lval_num(1) : lval_num(0);
    }
    if (strcmp(op, ">=") == 0) {
        return (left_val >= right_val) ? lval_num(1) : lval_num(0);
    }
    if (strcmp(op, "<") == 0) {
        return (left_val < right_val) ? lval_num(1) : lval_num(0);
    }
    if (strcmp(op, "<=") == 0) {
        return (left_val <= right_val) ? lval_num(1) : lval_num(0);
    }

    return lval_err("%s : comparison operator not found", op);
}

struct lval*
builtin_eq(struct lenv* e, struct lval* a) {
    return builtin_cmp(e, a, "==");
}

struct lval*
builtin_ne(struct lenv* e, struct lval* a) {
    return builtin_cmp(e, a, "!=");
}

static struct lval*
builtin_cmp(struct lenv* e, struct lval* a, char* op) {
    (void)e;
    LASSERT_NUM_ARGS(op, a, 2);

    int r;
    if (strcmp(op, "==") == 0) {
        r = lval_eq(a->cell[0], a->cell[1]);
    }
    if (strcmp(op, "!=") == 0) {
        r = (1 - lval_eq(a->cell[0], a->cell[1]));
    }
    lval_del(a);
    return lval_num(r);
}
