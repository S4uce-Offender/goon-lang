#ifndef INTERPVALER_H
#define INTERPVALER_H

#include <math.h>
#include <string.h>

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

#define APPLY_BINARY_BOOL_OP(result, left, right, op)                        \
    do {                                                                     \
        float lhs =                                                          \
            ((left).val_type == VAL_INT) ? (float)(left).as.i : (left).as.f; \
                                                                             \
        float rhs = ((right).val_type == VAL_INT) ? (float)(right).as.i      \
                                                  : (right).as.f;            \
                                                                             \
        (result).val_type = VAL_BOOL;                                        \
        (result).as.b = (lhs op rhs);                                        \
    } while (0)

enum ValueType { VAL_NONE, VAL_INT, VAL_FLOAT, VAL_BOOL, VAL_OBJ };

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

static inline bool isValueType1(struct Value* val, enum ValueType val_type);

static inline bool isValueType2(struct Value* val1, struct Value* val2,
                                enum ValueType val_type);

#define IS_VAL_TYPE_GET_MACRO(_1, _2, _3, NAME, ...) NAME

#define isValueType(...) \
    IS_VAL_TYPE_GET_MACRO(__VA_ARGS__, isValueType2, isValueType1)(__VA_ARGS__)

static inline bool isNumericVal1(struct Value* val);

static inline bool isNumericVal2(struct Value* val1, struct Value* val2);

#define IS_NUMERIC_TYPE_GET_MACRO(_1, _2, NAME, ...) NAME

#define isNumericVal(...)                                 \
    IS_NUMERIC_TYPE_GET_MACRO(__VA_ARGS__, isNumericVal2, \
                              isNumericVal1)(__VA_ARGS__)

enum LeftOrRight { LEFT, RIGHT };

struct Value interpret(struct Interpreter* intrptr, struct Node* ast,
                       enum LeftOrRight left_or_right);

void initInterpreter(struct Interpreter* intrptr, struct SourceFile* source);

void printValue(struct Value* val);

#endif