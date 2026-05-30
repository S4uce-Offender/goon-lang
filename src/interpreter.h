#ifndef INTERPVALER_H
#define INTERPVALER_H

#include <math.h>

#include "errors.h"
#include "model.h"

#define APPLY_UNARY_NUMERIC_OP(val, op)           \
    do {                                          \
        if ((val).val_type == VAL_INT) {          \
            (val).as.i = op(val).as.i;            \
        } else if ((val).val_type == VAL_FLOAT) { \
            (val).as.f = op(val).as.f;            \
        }                                         \
    } while (0)

#define APPLY_BINARY_NUMERIC_OP(result, lhs, rhs, op)                         \
    do {                                                                      \
        if ((result).val_type == VAL_INT) {                                   \
            (result).as.i = (lhs).as.i op(rhs).as.i;                          \
                                                                              \
        } else if ((result).val_type == VAL_FLOAT) {                          \
            (result).as.f =                                                   \
                ((lhs).val_type == VAL_INT ? (float)(lhs).as.i : (lhs).as.f)  \
                                                                              \
                    op                                                        \
                                                                              \
                ((rhs).val_type == VAL_INT ? (float)(rhs).as.i : (rhs).as.f); \
        }                                                                     \
    } while (0)

#define APPLY_BINARY_BOOL_OP(result, left, right, op) \
    do {                                              \
        result.val_type = VAL_BOOL;                   \
        result.as.b = (left).as.b op(right).as.b;     \
    } while (0)

enum ValueType { VAL_INT, VAL_FLOAT, VAL_BOOL, VAL_OBJ };

enum ObjectType { OBJ_STR };

struct Object {
    enum ObjectType kind;
};

struct ObjString {
    struct Object base;

    char* string;
    size_t size;
};

struct Value {
    enum ValueType val_type;

    union {
        int32_t i;
        float f;
        bool b;
        struct Object* obj;
    } as;
};

struct Interpreter {
    struct SourceFile* source;

    struct Token* tok_left;
    struct Token* tok_right;
};

enum LeftOrRight { LEFT, RIGHT };

struct Value interpret(struct Interpreter* intrptr, struct Node* ast,
                       enum LeftOrRight left_or_right);

void initInterpreter(struct Interpreter* intrptr, struct SourceFile* source);

#endif