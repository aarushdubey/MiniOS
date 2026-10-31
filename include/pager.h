#ifndef PAGER_H
#define PAGER_H

#define MAX_REF_STRING 50
#define MAX_FRAMES 10

// page replacement algorithms
void page_replace_fifo(int frames, int *ref_string, int ref_len);
void page_replace_lru(int frames, int *ref_string, int ref_len);
void page_replace_optimal(int frames, int *ref_string, int ref_len);

// run all three and compare
void page_replace_compare(int frames, int *ref_string, int ref_len);

#endif
