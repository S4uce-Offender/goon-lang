/*
A header that simply stores a struct that has the text of the source file
along with extra info to make constructing errors easy. Implemented in
the Lexer and Parser structs as well as the interpreter source file.
*/

#ifndef SOURCE_H
#define SOURCE_H

#include <stddef.h>

struct SourceFile {
    char* text;
    size_t length;
};

#endif