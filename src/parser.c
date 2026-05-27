#include "parser.h"

struct Token* parserAdvance(struct Parser* p) {
    struct Token* token = &p->tokens->data[p->current_token];
    p->current_token++;
    return token;
}

struct Token* getPreviousToken(struct Parser* p) {
    return &p->tokens->data[p->current_token - 1];
}

struct Token* parserPeek(struct Parser* p) {
    return &p->tokens->data[p->current_token];
}

bool parserIsNext(struct Parser* p, enum TokenType t) {
    if (p->current_token >= p->tokens->size) return false;
    return p->tokens->data[p->current_token + 1].token_type == t;
}

struct Token* expect(struct Parser* p, enum TokenType t) {
    if (p->current_token >= p->tokens->size) {
        printf("Found %s at the end of parsing!",
               tokenTypeToString(getPreviousToken(p)->token_type));
        return NULL;
    } else if (parserPeek(p)->token_type == t) {
        return parserAdvance(p);
    } else {
        printf("Expected %s, found %s", tokenTypeToString(t),
               tokenTypeToString(parserPeek(p)->token_type));
    }
}

bool matchToken(struct Parser* p, enum TokenType t) {
    if (p->tokens->data[p->current_token].token_type != t) return false;

    p->current_token++;
    return true;
}

struct Node* parsePrimary(struct Parser* p, struct Arena* arena) {
    if (matchToken(p, TOK_INT)) {
        struct Token* tok = getPreviousToken(p);
        char* int_lexeme = malloc(tok->len + 1);
        strncpy(int_lexeme, tok->lexeme, tok->len);
        int_lexeme[tok->len] = '\0';

        struct IntNode* n =
            createIntNode(arena, atoi(getPreviousToken(p)->lexeme));

        free(int_lexeme);
        return &n->base;
    } else if (matchToken(p, TOK_FLOAT)) {
        struct Token* tok = getPreviousToken(p);

        char saved = tok->lexeme[tok->len];
        tok->lexeme[tok->len] = '\0';

        struct FloatNode* n = createFloatNode(arena, strtof(tok->lexeme, NULL));

        tok->lexeme[tok->len] = saved;
        return &n->base;
    } else if (matchToken(p, TOK_LBRACE)) {
        struct Node* expr = parseExpression(p, arena);
        if (!matchToken(p, TOK_RBRACE)) {
            printf("Error: ')' expected.");
            exit(12);
        } else {
            struct GroupingNode* n = createGroupingNode(arena, expr);

            return &n->base;
        }
    }
}

struct Node* parseUnary(struct Parser* p, struct Arena* arena) {
    if (matchToken(p, TOK_PLUS) || matchToken(p, TOK_MINUS) ||
        matchToken(p, TOK_NOT)) {
        struct Token* op = getPreviousToken(p);
        struct Node* operand = parsePrimary(p, arena);

        struct UnaryNode* un_op = createUnaryNode(arena, *op->lexeme, operand);
        return &un_op->base;
    }

    return parsePrimary(p, arena);
}

struct Node* parseFactor(struct Parser* p, struct Arena* arena) {
    return parseUnary(p, arena);
}

struct Node* parseMultiplication(struct Parser* p, struct Arena* arena) {
    struct Node* expr = parseFactor(p, arena);
    while (matchToken(p, TOK_TIMES) || matchToken(p, TOK_DIV)) {
        struct Token* op = getPreviousToken(p);
        struct Node* right = parseFactor(p, arena);
        struct BinaryNode* bin_node =
            createBinaryNode(arena, *op->lexeme, expr, right);

        expr = &bin_node->base;
    }

    return expr;
}

struct Node* parseAddition(struct Parser* p, struct Arena* arena) {
    struct Node* expr = parseMultiplication(p, arena);
    while (matchToken(p, TOK_PLUS) || matchToken(p, TOK_MINUS)) {
        struct Token* op = getPreviousToken(p);
        struct Node* right = parseMultiplication(p, arena);
        struct BinaryNode* bin_node =
            createBinaryNode(arena, *op->lexeme, expr, right);

        expr = &bin_node->base;
    }

    return expr;
}

struct Node* parseExpression(struct Parser* p, struct Arena* arena) {
    struct ExpressionNode* expr =
        createExpressionNode(arena, parseAddition(p, arena));

    return &expr->base;
}

struct Node* parse(struct Parser* p, struct Arena* arena) {
    struct Node* ast = parseExpression(p, arena);

    return ast;
}

void printAST(struct Node* ast) { printNode(ast, 0); }

void initParser(struct Parser* p, struct TokenDA* tokens) {
    p->tokens = tokens;
    p->current_token = 0;
}
