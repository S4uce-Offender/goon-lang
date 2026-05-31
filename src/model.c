#include "model.h"

char* bracket_colors[] = {RED, GRN, YEL, BLU, MAG, CYN};

char* getBracketColor(uint8_t hierarchy) {
    return bracket_colors[hierarchy % NUM_COLORS];
}

struct IntNode* createIntNode(struct Arena* arena, struct Token* tok,
                              int value) {
    struct IntNode* node = arenaAlloc(arena, sizeof(struct IntNode));

    node->base.kind = NODE_INT;
    node->tok = tok;
    node->value = value;

    return node;
}

struct FloatNode* createFloatNode(struct Arena* arena, struct Token* tok,
                                  float value) {
    struct FloatNode* node = arenaAlloc(arena, sizeof(struct IntNode));

    node->base.kind = NODE_FLOAT;
    node->tok = tok;
    node->value = value;

    return node;
}

struct UnaryNode* createUnaryNode(struct Arena* arena, char op,
                                  struct Node* operand) {
    struct UnaryNode* node = arenaAlloc(arena, sizeof(struct UnaryNode));

    node->base.kind = NODE_UNARY;

    node->op = op;
    node->operand = operand;

    return node;
}

void printUnaryNode(struct UnaryNode* un_node, uint8_t hierarchy) {
    char* hierarchy_color = getBracketColor(hierarchy);

    printf("%sUnaryNode%s%s{%s %c, ", hierarchy_color, COLOR_RESET,
           hierarchy_color, COLOR_RESET, un_node->op);

    if (un_node->operand->kind == NODE_INT) {
        struct IntNode* n = (struct IntNode*)un_node->operand;
        printf("%d %s}%s", n->value, hierarchy_color, COLOR_RESET);

    } else if (un_node->operand->kind == NODE_FLOAT) {
        struct FloatNode* n = (struct FloatNode*)un_node->operand;
        printf("%f %s}%s", n->value, hierarchy_color, COLOR_RESET);

    } else {
        printNode(un_node->operand, hierarchy + 1);

        printf("\n");
        printTabs(hierarchy);

        printf("%s}%s", hierarchy_color, COLOR_RESET);
    }
}

struct BinaryNode* createBinaryNode(struct Arena* arena, char* op,
                                    uint8_t op_len, struct Node* left,
                                    struct Node* right) {
    struct BinaryNode* node = arenaAlloc(arena, sizeof(struct BinaryNode));

    node->base.kind = NODE_BINARY;

    node->op = op;
    node->op_len = op_len;
    node->left = left;
    node->right = right;

    return node;
}

void printBinaryNode(struct BinaryNode* binary_node, uint8_t hierarchy) {
    char* hierarchy_color = getBracketColor(hierarchy);

    printf("%sBinNode%s%s{%s %.*s, ", hierarchy_color, COLOR_RESET,
           hierarchy_color, COLOR_RESET, binary_node->op_len, binary_node->op);

    if (binary_node->left->kind == NODE_INT) {
        struct IntNode* n = (struct IntNode*)binary_node->left;
        printf("%d", n->value);

    } else if (binary_node->left->kind == NODE_FLOAT) {
        struct FloatNode* n = (struct FloatNode*)binary_node->left;
        printf("%f", n->value);

    } else {
        printNode(binary_node->left, hierarchy + 1);
    }

    printf(", ");

    if (binary_node->right->kind == NODE_INT) {
        struct IntNode* n = (struct IntNode*)binary_node->right;
        printf("%d %s}%s", n->value, hierarchy_color, COLOR_RESET);

    } else if (binary_node->right->kind == NODE_FLOAT) {
        struct FloatNode* n = (struct FloatNode*)binary_node->right;
        printf("%f %s}%s", n->value, hierarchy_color, COLOR_RESET);

    } else {
        printNode(binary_node->right, hierarchy + 1);

        printf("\n");
        printTabs(hierarchy);

        printf("%s}%s", hierarchy_color, COLOR_RESET);
    }
}

struct ExpressionNode* createExpressionNode(struct Arena* arena,
                                            struct Node* operations_start_ptr) {
    struct ExpressionNode* node =
        arenaAlloc(arena, sizeof(struct GroupingNode));

    node->base.kind = NODE_EXPRESSION;
    node->operations = operations_start_ptr;

    return node;
}

struct GroupingNode* createGroupingNode(struct Arena* arena,
                                        struct Node* expression) {
    struct GroupingNode* node = arenaAlloc(arena, sizeof(struct GroupingNode));

    node->base.kind = NODE_GROUPING;
    node->expr = expression;

    return node;
}

void printExpressionNode(struct ExpressionNode* expr_node, uint8_t hierarchy) {
    char* hierarchy_color = getBracketColor(hierarchy);

    printf("%sExpression%s%s{%s", hierarchy_color, COLOR_RESET, hierarchy_color,
           COLOR_RESET);

    printNode(expr_node->operations, hierarchy + 1);

    printf("\n");
    printTabs(hierarchy);

    printf("%s}%s", hierarchy_color, COLOR_RESET);
}

void printGroupingNode(struct GroupingNode* g_node, uint8_t hierarchy) {
    char* hierarchy_color = getBracketColor(hierarchy);

    printf("%sGrouping%s%s{%s", hierarchy_color, COLOR_RESET, hierarchy_color,
           COLOR_RESET);

    printNode(g_node->expr, hierarchy + 1);

    printf("\n");
    printTabs(hierarchy);

    printf("%s}%s", hierarchy_color, COLOR_RESET);
}

struct BooleanNode* createBooleanNode(struct Arena* arena, struct Token* tok,
                                      bool value) {
    struct BooleanNode* node = arenaAlloc(arena, sizeof(struct BooleanNode));

    node->base.kind = NODE_BOOLEAN;
    node->tok = tok;
    node->value = value;

    return node;
}

void printBooleanNode(struct BooleanNode* bool_node, uint8_t hierarchy) {
    char* hierarchy_color = getBracketColor(hierarchy);

    printf("%sBoolean%s%s{%s ", hierarchy_color, COLOR_RESET, hierarchy_color,
           COLOR_RESET);

    if (bool_node->value)
        printf("true");
    else
        printf("false");

    printf("%s }%s", hierarchy_color, COLOR_RESET);
}

struct StringNode* createStringNode(struct Arena* arena, struct Token* tok,
                                    char* string, uint64_t size) {
    struct StringNode* node = arenaAlloc(arena, sizeof(struct StringNode));

    node->base.kind = NODE_STRING;
    node->tok = tok;
    node->string = string;
    node->size = size;

    return node;
}

void printStringNode(struct StringNode* str_node, uint8_t hierarchy) {
    char* hierarchy_color = getBracketColor(hierarchy);

    printf("%sString%s%s{%s ", hierarchy_color, COLOR_RESET, hierarchy_color,
           COLOR_RESET);

    printf("%s", str_node->string);
    printf("%s }%s", hierarchy_color, COLOR_RESET);
}

void printTabs(uint8_t hierarchy) {
    for (int i = 0; i < hierarchy; i++) printf("\t");
}

void printNode(struct Node* n, uint8_t hierarchy) {
    printf("\n");
    printTabs(hierarchy);
    switch (n->kind) {
        case NODE_BINARY:
            printBinaryNode((struct BinaryNode*)n, hierarchy);
            break;
        case NODE_UNARY:
            printUnaryNode((struct UnaryNode*)n, hierarchy);
            break;
        case NODE_BOOLEAN:
            printBooleanNode((struct BooleanNode*)n, hierarchy);
            break;
        case NODE_STRING:
            printStringNode((struct StringNode*)n, hierarchy);
            break;
        case NODE_EXPRESSION:
            printExpressionNode((struct ExpressionNode*)n, hierarchy);
            break;
        case NODE_GROUPING:
            printGroupingNode((struct GroupingNode*)n, hierarchy);
            break;
        default:
            printf("Unkown type");
            break;
    }
}