#include "interpreter.h"

bool isValue(struct Value* val, enum ValueType val_type) {
    return val->val_type == val_type;
}

bool isNumericVal(struct Value* val) {
    return val->val_type == VAL_INT || val->val_type == VAL_FLOAT;
}

struct Value interpretBinaryNode(struct BinaryNode* bin_node) {
    struct Value left_val = interpret(bin_node->left);
    struct Value right_val = interpret(bin_node->right);
    struct Value result = {0};

    if (isValue(&left_val, VAL_FLOAT) || isValue(&right_val, VAL_FLOAT))
        result.val_type = VAL_FLOAT;
    else
        result.val_type = VAL_INT;

    if (bin_node->op_len == 1) {
        switch (bin_node->op[0]) {
            case '+':
                APPLY_BINARY_NUMERIC_OP(result, left_val, right_val, +);
                break;
            case '-':
                APPLY_BINARY_NUMERIC_OP(result, left_val, right_val, -);
                break;
            case '*':
                APPLY_BINARY_NUMERIC_OP(result, left_val, right_val, *);
                break;
            case '/':
                APPLY_BINARY_NUMERIC_OP(result, left_val, right_val, /);
                break;
        }
    } else {
        if (!isNumericVal(&left_val) || !isNumericVal(&right_val)) {
            printf(
                "Binary operation '**' not supported between <Placeholder> and "
                "<Placeholder>");
            exit(1);
        }

        result.val_type = VAL_FLOAT;
        float arg1;
        float arg2;

        arg1 = (left_val.val_type == VAL_INT) ? (float)left_val.as.i
                                              : left_val.as.f;

        arg2 = (right_val.val_type == VAL_INT) ? (float)right_val.as.i
                                               : right_val.as.f;

        result.as.f = powf(arg1, arg2);
    }

    return result;
}

struct Value interpretUnaryNode(struct UnaryNode* un_node) {
    struct Value value = interpret(un_node->operand);

    switch (un_node->op) {
        case '+':
            APPLY_UNARY_NUMERIC_OP(value, +);
            break;
        case '-':
            APPLY_UNARY_NUMERIC_OP(value, -);
            break;
        case '~':
            if (value.val_type != VAL_INT) {
                exit(1);
            }

            value.as.i = ~value.as.i;
    }

    return value;
}

struct Value interpret(struct Node* node) {
    switch (node->kind) {
        case NODE_INT:
            struct IntNode* int_n = (struct IntNode*)node;
            struct Value int_val = {.as.i = int_n->value, .val_type = VAL_INT};

            return int_val;
            break;
        case NODE_FLOAT:
            struct FloatNode* float_n = (struct FloatNode*)node;
            struct Value float_val = {.as.f = float_n->value,
                                      .val_type = VAL_FLOAT};

            return float_val;
            break;
        case NODE_BOOLEAN:
            struct BooleanNode* bool_n = (struct BooleanNode*)node;
            struct Value boolean_val = {.as.b = bool_n->value,
                                        .val_type = VAL_BOOL};
            return boolean_val;
            break;
        case NODE_STRING:
            struct StringNode* str_n = (struct StringNode*)node;
            struct ObjString* str = malloc(sizeof(struct ObjString));

            str->base.kind = OBJ_STR;
            str->string = str_n->string;
            str->size = str_n->size;

            struct Value str_val = {.as.obj = &str->base, .val_type = VAL_OBJ};

            return str_val;
        case NODE_UNARY:
            struct UnaryNode* un_n = (struct UnaryNode*)node;
            return interpretUnaryNode(un_n);
        case NODE_BINARY:
            struct BinaryNode* bin_n = (struct BinaryNode*)node;
            return interpretBinaryNode(bin_n);
        case NODE_EXPRESSION:
            struct ExpressionNode* exp_n = (struct ExpressionNode*)node;
            return interpret(exp_n->operations);
        case NODE_GROUPING:
            struct GroupingNode* grp_n = (struct GroupingNode*)node;
            return interpret(grp_n->expr);
    }
}