#ifndef MEMORY_H
#define MEMORY_H

#define TOTAL_MEMORY 1024  // 1024 MB total

// memory block node (doubly linked list)
typedef struct MemBlock {
    int offset;
    int size;
    int allocated;          // 1 = allocated, 0 = free
    char process_name[16];  // which process owns this block
    struct MemBlock *next;
    struct MemBlock *prev;
} MemBlock;

// memory allocation policies
typedef enum {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
} AllocPolicy;

// function declarations
void init_memory(void);
int mem_allocate(const char *pname, int size, AllocPolicy policy);
int mem_free(const char *pname);
void mem_status(void);
void cleanup_memory(void);

#endif
