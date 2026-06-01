#ifndef ERRORS_H
#define ERRORS_H

#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "source.h"
#include "terminal_colors.h"

struct Token;

enum LexerErrorType { LEX_ERR_UNKNOWN_SYMBOL, LEX_ERR_UNTERMINATED_STR };

enum ParserErrorType {
    PRSR_ERR_MISSING_RBRACE,
    PRSR_ERR_MISSING_LBRACE,
    PRSR_ERR_UNEXPECTED_TOKEN
};

enum InterpreterErrorType {
    INTPTR_ERR_UNSUPPORTED_UN_OP,
    INTPTR_ERR_UNSUPPORTED_BIN_OP
};

void lexerThrowError(enum LexerErrorType error_type, char* error_message,
                     struct SourceFile* source, uint64_t line,
                     uint64_t where_firstchar, uint64_t offset);

void parserThrowError(enum ParserErrorType error_type, char* error_message,
                      struct SourceFile* source, uint64_t line,
                      uint64_t where_firstchar, uint64_t offset);

void interpreterThrowError(enum InterpreterErrorType error_type,
                           char* error_message, struct SourceFile* source,
                           struct Token* tok1, struct Token* tok2);

#endif