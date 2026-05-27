#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <math.h>
#include "model.h"

#define APPLY_UNARY_NUMERIC_OP(val, op)             \
    do {                                            \
        if ((val).return_val == RET_INT) {          \
            (val).as.i = op(val).as.i;              \
        } else if ((val).return_val == RET_FLOAT) { \
            (val).as.f = op(val).as.f;              \
        }                                           \
    } while (0)

#define APPLY_BINARY_NUMERIC_OP(result, lhs, rhs, op)                          \
    do {                                                                       \
        if ((result).return_val == RET_INT) {                                  \
            (result).as.i = (lhs).as.i op(rhs).as.i;                           \
                                                                               \
        } else if ((result).return_val == RET_FLOAT) {                         \
            (result).as.f =                                                    \
                ((lhs).return_val == RET_INT ? (float)(lhs).as.i : (lhs).as.f) \
                                                                               \
                    op                                                         \
                                                                               \
                ((rhs).return_val == RET_INT ? (float)(rhs).as.i               \
                                             : (rhs).as.f);                    \
        }                                                                      \
    } while (0)

enum ReturnType { RET_INT, RET_FLOAT };

struct NumericalVal {
    enum ReturnType return_val;

    union {
        int32_t i;
        float f;
    } as;
};

struct NumericalVal interpret(struct Node* ast);

#endif