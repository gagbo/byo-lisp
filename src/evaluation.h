#ifndef EVALUATION_H_
#define EVALUATION_H_

#include "lval.h"
#include "mpc.h"

struct lval* builtin(struct lval* x, char* op);

#endif  // EVALUATION_H_
