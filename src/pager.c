/*
 * pager.c - Page Replacement Algorithms
 * 
 * Simulates FIFO, LRU, and Optimal page replacement.
 * Shows frame state at each step and counts page faults.
 */

#include <stdio.h>
#include <string.h>
#include "pager.h"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define RESET   "\033[0m"

// check if a page is in frames
static int is_in_frames(int *frames, int n, int page) {
    for (int i = 0; i < n; i++) {
        if (frames[i] == page) return i;
    }
    return -1;
}

// print frames state
static void print_frames(int *frames, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        if (frames[i] == -1)
            printf(" -");
        else
            printf(" %d", frames[i]);
        if (i < n - 1) printf(",");
    }
    printf(" ]");
}

/* ============ FIFO Page Replacement ============ */
void page_replace_fifo(int frames, int *ref_string, int ref_len) {
    printf(MAGENTA "\n========== FIFO Page Replacement (%d frames) ==========\n" RESET, frames);
    
    int frame[MAX_FRAMES];
    int faults = 0;
    int ptr = 0;  // points to the oldest page (circular)
    
    // initialize frames to -1 (empty)
    for (int i = 0; i < frames; i++) frame[i] = -1;
    
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status");
    printf("─────────────────────────────────\n");
    
    for (int i = 0; i < ref_len; i++) {
        int page = ref_string[i];
        printf("  %-6d ", page);
        
        if (is_in_frames(frame, frames, page) != -1) {
            // hit
            print_frames(frame, frames);
            printf(GREEN "  HIT\n" RESET);
        } else {
            // fault - replace the oldest (FIFO)
            frame[ptr] = page;
            ptr = (ptr + 1) % frames;
            faults++;
            print_frames(frame, frames);
            printf(RED "  FAULT\n" RESET);
        }
    }
    
    printf("─────────────────────────────────\n");
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len);
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100);
}

/* ============ LRU Page Replacement ============ */
void page_replace_lru(int frames, int *ref_string, int ref_len) {
    printf(MAGENTA "\n========== LRU Page Replacement (%d frames) ==========\n" RESET, frames);
    
    int frame[MAX_FRAMES];
    int last_used[MAX_FRAMES];  // timestamp of last use
    int faults = 0;
    
    for (int i = 0; i < frames; i++) {
        frame[i] = -1;
        last_used[i] = -1;
    }
    
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status");
    printf("─────────────────────────────────\n");
    
    for (int i = 0; i < ref_len; i++) {
        int page = ref_string[i];
        printf("  %-6d ", page);
        
        int pos = is_in_frames(frame, frames, page);
        
        if (pos != -1) {
            // hit - update last used time
            last_used[pos] = i;
            print_frames(frame, frames);
            printf(GREEN "  HIT\n" RESET);
        } else {
            // fault - find LRU page
            int lru_idx = 0;
            int min_time = last_used[0];
            
            for (int j = 1; j < frames; j++) {
                if (last_used[j] < min_time) {
                    min_time = last_used[j];
                    lru_idx = j;
                }
            }
            
            frame[lru_idx] = page;
            last_used[lru_idx] = i;
            faults++;
            print_frames(frame, frames);
            printf(RED "  FAULT\n" RESET);
        }
    }
    
    printf("─────────────────────────────────\n");
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len);
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100);
}

/* ============ Optimal Page Replacement ============ */
void page_replace_optimal(int frames, int *ref_string, int ref_len) {
    printf(MAGENTA "\n========== Optimal Page Replacement (%d frames) ==========\n" RESET, frames);
    
    int frame[MAX_FRAMES];
    int faults = 0;
    
    for (int i = 0; i < frames; i++) frame[i] = -1;
    
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status");
    printf("─────────────────────────────────\n");
    
    for (int i = 0; i < ref_len; i++) {
        int page = ref_string[i];
        printf("  %-6d ", page);
        
        if (is_in_frames(frame, frames, page) != -1) {
            print_frames(frame, frames);
            printf(GREEN "  HIT\n" RESET);
            continue;
        }
        
        // fault
        faults++;
        
        // check if there's an empty frame
        int empty_slot = -1;
        for (int j = 0; j < frames; j++) {
            if (frame[j] == -1) {
                empty_slot = j;
                break;
            }
        }
        
        if (empty_slot != -1) {
            frame[empty_slot] = page;
        } else {
            // find the page that won't be used for the longest time
            int farthest = -1;
            int replace_idx = 0;
            
            for (int j = 0; j < frames; j++) {
                int next_use = ref_len; // default: never used again
                for (int k = i + 1; k < ref_len; k++) {
                    if (ref_string[k] == frame[j]) {
                        next_use = k;
                        break;
                    }
                }
                if (next_use > farthest) {
                    farthest = next_use;
                    replace_idx = j;
                }
            }
            
            frame[replace_idx] = page;
        }
        
        print_frames(frame, frames);
        printf(RED "  FAULT\n" RESET);
    }
    
    printf("─────────────────────────────────\n");
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len);
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100);
}

/* ============ Compare All ============ */
void page_replace_compare(int frames, int *ref_string, int ref_len) {
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n");
    printf("║   Comparing Page Replacement Algorithms   ║\n");
    printf("╚══════════════════════════════════════════╝\n" RESET);
    
    // run all three
    page_replace_fifo(frames, ref_string, ref_len);
    page_replace_lru(frames, ref_string, ref_len);
    page_replace_optimal(frames, ref_string, ref_len);
    
    // summary will be printed by each individual function
    printf(CYAN "Note: Optimal is theoretical (requires future knowledge)\n" RESET);
    printf(CYAN "      and gives the minimum possible page faults.\n\n" RESET);
}
