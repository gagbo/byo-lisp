#ifndef EVALUATION_H_
#define EVALUATION_H_

#include "mpc.h"
#include "lval.h"

struct lval eval(mpc_ast_t* a);
struct lval eval_op(struct lval x, char* op, struct lval y);

#endif  // EVALUATION_H_
