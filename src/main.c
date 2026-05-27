#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

uint32_t getFileSize(FILE* file) {
    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    rewind(file);
    return size;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Invalid argument count.\n [USAGE]: gooner <filename>.goon");
        return 1;
    }

    char* text_buf;
    char* filename = argv[1];

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        perror("Invalid file path");
        return 2;
    }

    uint64_t file_size = getFileSize(file);
    text_buf = malloc(file_size);

    fread(text_buf, file_size, 1, file);

    struct Lexer lex = {0};
    initLexer(&lex, text_buf, file_size);

    lex.tokenize(&lex);

    for (int i = 0; i < lex.tokens.size; i++) {
        printToken(&lex, &lex.tokens.data[i]);
    }

    struct Parser p = {0};
    initParser(&p, &lex.tokens);

    struct Arena arena = {0};
    initArena(&arena, ARENA_SIZE);

    struct Node* ast = parse(&p, &arena);
    if (ast == NULL) {
        printf("AST is empty u done fucked up\n");
    } else {
        printAST(ast);
        printf("\n\n");
    }

    struct NumericalVal ret_val = interpret(ast);

    if (ret_val.return_val == RET_INT) {
        printf("%d", ret_val.as.i);
    } else {
        printf("%f", ret_val.as.f);
    }

    printf("\n\n");

    free(text_buf);
    fclose(file);
    freeLexer(&lex);
    destroyArena(&arena);
    return 0;
}