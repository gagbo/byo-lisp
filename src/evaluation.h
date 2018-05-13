#ifndef EVALUATION_H_
#define EVALUATION_H_

#include "lval.h"
#include "mpc.h"

struct lval* eval(mpc_ast_t* a);
struct lval* eval_op(struct lval* x, char* op, struct lval* y);

struct lval* builtin_op(struct lval* x, char* op);

#endif  // EVALUATION_H_
