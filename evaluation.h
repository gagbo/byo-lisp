#ifndef EVALUATION_H_
#define EVALUATION_H_

#include "mpc.h"

long eval(mpc_ast_t* a);
long eval_op(long x, char* op, long y);

#endif  // EVALUATION_H_
