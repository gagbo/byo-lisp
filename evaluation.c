#include "evaluation.h"

#include <stdio.h>
#include <string.h>

#include "mpc.h"

long eval(mpc_ast_t* a) {
    /**
     * Printing the AST node information
     * printf("Tag : %s\n", a->tag);
     * printf("Contents : %s\n", a->contents);
     * printf("Count of children : %d\n\n", a->children_num);
     */

    if (strstr(a->tag, "number")) {
        return atoi(a->contents);
    }

    /* In this case (!number), 0 is '(' */
    char* op = a->children[1]->contents;

    long x = eval(a->children[2]);

    int i = 3;
    while (strstr(a->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(a->children[i]));
        ++i;
    }

    return x;
}

long eval_op(long x, char* op, long y) {
    if (strcmp(op, "+") == 0) { return x + y; }
    if (strcmp(op, "-") == 0) { return x - y; }
    if (strcmp(op, "*") == 0) { return x * y; }
    if (strcmp(op, "/") == 0) { return x / y; }
    return 0;
}
