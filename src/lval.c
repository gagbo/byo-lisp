#include "lval.h"
#include <stdio.h>

struct lval lval_num(long x) {
    struct lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

struct lval lval_err(int x) {
    struct lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(struct lval v) {
    switch (v.type) {
        case LVAL_NUM:
            printf("%li", v.num);
            break;

        case LVAL_ERR:
            switch (v.err) {
                case LERR_DIV_ZERO:
                    printf("Error : Division by Zero !");
                    break;
                case LERR_BAD_OP:
                    printf("Error : Invalid Operator !");
                    break;
                case LERR_BAD_NUM:
                    printf("Error : Invalid Number !");
                    break;
                default:
                    printf("Error : Unkwnown Error Type !");
            }
            break;
    }
}

void lval_println(struct lval v) {
    lval_print(v);
    putchar('\n');
}
