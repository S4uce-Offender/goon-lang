#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define ARENA_SIZE 1024 * 1024

struct Arena {
    uint8_t* memory;
    size_t capacity, offset;
};

void initArena(struct Arena* arena, size_t capacity);
void* arenaAlloc(struct Arena* arena, size_t size);
void destroyArena(struct Arena* arena);

#endif