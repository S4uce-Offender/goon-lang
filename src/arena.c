#include "arena.h"

void initArena(struct Arena* arena, size_t capacity) {
    arena->memory = malloc(capacity);
    arena->capacity = capacity;
    arena->offset = 0;
}

void* arenaAlloc(struct Arena* arena, size_t size) {
    size_t aligned = (size + 7) & ~7;

    if (arena->offset + aligned >= arena->capacity) {
        return NULL;
    }

    void* ptr = arena->memory + arena->offset;
    arena->offset += aligned;
    return ptr;
}

void destroyArena(struct Arena* arena) { free(arena->memory); }