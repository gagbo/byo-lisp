#ifndef LVAL_H_
#define LVAL_H_

enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

struct lval {
    int type;
    long num;
    int err;
};

/* Create a new lval from a number */
struct lval lval_num(long x);

/* Create a new lval from an error */
struct lval lval_err(int x);

/* Print an lval */
void lval_print(struct lval v);

/* Print an lval followed by a newline */
void lval_println(struct lval v);

#endif  // LVAL_H_
