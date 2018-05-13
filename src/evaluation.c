#include "evaluation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpc.h"

struct lval
eval(mpc_ast_t* a) {
    if (strstr(a->tag, "number")) {
        errno = 0;
        long x = strtol(a->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    /* In this case (!number), 0 is '(' */
    char* op = a->children[1]->contents;

    struct lval x = eval(a->children[2]);

    int i = 3;
    while (strstr(a->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(a->children[i]));
        ++i;
    }

    return x;
}

struct lval
eval_op(struct lval x, char* op, struct lval y) {
    if (x.type == LVAL_ERR) {
        return x;
    }
    if (y.type == LVAL_ERR) {
        return y;
    }

    if (strcmp(op, "+") == 0) {
        return lval_num(x.num + y.num);
    }
    if (strcmp(op, "-") == 0) {
        return lval_num(x.num - y.num);
    }
    if (strcmp(op, "*") == 0) {
        return lval_num(x.num * y.num);
    }
    if (strcmp(op, "/") == 0) {
        return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
    }

    return lval_err(LERR_BAD_OP);
}
