#ifndef PARSER_H
#define PARSER_H

#include <assert.h>

#include "lexer.h"
#include "model.h"

struct Parser {
    struct TokenDA* tokens;
    uint64_t current_token;
};

struct Token* parserAdvance(struct Parser* p);
struct Token* parserPeek(struct Parser* p);
struct Token* getPreviousToken(struct Parser* p);
struct Token* expect(struct Parser* p, enum TokenType t);
bool parserIsNext(struct Parser* p, enum TokenType t);
bool matchToken(struct Parser* p, enum TokenType t);

struct Node* parsePrimary(struct Parser* p, struct Arena* arena);
struct Node* parsePower(struct Parser* p, struct Arena* arena);
struct Node* parseUnary(struct Parser* p, struct Arena* arena);
struct Node* parseFactor(struct Parser* p, struct Arena* arena);
struct Node* parseMultiplication(struct Parser* p, struct Arena* arena);
struct Node* parseAddition(struct Parser* p, struct Arena* arena);
struct Node* parseExpression(struct Parser* p, struct Arena* arena);
struct Node* parse(struct Parser* p, struct Arena* arena);

void initParser(struct Parser* p, struct TokenDA* tokens);

void printAST(struct Node* ast);

#endif