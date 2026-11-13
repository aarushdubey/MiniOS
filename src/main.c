/*
 * MiniOS - Operating System Simulator
 * Main shell / command loop
 * 
 * This is the entry point. It provides a simple command line
 * interface where the user can interact with different OS modules.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "sync.h"
#include "pager.h"
#include "disk.h"

// ANSI color codes for terminal output
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_RESET   "\033[0m"

// clear screen - works on both windows and unix
void clear_screen(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void print_banner(void) {
    printf(COLOR_CYAN);
    printf("╔══════════════════════════════════════════╗\n");
    printf("║          MiniOS v1.0 Simulator           ║\n");
    printf("║     Operating System Course Project      ║\n");
    printf("╚══════════════════════════════════════════╝\n");
    printf(COLOR_RESET);
    printf("Type 'help' for list of commands.\n\n");
}

void print_help(void) {
    printf(COLOR_YELLOW "\n=== MiniOS Commands ===" COLOR_RESET "\n\n");
    
    printf(COLOR_GREEN "Process Management:\n" COLOR_RESET);
    printf("  create <name> <burst> <priority> [arrival]  - Create a new process\n");
    printf("  list                                        - List all processes\n");
    printf("  kill <pid>                                  - Terminate a process\n");
    printf("  clear_all                                   - Remove all processes\n\n");
    
    printf(COLOR_GREEN "CPU Scheduling:\n" COLOR_RESET);
    printf("  schedule fcfs                 - First Come First Serve\n");
    printf("  schedule sjf                  - Shortest Job First\n");
    printf("  schedule rr <quantum>         - Round Robin\n");
    printf("  schedule priority             - Priority Scheduling\n");
    printf("  schedule mlfq                 - Multilevel Feedback Queue\n");
    printf("  schedule compare              - Compare all algorithms\n\n");
    
    printf(COLOR_GREEN "Memory Management:\n" COLOR_RESET);
    printf("  mem_alloc <name> <size> <ff|bf|wf>  - Allocate memory\n");
    printf("  mem_free <name>                     - Free memory\n");
    printf("  mem_status                          - Show memory map\n");
    printf("  mem_reset                           - Reset memory\n\n");
    
    printf(COLOR_GREEN "Synchronization:\n" COLOR_RESET);
    printf("  producer_consumer             - Run Producer-Consumer simulation\n");
    printf("  bankers                       - Run Banker's Algorithm\n\n");
    
    printf(COLOR_GREEN "Page Replacement:\n" COLOR_RESET);
    printf("  page <fifo|lru|optimal|compare> <frames> <ref_string>\n");
    printf("  Example: page lru 3 7,0,1,2,0,3,0,4,2,3\n\n");
    
    printf(COLOR_GREEN "Disk Scheduling:\n" COLOR_RESET);
    printf("  disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n");
    printf("  Example: disk sstf 53 98,183,37,122,14,124,65,67\n\n");
    
    printf(COLOR_GREEN "Other:\n" COLOR_RESET);
    printf("  help                          - Show this help\n");
    printf("  clear                         - Clear screen\n");
    printf("  exit                          - Exit MiniOS\n\n");
}

// parse a comma-separated list of integers into an array
// returns the count of numbers parsed
int parse_int_list(const char *str, int *arr, int max_count) {
    int count = 0;
    char buffer[256];
    strncpy(buffer, str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    char *token = strtok(buffer, ",");
    while (token != NULL && count < max_count) {
        arr[count++] = atoi(token);
        token = strtok(NULL, ",");
    }
    return count;
}

// handle the 'schedule' command
void handle_schedule(char *args) {
    if (process_count == 0) {
        printf(COLOR_RED "Error: No processes created. Use 'create' first.\n" COLOR_RESET);
        return;
    }
    
    if (args == NULL) {
        printf(COLOR_RED "Usage: schedule <fcfs|sjf|rr|priority|mlfq|compare>\n" COLOR_RESET);
        return;
    }
    
    if (strcmp(args, "fcfs") == 0) {
        schedule_fcfs();
    } else if (strcmp(args, "sjf") == 0) {
        schedule_sjf();
    } else if (strncmp(args, "rr", 2) == 0) {
        int quantum = 2;  // default quantum
        char *q = strtok(args + 2, " ");
        if (q != NULL) quantum = atoi(q);
        if (quantum <= 0) quantum = 2;
        schedule_rr(quantum);
    } else if (strcmp(args, "priority") == 0) {
        schedule_priority();
    } else if (strcmp(args, "mlfq") == 0) {
        schedule_mlfq();
    } else if (strcmp(args, "compare") == 0) {
        schedule_compare();
    } else {
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, args);
    }
}

// handle the 'page' command
void handle_page(char *args) {
    if (args == NULL) {
        printf(COLOR_RED "Usage: page <fifo|lru|optimal|compare> <frames> <ref_string>\n" COLOR_RESET);
        return;
    }
    
    char algo[20];
    int frames;
    char ref_str[256];
    
    if (sscanf(args, "%s %d %s", algo, &frames, ref_str) != 3) {
        printf(COLOR_RED "Usage: page <fifo|lru|optimal|compare> <frames> <ref_string>\n" COLOR_RESET);
        return;
    }
    
    if (frames <= 0 || frames > MAX_FRAMES) {
        printf(COLOR_RED "Error: frames must be between 1 and %d\n" COLOR_RESET, MAX_FRAMES);
        return;
    }
    
    int ref[MAX_REF_STRING];
    int ref_len = parse_int_list(ref_str, ref, MAX_REF_STRING);
    
    if (ref_len == 0) {
        printf(COLOR_RED "Error: empty reference string\n" COLOR_RESET);
        return;
    }
    
    if (strcmp(algo, "fifo") == 0) {
        page_replace_fifo(frames, ref, ref_len);
    } else if (strcmp(algo, "lru") == 0) {
        page_replace_lru(frames, ref, ref_len);
    } else if (strcmp(algo, "optimal") == 0) {
        page_replace_optimal(frames, ref, ref_len);
    } else if (strcmp(algo, "compare") == 0) {
        page_replace_compare(frames, ref, ref_len);
    } else {
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, algo);
    }
}

// handle the 'disk' command
void handle_disk(char *args) {
    if (args == NULL) {
        printf(COLOR_RED "Usage: disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n" COLOR_RESET);
        return;
    }
    
    char algo[20];
    int head;
    char req_str[256];
    
    if (sscanf(args, "%s %d %s", algo, &head, req_str) != 3) {
        printf(COLOR_RED "Usage: disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n" COLOR_RESET);
        return;
    }
    
    int requests[MAX_REQUESTS];
    int n = parse_int_list(req_str, requests, MAX_REQUESTS);
    
    if (n == 0) {
        printf(COLOR_RED "Error: empty request list\n" COLOR_RESET);
        return;
    }
    
    if (strcmp(algo, "fcfs") == 0) {
        disk_fcfs(requests, n, head);
    } else if (strcmp(algo, "sstf") == 0) {
        disk_sstf(requests, n, head);
    } else if (strcmp(algo, "scan") == 0) {
        disk_scan(requests, n, head);
    } else if (strcmp(algo, "cscan") == 0) {
        disk_cscan(requests, n, head);
    } else if (strcmp(algo, "compare") == 0) {
        disk_compare(requests, n, head);
    } else {
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, algo);
    }
}

int main(void) {
    char input[512];
    
    // disable output buffering so pipes and UI communicate in real time
    setvbuf(stdout, NULL, _IONBF, 0);
    
    // initialize modules
    init_process_table();
    init_memory();
    
    clear_screen();
    print_banner();
    
    // main command loop
    while (1) {
        printf(COLOR_BLUE "minios> " COLOR_RESET);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;  // EOF
        }
        
        // remove trailing newline
        input[strcspn(input, "\n")] = '\0';
        
        // skip empty input
        if (strlen(input) == 0) continue;
        
        // parse command
        char *cmd = strtok(input, " ");
        char *args = cmd + strlen(cmd) + 1;
        // check if there are actually args after the command
        if (args >= input + strlen(input) + 1) args = NULL;
        // recalculate args properly
        {
            // strtok already modified input, so args points right after cmd
            if (cmd + strlen(cmd) < input + 511) {
                args = cmd + strlen(cmd) + 1;
                // trim leading spaces
                while (args < input + 512 && *args == ' ') args++;
                if (*args == '\0') args = NULL;
            } else {
                args = NULL;
            }
        }
        
        // --- handle commands ---
        
        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            printf("Shutting down MiniOS...\n");
            cleanup_memory();
            break;
        }
        else if (strcmp(cmd, "help") == 0) {
            print_help();
        }
        else if (strcmp(cmd, "clear") == 0) {
            clear_screen();
            print_banner();
        }
        else if (strcmp(cmd, "create") == 0) {
            if (args == NULL) {
                printf(COLOR_RED "Usage: create <name> <burst_time> <priority> [arrival_time]\n" COLOR_RESET);
                continue;
            }
            char name[MAX_NAME_LEN];
            int burst, prio, arrival = 0;
            int parsed = sscanf(args, "%s %d %d %d", name, &burst, &prio, &arrival);
            if (parsed < 3) {
                printf(COLOR_RED "Usage: create <name> <burst_time> <priority> [arrival_time]\n" COLOR_RESET);
                continue;
            }
            int pid = create_process(name, burst, prio, arrival);
            if (pid >= 0) {
                printf(COLOR_GREEN "Process '%s' created with PID %d\n" COLOR_RESET, name, pid);
            }
        }
        else if (strcmp(cmd, "list") == 0) {
            list_processes();
        }
        else if (strcmp(cmd, "kill") == 0) {
            if (args == NULL) {
                printf(COLOR_RED "Usage: kill <pid>\n" COLOR_RESET);
                continue;
            }
            int pid = atoi(args);
            if (kill_process(pid) == 0) {
                printf(COLOR_GREEN "Process %d terminated.\n" COLOR_RESET, pid);
            }
        }
        else if (strcmp(cmd, "clear_all") == 0) {
            init_process_table();
            printf(COLOR_GREEN "All processes cleared.\n" COLOR_RESET);
        }
        else if (strcmp(cmd, "schedule") == 0) {
            handle_schedule(args);
        }
        else if (strcmp(cmd, "mem_alloc") == 0) {
            if (args == NULL) {
                printf(COLOR_RED "Usage: mem_alloc <name> <size> <ff|bf|wf>\n" COLOR_RESET);
                continue;
            }
            char name[MAX_NAME_LEN];
            int size;
            char policy_str[4];
            if (sscanf(args, "%s %d %s", name, &size, policy_str) != 3) {
                printf(COLOR_RED "Usage: mem_alloc <name> <size> <ff|bf|wf>\n" COLOR_RESET);
                continue;
            }
            AllocPolicy pol;
            if (strcmp(policy_str, "ff") == 0) pol = FIRST_FIT;
            else if (strcmp(policy_str, "bf") == 0) pol = BEST_FIT;
            else if (strcmp(policy_str, "wf") == 0) pol = WORST_FIT;
            else {
                printf(COLOR_RED "Policy must be ff (First Fit), bf (Best Fit), or wf (Worst Fit)\n" COLOR_RESET);
                continue;
            }
            if (mem_allocate(name, size, pol) == 0) {
                printf(COLOR_GREEN "Allocated %d MB to '%s'\n" COLOR_RESET, size, name);
            }
        }
        else if (strcmp(cmd, "mem_free") == 0) {
            if (args == NULL) {
                printf(COLOR_RED "Usage: mem_free <name>\n" COLOR_RESET);
                continue;
            }
            char name[MAX_NAME_LEN];
            sscanf(args, "%s", name);
            if (mem_free(name) == 0) {
                printf(COLOR_GREEN "Freed memory for '%s'\n" COLOR_RESET, name);
            }
        }
        else if (strcmp(cmd, "mem_status") == 0) {
            mem_status();
        }
        else if (strcmp(cmd, "mem_reset") == 0) {
            cleanup_memory();
            init_memory();
            printf(COLOR_GREEN "Memory reset.\n" COLOR_RESET);
        }
        else if (strcmp(cmd, "producer_consumer") == 0) {
            run_producer_consumer();
        }
        else if (strcmp(cmd, "bankers") == 0) {
            run_bankers();
        }
        else if (strcmp(cmd, "page") == 0) {
            handle_page(args);
        }
        else if (strcmp(cmd, "disk") == 0) {
            handle_disk(args);
        }
        else {
            printf(COLOR_RED "Unknown command: '%s'. Type 'help' for list of commands.\n" COLOR_RESET, cmd);
        }
    }
    
    return 0;
}
