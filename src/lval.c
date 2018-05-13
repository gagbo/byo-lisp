#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lval.h"

static struct lval* lval_eval_sexpr(struct lval* v);

struct lval*
lval_num(long x) {
    struct lval* v = malloc(sizeof(struct lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

struct lval*
lval_err(char* message) {
    struct lval* v = malloc(sizeof(struct lval));
    v->type = LVAL_ERR;
    v->err = strdup(message);
    return v;
}

struct lval*
lval_sym(char* symbol) {
    struct lval* v = malloc(sizeof(struct lval));
    v->type = LVAL_SYM;
    v->sym = strdup(symbol);
    return v;
}

struct lval*
lval_sexpr() {
    struct lval* v = malloc(sizeof(struct lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void
lval_del(struct lval* v) {
    switch (v->type) {
        case LVAL_NUM:
            break;
        case LVAL_ERR:
            free(v->err);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; ++i) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }

    free(v);
}

struct lval*
lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

struct lval*
lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        return lval_read_num(t);
    }

    if (strstr(t->tag, "symbol")) {
        return lval_sym(t->contents);
    }

    struct lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) {
        x = lval_sexpr();
    }
    if (strstr(t->tag, "sexpr")) {
        x = lval_sexpr();
    }

    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->contents, ")") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->tag, "regex") == 0) {
            continue;
        }
        x = lval_add(x, lval_read(t->children[i]));
    }

    return x;
}

struct lval*
lval_add(struct lval* v, struct lval* new_subexpr) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(struct lval*) * v->count);
    v->cell[v->count - 1] = new_subexpr;
    return v;
}

struct lval*
lval_take(struct lval* v, int index) {
    struct lval* x = lval_pop(v, index);
    lval_del(v);
    return x;
}

struct lval*
lval_pop(struct lval* v, int index) {
    if (v->type != LVAL_SEXPR) {
        return lval_err("Value is not a S-expr");
    }
    if (index >= v->count) {
        return lval_err("S-expression does not have so many sub expr");
    }
    struct lval* ans = v->cell[index];

    memmove(&v->cell[index], &v->cell[index + 1],
            sizeof(struct lval*) * (v->count - index - 1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(struct lval*) * v->count);
    return ans;
}

static struct lval*
lval_eval_sexpr(struct lval* v) {
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }

    for (int i = 0; i < v->count; ++i) {
        if (v->cell[i]->type == LVAL_ERR) {
            return lval_take(v, i);
        }
    }

    if (v->count == 0) {
        return v;
    }

    if (v->count == 1) {
        return lval_take(v, 0);
    }

    struct lval* f = lval_pop(v, 0);
    if (f->type != LVAL_SYM) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression does not start with symbol !");
    }

    struct lval* result = builtin_op(v, f->sym);
    lval_del(f);
    return result;
}

struct lval*
lval_eval(struct lval* v) {
    if (v->type == LVAL_SEXPR) {
        return lval_eval_sexpr(v);
    }
    return v;
}

void
lval_print(struct lval* v) {
    switch (v->type) {
        case LVAL_NUM:
            printf("%li", v->num);
            break;

        case LVAL_ERR:
            printf("Error : %s", v->err);
            break;

        case LVAL_SYM:
            printf("%s", v->sym);
            break;

        case LVAL_SEXPR:
            lval_expr_print(v, '(', ')');
            break;
    }
}

void
lval_expr_print(struct lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; ++i) {
        lval_print(v->cell[i]);

        if (i != v->count - 1) {
            putchar(' ');
        }
    }
    putchar(close);
}

void
lval_println(struct lval* v) {
    lval_print(v);
    putchar('\n');
}
