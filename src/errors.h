#ifndef ERRORS_H
#define ERRORS_H

#include <stdint.h>
#include <stdlib.h>

#include "terminal_colors.h"

struct Lexer;

enum LexerErrorType { LEX_ERR_UNKNOWN_SYMBOL };

void lexerThrowError(enum LexerErrorType error_type, char* error_message,
                     struct Lexer* lex);

#endif