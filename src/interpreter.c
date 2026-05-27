#include "interpreter.h"

struct NumericalVal interpretBinaryNode(struct BinaryNode* bin_node) {
    struct NumericalVal left_val = interpret(bin_node->left);
    struct NumericalVal right_val = interpret(bin_node->right);
    struct NumericalVal result = {0};

    if (left_val.return_val == RET_FLOAT || right_val.return_val == RET_FLOAT)
        result.return_val = RET_FLOAT;
    else
        result.return_val = RET_INT;

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
        result.return_val = RET_FLOAT;
        float arg1;
        float arg2;

        arg1 = (left_val.return_val == RET_INT) ? (float)left_val.as.i
                                                : left_val.as.f;

        arg2 = (right_val.return_val == RET_INT) ? (float)right_val.as.i
                                                 : right_val.as.f;

        result.as.f = powf(arg1, arg2);
    }

    return result;
}

struct NumericalVal interpretUnaryNode(struct UnaryNode* un_node) {
    struct NumericalVal value = interpret(un_node->operand);

    switch (un_node->op) {
        case '+':
            APPLY_UNARY_NUMERIC_OP(value, +);
            break;
        case '-':
            APPLY_UNARY_NUMERIC_OP(value, -);
            break;
        case '~':
            if (value.return_val != RET_INT) {
                exit(1);
            }

            value.as.i = ~value.as.i;
    }

    return value;
}

struct NumericalVal interpret(struct Node* node) {
    switch (node->kind) {
        case NODE_INT:
            struct IntNode* int_n = (struct IntNode*)node;
            struct NumericalVal int_val = {.as.i = int_n->value,
                                           .return_val = RET_INT};

            return int_val;
            break;
        case NODE_FLOAT:
            struct FloatNode* float_n = (struct FloatNode*)node;
            struct NumericalVal float_val = {.as.f = float_n->value,
                                             .return_val = RET_FLOAT};

            return float_val;
            break;
        case NODE_UNARY:
            struct UnaryNode* un_n = (struct UnaryNode*)node;
            return interpretUnaryNode(un_n);
        case NODE_BINARY:
            struct BinaryNode* bin_n = (struct BinaryNode*)node;
            return interpretBinaryNode(bin_n);
        case NODE_EXPRESSION:
            struct ExpressionNode* exp_n = (struct ExpressionNode*)node;
            interpret(exp_n->operations);
        case NODE_GROUPING:
            struct GroupingNode* grp_n = (struct GroupingNode*)node;
            interpret(grp_n->expr);
    }
}