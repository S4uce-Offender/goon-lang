#include "parser.h"

bool parserAtEnd(struct Parser* p) {
    return parserPeek(p)->token_type == TOK_EOF;
}

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

        char saved = tok->lexeme[tok->len];
        tok->lexeme[tok->len] = '\0';

        struct IntNode* n =
            createIntNode(arena, tok, atoi(getPreviousToken(p)->lexeme));

        tok->lexeme[tok->len] = saved;
        return &n->base;
    }
    if (matchToken(p, TOK_FLOAT)) {
        struct Token* tok = getPreviousToken(p);

        char saved = tok->lexeme[tok->len];
        tok->lexeme[tok->len] = '\0';

        struct FloatNode* n =
            createFloatNode(arena, tok, strtof(tok->lexeme, NULL));

        tok->lexeme[tok->len] = saved;
        return &n->base;
    }
    if (matchToken(p, TOK_TRUE) || matchToken(p, TOK_FALSE)) {
        struct Token* tok = getPreviousToken(p);
        bool value = (tok->token_type == TOK_TRUE) ? true : false;

        struct BooleanNode* n = createBooleanNode(arena, tok, value);

        return &n->base;
    }
    if (matchToken(p, TOK_STRING)) {
        struct Token* tok = getPreviousToken(p);

        struct StringNode* n =
            createStringNode(arena, tok, tok->lexeme, tok->len);

        return &n->base;
    }
    if (matchToken(p, TOK_LBRACE)) {
        struct Token* lbrace_tok = getPreviousToken(p);

        struct Node* expr = parseExpression(p, arena);
        if (!matchToken(p, TOK_RBRACE)) {
            parserThrowError(PRSR_ERR_MISSING_RBRACE,
                             "Expected ')' before end of line", p->source,
                             lbrace_tok->line, lbrace_tok->line_start,
                             lbrace_tok->offset);
        } else {
            struct GroupingNode* n = createGroupingNode(arena, expr);

            return &n->base;
        }
    }

    struct Token* tok = parserPeek(p);

    parserThrowError(PRSR_ERR_UNEXPECTED_TOKEN, "Unexpected token", p->source,
                     tok->line, tok->line_start, tok->offset);

    return NULL;
}

struct Node* parsePower(struct Parser* p, struct Arena* arena) {
    struct Node* left = parsePrimary(p, arena);

    while (matchToken(p, TOK_POW)) {
        struct Token* op = getPreviousToken(p);
        struct Node* right = parsePower(p, arena);
        struct BinaryNode* bin_node =
            createBinaryNode(arena, op->lexeme, 2, left, right);

        return &bin_node->base;
    }

    return left;
}

struct Node* parseUnary(struct Parser* p, struct Arena* arena) {
    if (matchToken(p, TOK_PLUS) || matchToken(p, TOK_MINUS) ||
        matchToken(p, TOK_BIT_NOT)) {
        struct Token* op = getPreviousToken(p);
        struct Node* operand = parseUnary(p, arena);

        struct UnaryNode* un_op = createUnaryNode(arena, *op->lexeme, operand);
        return &un_op->base;
    }

    return parsePower(p, arena);
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
            createBinaryNode(arena, op->lexeme, 1, expr, right);

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
            createBinaryNode(arena, op->lexeme, 1, expr, right);

        expr = &bin_node->base;
    }

    return expr;
}

struct Node* parseComparison(struct Parser* p, struct Arena* arena) {
    struct Node* expr = parseAddition(p, arena);

    while (matchToken(p, TOK_LESS) || matchToken(p, TOK_LESS_EQUALS) ||
           matchToken(p, TOK_GREATER) || matchToken(p, TOK_GREATER_EQUALS) ||
           matchToken(p, TOK_EQUALS) || matchToken(p, TOK_NOT_EQUALS)) {
        struct Token* op = getPreviousToken(p);
        struct Node* right = parseAddition(p, arena);
        struct BinaryNode* bin_node =
            createBinaryNode(arena, op->lexeme, op->len, expr, right);

        expr = &bin_node->base;
    }

    return expr;
}

struct Node* parseBooleanOR(struct Parser* p, struct Arena* arena) {
    struct Node* expr = parseComparison(p, arena);

    while (matchToken(p, TOK_BOOL_OR)) {
        struct Token* op = getPreviousToken(p);
        struct Node* right = parseComparison(p, arena);
        struct BinaryNode* bin_node =
            createBinaryNode(arena, op->lexeme, 2, expr, right);

        expr = &bin_node->base;
    }

    return expr;
}

struct Node* parseBooleanAND(struct Parser* p, struct Arena* arena) {
    struct Node* expr = parseBooleanOR(p, arena);

    while (matchToken(p, TOK_BOOL_AND)) {
        struct Token* op = getPreviousToken(p);
        struct Node* right = parseBooleanOR(p, arena);
        struct BinaryNode* bin_node =
            createBinaryNode(arena, op->lexeme, 2, expr, right);

        expr = &bin_node->base;
    }

    return expr;
}
struct Node* parseBooleanNOT(struct Parser* p, struct Arena* arena) {
    while (matchToken(p, TOK_BOOL_NOT)) {
        struct Token* op = getPreviousToken(p);
        struct Node* operand = parseBooleanNOT(p, arena);

        struct UnaryNode* un_op = createUnaryNode(arena, *op->lexeme, operand);
        return &un_op->base;
    }

    return parseBooleanAND(p, arena);
}

struct Node* parseExpression(struct Parser* p, struct Arena* arena) {
    struct ExpressionNode* expr =
        createExpressionNode(arena, parseBooleanNOT(p, arena));

    return &expr->base;
}

struct Node* parse(struct Parser* p, struct Arena* arena) {
    struct Node* ast = parseExpression(p, arena);

    if (!parserAtEnd(p)) {
        if (matchToken(p, TOK_RBRACE)) {
            struct Token* tok = getPreviousToken(p);

            parserThrowError(PRSR_ERR_MISSING_LBRACE,
                             "Missing '(' before ')' token", p->source,
                             tok->line, tok->line_start, tok->offset);
        }
    }

    return ast;
}

void printAST(struct Node* ast) { printNode(ast, 0); }

void initParser(struct Parser* p, struct TokenDA* tokens,
                struct SourceFile* source) {
    p->tokens = tokens;
    p->source = source;
    p->current_token = 0;
}
