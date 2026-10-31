/*
 * memory.c - Memory Management Module
 * 
 * Simulates a 1024 MB memory arena using a doubly linked list.
 * Supports First Fit, Best Fit, and Worst Fit allocation.
 * On free, adjacent free blocks are merged together.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define RESET   "\033[0m"

// head of memory block list
static MemBlock *mem_head = NULL;

void init_memory(void) {
    // free existing list if any
    cleanup_memory();
    
    // create one big free block
    mem_head = (MemBlock *)malloc(sizeof(MemBlock));
    if (mem_head == NULL) {
        printf(RED "Error: Failed to initialize memory\n" RESET);
        return;
    }
    mem_head->offset = 0;
    mem_head->size = TOTAL_MEMORY;
    mem_head->allocated = 0;
    strcpy(mem_head->process_name, "");
    mem_head->next = NULL;
    mem_head->prev = NULL;
}

// split a block: allocate 'size' from block 'blk', remaining stays free
static void split_block(MemBlock *blk, int size) {
    if (blk->size == size) {
        // exact fit, no split needed
        return;
    }
    
    // create a new free block for the remaining part
    MemBlock *new_block = (MemBlock *)malloc(sizeof(MemBlock));
    new_block->offset = blk->offset + size;
    new_block->size = blk->size - size;
    new_block->allocated = 0;
    strcpy(new_block->process_name, "");
    
    // insert after blk
    new_block->next = blk->next;
    new_block->prev = blk;
    if (blk->next != NULL)
        blk->next->prev = new_block;
    blk->next = new_block;
    
    blk->size = size;
}

// merge adjacent free blocks around 'blk'
static void merge_blocks(MemBlock *blk) {
    // merge with next block if it's free
    if (blk->next != NULL && !blk->next->allocated) {
        MemBlock *next = blk->next;
        blk->size += next->size;
        blk->next = next->next;
        if (next->next != NULL)
            next->next->prev = blk;
        free(next);
    }
    
    // merge with previous block if it's free
    if (blk->prev != NULL && !blk->prev->allocated) {
        MemBlock *prev = blk->prev;
        prev->size += blk->size;
        prev->next = blk->next;
        if (blk->next != NULL)
            blk->next->prev = prev;
        free(blk);
    }
}

int mem_allocate(const char *pname, int size, AllocPolicy policy) {
    if (size <= 0 || size > TOTAL_MEMORY) {
        printf(RED "Error: Invalid size %d (must be 1-%d)\n" RESET, size, TOTAL_MEMORY);
        return -1;
    }
    
    // check if process already has memory
    MemBlock *cur = mem_head;
    while (cur != NULL) {
        if (cur->allocated && strcmp(cur->process_name, pname) == 0) {
            printf(RED "Error: '%s' already has memory allocated\n" RESET, pname);
            return -1;
        }
        cur = cur->next;
    }
    
    MemBlock *chosen = NULL;
    
    if (policy == FIRST_FIT) {
        // first block that fits
        cur = mem_head;
        while (cur != NULL) {
            if (!cur->allocated && cur->size >= size) {
                chosen = cur;
                break;
            }
            cur = cur->next;
        }
    }
    else if (policy == BEST_FIT) {
        // smallest block that fits
        int min_size = TOTAL_MEMORY + 1;
        cur = mem_head;
        while (cur != NULL) {
            if (!cur->allocated && cur->size >= size && cur->size < min_size) {
                min_size = cur->size;
                chosen = cur;
            }
            cur = cur->next;
        }
    }
    else if (policy == WORST_FIT) {
        // largest block
        int max_size = -1;
        cur = mem_head;
        while (cur != NULL) {
            if (!cur->allocated && cur->size >= size && cur->size > max_size) {
                max_size = cur->size;
                chosen = cur;
            }
            cur = cur->next;
        }
    }
    
    if (chosen == NULL) {
        printf(RED "Error: Not enough contiguous memory for %d MB\n" RESET, size);
        return -1;
    }
    
    // allocate
    split_block(chosen, size);
    chosen->allocated = 1;
    strncpy(chosen->process_name, pname, 15);
    chosen->process_name[15] = '\0';
    
    const char *policy_names[] = {"First Fit", "Best Fit", "Worst Fit"};
    printf(GREEN "  [%s] Allocated at offset %d\n" RESET, policy_names[policy], chosen->offset);
    return 0;
}

int mem_free(const char *pname) {
    MemBlock *cur = mem_head;
    while (cur != NULL) {
        if (cur->allocated && strcmp(cur->process_name, pname) == 0) {
            cur->allocated = 0;
            strcpy(cur->process_name, "");
            merge_blocks(cur);
            return 0;
        }
        cur = cur->next;
    }
    printf(RED "Error: No memory allocated to '%s'\n" RESET, pname);
    return -1;
}

void mem_status(void) {
    printf(CYAN "\n=== Memory Map (Total: %d MB) ===\n" RESET, TOTAL_MEMORY);
    printf("────────────────────────────────────────────────\n");
    
    MemBlock *cur = mem_head;
    int total_free = 0, total_alloc = 0;
    
    while (cur != NULL) {
        int end = cur->offset + cur->size - 1;
        if (cur->allocated) {
            printf(GREEN "  [%4d - %4d]  %-10s  %4d MB  ALLOCATED\n" RESET,
                   cur->offset, end, cur->process_name, cur->size);
            total_alloc += cur->size;
        } else {
            printf(YELLOW "  [%4d - %4d]  %-10s  %4d MB  FREE\n" RESET,
                   cur->offset, end, "---", cur->size);
            total_free += cur->size;
        }
        cur = cur->next;
    }
    
    printf("────────────────────────────────────────────────\n");
    printf("  Allocated: %d MB | Free: %d MB\n\n", total_alloc, total_free);
}

void cleanup_memory(void) {
    MemBlock *cur = mem_head;
    while (cur != NULL) {
        MemBlock *next = cur->next;
        free(cur);
        cur = next;
    }
    mem_head = NULL;
}
