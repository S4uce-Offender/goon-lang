#include "interpreter.h"

static inline bool isValueType(struct Value* val, enum ValueType val_type) {
    return val->val_type == val_type;
}

static inline bool isObjectType(struct Value* val, enum ObjectType obj_type) {
    if (val->val_type != VAL_OBJ) return false;
    return val->as.obj->kind == obj_type;
}
static inline bool isNumericVal(struct Value* val) {
    return val->val_type == VAL_INT || val->val_type == VAL_FLOAT;
}

static inline bool isBoolable(struct Value* val) {
    return val->val_type == VAL_INT || val->val_type == VAL_FLOAT ||
           val->val_type == VAL_BOOL;
}

static void handleUnsupportedBinOp(struct Interpreter* intrptr,
                                   struct BinaryNode* bin_node) {
    char err_msg[256];

    snprintf(err_msg, sizeof(err_msg),
             "Unsupported operation '%.*s' between %s token and %s token",
             bin_node->op_len, bin_node->op,
             tokenTypeToString(intrptr->tok_left->token_type),
             tokenTypeToString(intrptr->tok_right->token_type));

    size_t err_msg_len = strlen(err_msg);
    err_msg[err_msg_len] = '\0';

    interpreterThrowError(INTPTR_ERR_UNSUPPORTED_BIN_OP, err_msg,
                          intrptr->source, intrptr->tok_left,
                          intrptr->tok_right);
}

static void handleUnsupportedUnOp(struct Interpreter* intrptr,
                                  struct UnaryNode* un_node) {
    char err_msg[256];

    snprintf(err_msg, sizeof(err_msg),
             "Unsupported operation '%c' with %s token", un_node->op,
             tokenTypeToString(intrptr->tok_left->token_type));

    size_t err_msg_len = strlen(err_msg);
    err_msg[err_msg_len] = '\0';

    interpreterThrowError(INTPTR_ERR_UNSUPPORTED_UN_OP, err_msg,
                          intrptr->source, intrptr->tok_left, NULL);
}

struct Value interpretBinaryNode(struct Interpreter* intrptr,
                                 struct BinaryNode* bin_node) {
    struct Value left_val = interpret(intrptr, bin_node->left, LEFT);
    struct Value right_val = interpret(intrptr, bin_node->right, RIGHT);
    struct Value result = {0};

    result.val_type = (isValueType(&left_val, VAL_FLOAT) ||
                       isValueType(&right_val, VAL_FLOAT))
                          ? VAL_FLOAT
                          : VAL_INT;

    if (bin_node->op_len == 1) {
        switch (bin_node->op[0]) {
            case '+':
                if (isNumericVal(&left_val) && isNumericVal(&right_val)) {
                    APPLY_BINARY_NUMERIC_OP(result, left_val, right_val, +);
                    return result;
                }

                else if (isObjectType(&left_val, OBJ_STR) &&
                         isObjectType(&right_val, OBJ_STR)) {
                    result.val_type = VAL_OBJ;
                    size_t size =
                        intrptr->tok_left->len + intrptr->tok_right->len;

                    char* str = malloc(size);
                    strncpy(str, intrptr->tok_left->lexeme,
                            intrptr->tok_left->len);
                    strncpy(str + intrptr->tok_left->len,
                            intrptr->tok_right->lexeme,
                            intrptr->tok_right->len);

                    struct ObjString* str_obj =
                        malloc(sizeof(struct ObjString));

                    str_obj->base.kind = OBJ_STR;
                    str_obj->string = str;
                    str_obj->size = size;

                    result.as.obj = &str_obj->base;
                } else {
                    handleUnsupportedBinOp(intrptr, bin_node);
                }
            case '<':
                if (isNumericVal(&left_val) && isNumericVal(&right_val))
                    APPLY_BINARY_BOOL_OP(result, left_val, right_val, <);
                else if (isObjectType(&left_val, OBJ_STR) &&
                         isObjectType(&right_val, OBJ_STR)) {
                    result.val_type = VAL_BOOL;

                    result.as.b = (strcmp(intrptr->tok_left->lexeme,
                                          intrptr->tok_right->lexeme) < 0)
                                      ? true
                                      : false;
                } else {
                    handleUnsupportedBinOp(intrptr, bin_node);
                }
                break;
            case '>':
                if (isNumericVal(&left_val) && isNumericVal(&right_val))
                    APPLY_BINARY_BOOL_OP(result, left_val, right_val, >);
                else if (isObjectType(&left_val, OBJ_STR) &&
                         isObjectType(&right_val, OBJ_STR)) {
                    result.val_type = VAL_BOOL;

                    result.as.b = (strcmp(intrptr->tok_left->lexeme,
                                          intrptr->tok_right->lexeme) > 0)
                                      ? true
                                      : false;
                } else {
                    handleUnsupportedBinOp(intrptr, bin_node);
                }
                break;
        }

        if (isValueType(&left_val, VAL_INT) &&
            isValueType(&right_val, VAL_INT)) {
            switch (bin_node->op[0]) {
                case '-':
                    APPLY_BINARY_NUMERIC_OP(result, left_val, right_val, -);
                    break;
                case '*':
                    APPLY_BINARY_NUMERIC_OP(result, left_val, right_val, *);
                    break;
                case '/':
                    APPLY_BINARY_NUMERIC_OP(result, left_val, right_val, /);
                    break;
                case '&':
                    result.as.i = left_val.as.i & right_val.as.i;
                    break;
                case '^':
                    result.as.i = left_val.as.i ^ right_val.as.i;
                    break;
                case '|':
                    result.as.i = left_val.as.i | right_val.as.i;
                    break;

                default:
                    handleUnsupportedBinOp(intrptr, bin_node);
            }
        }
    } else {
        switch (bin_node->op[0]) {
            case '*':
                if (!isNumericVal(&left_val) || !isNumericVal(&right_val))
                    break;

                result.val_type = VAL_FLOAT;
                float arg1;
                float arg2;

                arg1 = (left_val.val_type == VAL_INT) ? (float)left_val.as.i
                                                      : left_val.as.f;

                arg2 = (right_val.val_type == VAL_INT) ? (float)right_val.as.i
                                                       : right_val.as.f;

                result.as.f = powf(arg1, arg2);
                break;
            case '<':
                if (isNumericVal(&left_val) && isNumericVal(&right_val))
                    if (bin_node->op[1] == '<')
                        result.as.i = left_val.as.i << right_val.as.i;
                    else
                        APPLY_BINARY_BOOL_OP(result, left_val, right_val, <=);
                else if (isObjectType(&left_val, OBJ_STR) &&
                         isObjectType(&right_val, OBJ_STR)) {
                    result.val_type = VAL_BOOL;

                    result.as.b = (strcmp(intrptr->tok_left->lexeme,
                                          intrptr->tok_right->lexeme) <= 0)
                                      ? true
                                      : false;
                } else {
                    handleUnsupportedBinOp(intrptr, bin_node);
                }
                return result;
            case '>':
                if (isNumericVal(&left_val) && isNumericVal(&right_val))
                    if (bin_node->op[1] == '>')
                        result.as.i = left_val.as.i >> right_val.as.i;
                    else
                        APPLY_BINARY_BOOL_OP(result, left_val, right_val, >=);
                else if (isObjectType(&left_val, OBJ_STR) &&
                         isObjectType(&right_val, OBJ_STR)) {
                    result.val_type = VAL_BOOL;

                    result.as.b = (strcmp(intrptr->tok_left->lexeme,
                                          intrptr->tok_right->lexeme) >= 0)
                                      ? true
                                      : false;
                } else {
                    handleUnsupportedBinOp(intrptr, bin_node);
                }
                return result;
            case '=':
                if (isBoolable(&left_val) && isBoolable(&right_val))
                    APPLY_BINARY_BOOL_OP(result, left_val, right_val, ==);
                else if (isObjectType(&left_val, OBJ_STR) &&
                         isObjectType(&right_val, OBJ_STR)) {
                    result.val_type = VAL_BOOL;

                    result.as.b = (strcmp(intrptr->tok_left->lexeme,
                                          intrptr->tok_right->lexeme) == 0)
                                      ? true
                                      : false;
                } else {
                    handleUnsupportedBinOp(intrptr, bin_node);
                }
                return result;
            case '!':
                if (isBoolable(&left_val) && isBoolable(&right_val))
                    APPLY_BINARY_BOOL_OP(result, left_val, right_val, !=);
                else if (isObjectType(&left_val, OBJ_STR) &&
                         isObjectType(&right_val, OBJ_STR)) {
                    result.val_type = VAL_BOOL;

                    result.as.b = (strcmp(intrptr->tok_left->lexeme,
                                          intrptr->tok_right->lexeme) != false)
                                      ? true
                                      : false;
                } else {
                    handleUnsupportedBinOp(intrptr, bin_node);
                }

                return result;
        }

        if (isBoolable(&left_val) && isBoolable(&right_val)) {
            switch (bin_node->op[0]) {
                case '|':
                    APPLY_BINARY_BOOL_OP(result, left_val, right_val, ||);
                    break;
                case '&':
                    APPLY_BINARY_BOOL_OP(result, left_val, right_val, &&);
                    break;
                default:
                    handleUnsupportedBinOp(intrptr, bin_node);
            }
        } else
            handleUnsupportedBinOp(intrptr, bin_node);
    }

    return result;
}

struct Value interpretUnaryNode(struct Interpreter* intrptr,
                                struct UnaryNode* un_node) {
    struct Value value = interpret(intrptr, un_node->operand, LEFT);

    if (!isBoolable(&value)) {
        handleUnsupportedUnOp(intrptr, un_node);
    }

    switch (un_node->op) {
        case '+':
            APPLY_UNARY_NUMERIC_OP(value, +);
            break;
        case '-':
            APPLY_UNARY_NUMERIC_OP(value, -);
            break;
        case '~':
            if (!isValueType(&value, VAL_INT)) {
                handleUnsupportedUnOp(intrptr, un_node);
            }

            value.as.i = ~value.as.i;
            break;
        case '!':
            if (isValueType(&value, VAL_INT)) {
                value.as.f = (float)value.as.i;
            }

            value.as.b = (value.as.f == 0.0f) ? true : false;
            break;
    }

    return value;
}

struct Value interpret(struct Interpreter* intrptr, struct Node* node,
                       enum LeftOrRight left_or_right) {
    switch (node->kind) {
        case NODE_INT:
            struct IntNode* int_n = (struct IntNode*)node;
            struct Value int_val = {.as.i = int_n->value, .val_type = VAL_INT};

            if (left_or_right == LEFT)
                intrptr->tok_left = int_n->tok;
            else
                intrptr->tok_right = int_n->tok;

            return int_val;

        case NODE_FLOAT:
            struct FloatNode* float_n = (struct FloatNode*)node;
            struct Value float_val = {.as.f = float_n->value,
                                      .val_type = VAL_FLOAT};

            if (left_or_right == LEFT)
                intrptr->tok_left = float_n->tok;
            else
                intrptr->tok_right = float_n->tok;

            return float_val;

        case NODE_BOOLEAN:
            struct BooleanNode* bool_n = (struct BooleanNode*)node;
            struct Value boolean_val = {.as.b = bool_n->value,
                                        .val_type = VAL_BOOL};

            if (left_or_right == LEFT)
                intrptr->tok_left = bool_n->tok;
            else
                intrptr->tok_right = bool_n->tok;

            return boolean_val;

        case NODE_STRING:
            struct StringNode* str_n = (struct StringNode*)node;
            struct ObjString* str = malloc(sizeof(struct ObjString));

            str->base.kind = OBJ_STR;
            str->string = str_n->string;
            str->size = str_n->size;

            struct Value str_val = {.as.obj = &str->base, .val_type = VAL_OBJ};

            if (left_or_right == LEFT)
                intrptr->tok_left = str_n->tok;
            else
                intrptr->tok_right = str_n->tok;

            return str_val;

        case NODE_UNARY:
            struct UnaryNode* un_n = (struct UnaryNode*)node;
            return interpretUnaryNode(intrptr, un_n);

        case NODE_BINARY:
            struct BinaryNode* bin_n = (struct BinaryNode*)node;
            return interpretBinaryNode(intrptr, bin_n);

        case NODE_EXPRESSION:
            struct ExpressionNode* exp_n = (struct ExpressionNode*)node;
            return interpret(intrptr, exp_n->operations, LEFT);

        case NODE_GROUPING:
            struct GroupingNode* grp_n = (struct GroupingNode*)node;
            return interpret(intrptr, grp_n->expr, LEFT);
    }
}

void initInterpreter(struct Interpreter* intrptr, struct SourceFile* source) {
    intrptr->source = source;
    intrptr->tok_left = NULL;
    intrptr->tok_right = NULL;
}

void printValue(struct Value* val) {
    switch (val->val_type) {
        case VAL_INT:
            printf("%d", val->as.i);
            break;
        case VAL_FLOAT:
            printf("%f", val->as.f);
            break;
        case VAL_BOOL:
            printf("%s", (val->as.b) ? "true" : "false");
            break;
        default:
            struct ObjString* string = (struct ObjString*)val->as.obj;

            printf("%.*s", (int)string->size, string->string);
    }
}