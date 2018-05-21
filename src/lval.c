#include "lval.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lenv.h"

#define MAX_ERROR_LEN 512

static struct lval* lval_eval_sexpr(struct lenv* e, struct lval* v);
static void lval_print_str(struct lval* v);
static struct lval* lval_read_str(mpc_ast_t* t);
/* Put all pointers to NULL */
static void lval_default(struct lval* v);

char*
ltype_name(int t) {
    switch (t) {
        case LVAL_FUN:
            return "Function";
        case LVAL_NUM:
            return "Number";
        case LVAL_ERR:
            return "Error";
        case LVAL_SYM:
            return "Symbol";
        case LVAL_BOOL:
            return "Boolean";
        case LVAL_STR:
            return "String";
        case LVAL_SEXPR:
            return "S-Expression";
        case LVAL_QEXPR:
            return "Q-Expression";
        case LVAL_EXIT_REQ:
            return "Exit request";
        default:
            return "Unknown";
    }
}

static void
lval_default(struct lval* v) {
    v->err = NULL;
    v->sym = NULL;
    v->str = NULL;

    v->env = NULL;
    v->formals = NULL;
    v->body = NULL;

    v->cell = NULL;
}

struct lval*
lval_num(double x) {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

struct lval*
lval_err(char* fmt, ...) {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);

    v->err = malloc(MAX_ERROR_LEN);
    vsnprintf(v->err, MAX_ERROR_LEN, fmt, va);
    v->err[MAX_ERROR_LEN - 1] = '\0';

    v->err = realloc(v->err, strlen(v->err) + 1);

    va_end(va);
    return v;
}

struct lval*
lval_sym(char* symbol) {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_SYM;
    v->sym = strdup(symbol);
    return v;
}

struct lval*
lval_bool(bool value) {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_BOOL;
    v->t = value;
    return v;
}

struct lval*
lval_str(char* s) {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_STR;
    v->str = strdup(s);
    return v;
}

struct lval*
lval_builtin(char* name, lbuiltin builtin) {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_FUN;
    v->builtin = builtin;
    v->sym = strdup(name);
    return v;
}

struct lval*
lval_lambda(struct lval* formals, struct lval* body) {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_FUN;

    v->builtin = NULL;

    v->env = lenv_new();
    v->formals = formals;
    v->body = body;

    return v;
}

struct lval*
lval_call(struct lenv* e, struct lval* f, struct lval* a) {
    if (f->type != LVAL_FUN) {
        struct lval* err = lval_err("Not evalutating a function");
        lval_del(a);
        return err;
    }

    /* If builtin, return the result directly */
    if (f->builtin) {
        return f->builtin(e, a);
    }

    int given = a->count;
    int total = f->formals->count;

    while (a->count > 0) {
        if (f->formals->count == 0) {
            lval_del(a);
            return lval_err(
                "Function passed too many arguments. Got %i, Expected %i",
                given, total);
        }

        /* Bind the argument value to the function formal symbol */
        struct lval* sym = lval_pop(f->formals, 0);
        /* Special case to deal with '&' */
        if (strcmp(sym->sym, "&") == 0) {
            /* Ensure '&' is followed by exactly one other symbol */
            if (f->formals->count != 1) {
                lval_del(a);
                return lval_err(
                    "Function format invalid. "
                    "Symbol '&' not followed by single symbol");
            }

            struct lval* nsym = lval_pop(f->formals, 0);
            /* Bind the list of var args as a Q-Expr to the symbol after '&' */
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym);
            lval_del(nsym);
            break;
        }
        struct lval* bind_val = lval_pop(a, 0);

        lenv_put(f->env, sym, bind_val);

        /* Delete the popped lvals */
        lval_del(sym);
        lval_del(bind_val);
    }

    lval_del(a);

    /* Case where '&' is left : we have to give an empty list as optional args
     */
    if (f->formals->count > 0 && strcmp(f->formals->cell[0]->sym, "&") == 0) {
        /* Check that the function is well formed : only 1 symbol after & */
        if (f->formals->count != 2) {
            return lval_err(
                "Function format invalid. "
                "Symbol '&' not followed by single symbol.");
        }

        /* Delete the '&' lval */
        lval_del(lval_pop(f->formals, 0));

        struct lval* sym = lval_pop(f->formals, 0);
        struct lval* val = lval_qexpr();

        lenv_put(f->env, sym, val);
        lval_del(sym);
        lval_del(val);
    }

    /* If all function arguments have been bound then evaluate */
    if (f->formals->count == 0) {
        f->env->par = e;
        return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    } else {
        /* Return a copy of the function with partially bound arguments */
        return lval_copy(f);
    }
}

struct lval*
lval_sexpr() {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

struct lval*
lval_qexpr() {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

struct lval*
lval_exit_req(char* fmt, ...) {
    struct lval* v = malloc(sizeof(struct lval));
    assert(v);
    lval_default(v);
    v->type = LVAL_EXIT_REQ;

    va_list va;
    va_start(va, fmt);

    v->err = malloc(MAX_ERROR_LEN);
    vsnprintf(v->err, MAX_ERROR_LEN, fmt, va);
    v->err[MAX_ERROR_LEN - 1] = '\0';

    v->err = realloc(v->err, strlen(v->err) + 1);

    va_end(va);
    return v;
}

struct lval*
lval_copy(struct lval* rhs) {
    struct lval* x = malloc(sizeof(struct lval));
    assert(x);
    lval_default(x);

    x->type = rhs->type;

    switch (rhs->type) {
        case LVAL_FUN:
            if (rhs->builtin) {
                x->builtin = rhs->builtin;
                x->sym = strdup(rhs->sym);
            } else {
                x->builtin = NULL;
                x->env = lenv_copy(rhs->env);
                x->formals = lval_copy(rhs->formals);
                x->body = lval_copy(rhs->body);
            }
            break;
        case LVAL_NUM:
            x->num = rhs->num;
            break;
        case LVAL_ERR:
        case LVAL_EXIT_REQ:
            x->err = strdup(rhs->err);
            break;
        case LVAL_BOOL:
            x->t = rhs->t;
            break;
        case LVAL_STR:
            x->str = strdup(rhs->str);
            break;
        case LVAL_SYM:
            x->sym = strdup(rhs->sym);
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            x->count = rhs->count;
            x->cell = malloc(sizeof(struct lval*) * x->count);
            for (int i = 0; i < x->count; ++i) {
                x->cell[i] = lval_copy(rhs->cell[i]);
            }
            break;
    }
    return x;
}

void
lval_del(struct lval* v) {
    switch (v->type) {
        case LVAL_NUM:
        case LVAL_BOOL:
            break;
        case LVAL_STR:
            free(v->str);
            break;
        case LVAL_ERR:
        case LVAL_EXIT_REQ:
            free(v->err);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; ++i) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
        case LVAL_FUN:
            if (v->builtin) {
                free(v->sym);
            } else {
                lenv_del(v->env);
                lval_del(v->formals);
                lval_del(v->body);
            }
            break;
    }

    free(v);
}

struct lval*
lval_read_num(mpc_ast_t* t) {
    errno = 0;
    double x = strtod(t->contents, NULL);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

static struct lval*
lval_read_str(mpc_ast_t* t) {
    /* Cut off the final quote */
    t->contents[strlen(t->contents) - 1] = '\0';

    /* Cut out the first quote and copy in variable */
    char* unescaped = strdup(t->contents + 1);

    /* Use mpc function to interpret escape sequences */
    unescaped = mpcf_unescape(unescaped);
    struct lval* str = lval_str(unescaped);

    free(unescaped);
    return str;
}

struct lval*
lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) {
        return lval_read_num(t);
    }

    if (strstr(t->tag, "symbol")) {
        return lval_sym(t->contents);
    }

    if (strstr(t->tag, "string")) {
        return lval_read_str(t);
    }

    struct lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) {
        x = lval_sexpr();
    }
    if (strstr(t->tag, "sexpr")) {
        x = lval_sexpr();
    }
    if (strstr(t->tag, "qexpr")) {
        x = lval_qexpr();
    }

    for (int i = 0; i < t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->contents, ")") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->contents, "{") == 0) {
            continue;
        }
        if (strcmp(t->children[i]->contents, "}") == 0) {
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
    if (NULL == new_subexpr) {
        return v;
    }
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
    if (v->type != LVAL_SEXPR && v->type != LVAL_QEXPR) {
        return lval_err("Value is neither a S-expr nor a Q-expr");
    }
    if (index >= v->count) {
        return lval_err("{Q,S}-expression does not have so many sub expr");
    }
    struct lval* ans = v->cell[index];

    memmove(&v->cell[index], &v->cell[index + 1],
            sizeof(struct lval*) * (v->count - index - 1));

    v->count--;

    v->cell = realloc(v->cell, sizeof(struct lval*) * v->count);
    return ans;
}

static struct lval*
lval_eval_sexpr(struct lenv* e, struct lval* v) {
    /* Evaluate each cell */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(e, v->cell[i]);
    }

    /* Check that no cell had an error */
    for (int i = 0; i < v->count; ++i) {
        if (v->cell[i]->type == LVAL_ERR) {
            return lval_take(v, i);
        }
    }

    /* Return itself if v contains no sub-expr */
    if (v->count == 0) {
        return v;
    }

    /* Return the inner sigleton for small S-Expr */
    if (v->count == 1) {
        return lval_take(v, 0);
    }

    /* Here we know we have a 'long' S-Expression, so it must start with a
     * function */
    struct lval* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression does not start with a function !");
    }

    struct lval* result = lval_call(e, f, v);
    lval_del(f);
    return result;
}

struct lval*
lval_eval(struct lenv* e, struct lval* v) {
    if (v->type == LVAL_SYM) {
        struct lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }

    if (v->type == LVAL_SEXPR) {
        return lval_eval_sexpr(e, v);
    }
    return v;
}

static void
lval_print_str(struct lval* v) {
    char* escaped = strdup(v->str);
    escaped = mpcf_escape(escaped);
    printf("\"%s\"", escaped);
    free(escaped);
}

void
lval_print(struct lval* v) {
    switch (v->type) {
        case LVAL_NUM:
            printf("%g", v->num);
            break;

        case LVAL_STR:
            lval_print_str(v);
            break;

        case LVAL_BOOL:
            printf("%s", v->t ? "t" : "f");
            break;

        case LVAL_ERR:
            printf("Error : %s", v->err);
            break;

        case LVAL_EXIT_REQ:
            printf("Exit request : %s", v->err);
            break;

        case LVAL_SYM:
            printf("%s", v->sym);
            break;

        case LVAL_SEXPR:
            lval_expr_print(v, '(', ')');
            break;

        case LVAL_QEXPR:
            lval_expr_print(v, '{', '}');
            break;

        case LVAL_FUN:
            if (v->builtin) {
                printf("<builtin> : %s", v->sym);
            } else {
                printf("(\\ ");
                lval_print(v->formals);
                putchar(' ');
                lval_print(v->body);
                putchar(')');
            }
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

bool
lval_eq(struct lval* x, struct lval* y) {
    if (x->type != y->type) {
        return false;
    }

    switch (x->type) {
        case LVAL_ERR:
            return (strcmp(x->err, y->err) == 0);
        case LVAL_SYM:
            return (strcmp(x->sym, y->sym) == 0);
        case LVAL_NUM:
            return (x->num == y->num);
        case LVAL_BOOL:
            return (x->t == y->t);
        case LVAL_STR:
            return (strcmp(x->str, y->str) == 0);
        case LVAL_FUN:
            if (x->builtin || y->builtin) {
                return (x->builtin == y->builtin);
            } else {
                return lval_eq(x->formals, y->formals) &&
                       lval_eq(x->body, y->body);
            }
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if (x->count != y->count) {
                return false;
            }
            for (int i = 0; i < x->count; ++i) {
                if (lval_eq(x->cell[i], y->cell[i]) == 0) {
                    return false;
                }
            }
            return true;
        case LVAL_EXIT_REQ:
            /* There's only 1 type of Exit requests */
            return true;
        default:
            return false;
    }
}
