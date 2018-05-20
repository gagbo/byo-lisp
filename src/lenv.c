#include "lenv.h"
#include <string.h>
#include "evaluation.h"

struct lenv*
lenv_new() {
    struct lenv* e = malloc(sizeof(struct lenv));
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

struct lenv*
lenv_copy(struct lenv* rhs) {
    struct lenv* e = malloc(sizeof(struct lenv));
    e->par = rhs->par;
    e->count = rhs->count;
    e->syms = malloc(sizeof(char*) * e->count);
    e->vals = malloc(sizeof(struct lval*) * e->count);

    for (int i = 0; i < e->count; ++i) {
        e->syms[i] = strdup(rhs->syms[i]);
        e->vals[i] = lval_copy(rhs->vals[i]);
    }
    return e;
}

void
lenv_del(struct lenv* e) {
    for (int i = 0; i < e->count; ++i) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

struct lval*
lenv_get(struct lenv* e, struct lval* k) {
    for (int i = 0; i < e->count; ++i) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }

    if (e->par) {
        return lenv_get(e->par, k);
    }
    return lval_err("Unbound symbol '%s' !", k->sym);
}

bool
lenv_is_builtin(struct lenv* e, struct lval* k) {
    struct lval* target = lenv_get(e, k);
    if (target->type != LVAL_FUN) {
        return false;
    }

    return (target->builtin != NULL);
}

void
lenv_put(struct lenv* e, struct lval* k, struct lval* v) {
    for (int i = 0; i < e->count; ++i) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    e->count++;
    e->vals = realloc(e->vals, sizeof(struct lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = strdup(k->sym);
}

void
lenv_def(struct lenv* e, struct lval* k, struct lval* v) {
    while (e->par) {
        e = e->par;
    }

    lenv_put(e, k, v);
}

void
lenv_add_builtin(struct lenv* e, char* name, lbuiltin fun) {
    struct lval* key = lval_sym(name);
    struct lval* value = lval_builtin(name, fun);
    lenv_put(e, key, value);
    lval_del(key);
    lval_del(value);
}

void
lenv_add_builtins(struct lenv* e) {
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    lenv_add_builtin(e, "%", builtin_mod);
    lenv_add_builtin(e, "floor", builtin_floor);

    lenv_add_builtin(e, "!", builtin_not);
    lenv_add_builtin(e, "||", builtin_or);
    lenv_add_builtin(e, "&&", builtin_and);

    lenv_add_builtin(e, ">", builtin_gt);
    lenv_add_builtin(e, ">=", builtin_ge);
    lenv_add_builtin(e, "<", builtin_lt);
    lenv_add_builtin(e, "<=", builtin_le);

    lenv_add_builtin(e, "==", builtin_eq);
    lenv_add_builtin(e, "!=", builtin_ne);

    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "len", builtin_len);
    lenv_add_builtin(e, "cons", builtin_cons);
    lenv_add_builtin(e, "init", builtin_init);

    lenv_add_builtin(e, "cond", builtin_cond);

    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "=", builtin_put);
    lenv_add_builtin(e, "exit", builtin_exit);

    lenv_add_builtin(e, "\\", builtin_lambda);
    lenv_add_builtin(e, "fun", builtin_fun);
}
