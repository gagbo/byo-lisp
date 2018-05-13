#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <histedit.h>

#include "evaluation.h"
#include "lval.h"
#include "mpc.h"

int
main(int argc, char** argv) {
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("symbol");
    mpc_parser_t* SExpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                    \
                number   : /-?[0-9]+/ ;                            \
                symbol   : '+' | '-' | '*' | '/' | '%' ;           \
                sexpr    : '(' <expr>* ')' ;                       \
                expr     : <number> | <symbol> | <sexpr> ;         \
                lispy    : /^/ <expr>* /$/ ;                       \
              ",
              Number, Operator, SExpr, Expr, Lispy);

    puts("Lispy Version 0.0.0.0.4");
    puts("Press Ctrl+C to exit\n");

    while (1) {
        char* input = readline("lispy> ");

        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            struct lval* result = lval_eval(lval_read(r.output));
            lval_println(result);
            mpc_ast_delete(r.output);
            lval_del(result);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(5, Number, Operator, SExpr, Expr, Lispy);
    return EXIT_SUCCESS;
}
