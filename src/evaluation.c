#include "evaluation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpc.h"

struct lval*
eval(mpc_ast_t* a) {
    if (strstr(a->tag, "number")) {
        errno = 0;
        long x = strtol(a->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
    }

    /* In this case (!number), 0 is '(' */
    char* op = a->children[1]->contents;

    struct lval* x = eval(a->children[2]);

    int i = 3;
    while (strstr(a->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(a->children[i]));
        ++i;
    }

    return x;
}

struct lval*
eval_op(struct lval* x, char* op, struct lval* y) {
    if (x->type == LVAL_ERR) {
        return x;
    }
    if (y->type == LVAL_ERR) {
        return y;
    }

    if (strcmp(op, "+") == 0) {
        return lval_num(x->num + y->num);
    }
    if (strcmp(op, "-") == 0) {
        return lval_num(x->num - y->num);
    }
    if (strcmp(op, "*") == 0) {
        return lval_num(x->num * y->num);
    }
    if (strcmp(op, "/") == 0) {
        return y->num == 0 ? lval_err("division by zero")
                           : lval_num(x->num / y->num);
    }

    return lval_err("bad operator detected");
}

struct lval*
builtin_op(struct lval* a, char* op) {
    if (a->type == LVAL_ERR) {
        return a;
    }

    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
            lval_del(a);
            return lval_err("cannot operate on non-number!");
        }
    }

    struct lval* x = lval_pop(a, 0);
    if (strcmp(op, "-") == 0 && a->count == 0) {
        x->num = -x->num;
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
            x->num %= y->num;
        }
        lval_del(y);
    }

    lval_del(a);
    return x;
}
