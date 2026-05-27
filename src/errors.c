#include "errors.h"

#include "lexer.h"

void printRemainingLine(char* start) {
    while (*start != '\n') {
        putchar(*start++);
    }
}

void printSpaces(uint32_t num) {
    while (num--) putchar(' ');
}

void lexerThrowError(enum LexerErrorType error_type, char* error_message,
                     struct Lexer* lex) {
    printf("LEXER ERROR: %s\n%ld |\t", error_message, lex->line);

    switch (error_type) {
        case LEX_ERR_UNKNOWN_SYMBOL:
            // Print everything before symbol
            uint32_t offset = lex->current_char - lex->where_firstchar;

            printf("%.*s", (offset - 1), &lex->source[lex->where_firstchar]);
            printf("%s%c%s", BRED, lex->source[lex->current_char - 1],
                   COLOR_RESET);
            printRemainingLine(&lex->source[lex->current_char]);
            printf("\n  |\t");
            printSpaces(offset - 1);
            printf("%s^%s", BRED, COLOR_RESET);
    }

    printf("\n");

    exit(1);
}