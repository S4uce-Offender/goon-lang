#ifndef MODEL_H
#define MODEL_H

#include "arena.h"
#include "lexer.h"
#include "terminal_colors.h"

#define NUM_COLORS 6

extern char* bracket_colors[];

char* getBracketColor(uint8_t hierarchy);

enum NodeKind {
    NODE_INT,
    NODE_FLOAT,
    NODE_BINARY,
    NODE_UNARY,
    NODE_EXPRESSION,
    NODE_GROUPING
};

struct Node {
    enum NodeKind kind;
};

struct IntNode {
    struct Node base;

    int value;
};

struct IntNode* createIntNode(struct Arena* arena, int value);

struct FloatNode {
    struct Node base;

    float value;
};

struct FloatNode* createFloatNode(struct Arena* arena, float value);

struct UnaryNode {
    struct Node base;

    char op;
    struct Node* operand;
};

struct UnaryNode* createUnaryNode(struct Arena* arena, char op,
                                  struct Node* operand);

void printUnaryNode(struct UnaryNode* un_node, uint8_t hierarchy);

struct BinaryNode {
    struct Node base;

    char op;

    struct Node* left;
    struct Node* right;
};

struct BinaryNode* createBinaryNode(struct Arena* arena, char op,
                                    struct Node* left, struct Node* right);

void printBinaryNode(struct BinaryNode* binary_node, uint8_t hierarchy);

struct ExpressionNode {
    struct Node base;

    struct Node* operations;
};

struct ExpressionNode* createExpressionNode(struct Arena* arena,
                                            struct Node* operations_start_ptr);

void printExpressionNode(struct ExpressionNode* expr_node, uint8_t hierarchy);

struct GroupingNode {
    struct Node base;

    struct Node* expr;
};

struct GroupingNode* createGroupingNode(struct Arena* arena,
                                        struct Node* expression);

void printGroupingNode(struct GroupingNode* g_node, uint8_t hierarchy);

void printTabs(uint8_t hierarchy);
void printNode(struct Node* n, uint8_t hierarchy);

#endif