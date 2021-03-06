#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <histedit.h>

#include "evaluation.h"
#include "lenv.h"
#include "lval.h"
#include "mpc.h"

int
main(int argc, char* argv[]) {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* String = mpc_new("string");
    mpc_parser_t* Comment = mpc_new("comment");
    mpc_parser_t* SExpr = mpc_new("sexpr");
    mpc_parser_t* QExpr = mpc_new("qexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                               \
                number   : /[-]?([0-9]*[.])?[0-9]+([eE]?[+-]?[0-9]+)?/ ;      \
                symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&|%]+/ ;               \
                string   : /\"(\\\\.|[^\"])*\"/ ;                             \
                comment  : /;[^\\r\\n]*/;                                     \
                sexpr    : '(' <expr>* ')' ;                                  \
                qexpr    : '{' <expr>* '}' ;                                  \
                expr     : <number> | <symbol> | <string>                     \
                         | <comment> | <sexpr> | <qexpr> ;                    \
                lispy    : /^/ <expr>* /$/ ;                                  \
              ",
              Number, Symbol, String, Comment, SExpr, QExpr, Expr, Lispy);

    puts("Lispy Version 0.0.1.1.0");
    puts("Press Ctrl+C, Ctrl+D, or type \"exit\" in prompt to exit\n");

    struct lenv* e = lenv_new(Lispy);
    lenv_add_builtins(e);

    /* Load input files if given as arguments */
    if (argc >= 2) {
        for (int i = 1; i < argc; ++i) {
            struct lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));

            struct lval* x = builtin_load(e, args);

            if (x->type == LVAL_ERR) {
                lval_println(x);
            }
            lval_del(x);
        }
    }
    while (1) {
        char* input = readline("lispy> ");

        /* Case where User input ^D on prompt */
        if (NULL == input) {
            printf(
                "\n"
                "End of input detected, exiting..."
                "\n");
            break;
        }
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            struct lval* result = lval_eval(e, lval_read(r.output));
            lval_println(result);
            mpc_ast_delete(r.output);
            if (result->type == LVAL_EXIT_REQ) {
                lval_del(result);
                break;
            }
            lval_del(result);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    lenv_del(e);

    mpc_cleanup(8, Number, Symbol, String, Comment, SExpr, QExpr, Expr, Lispy);
    return EXIT_SUCCESS;
}
