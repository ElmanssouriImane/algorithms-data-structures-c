#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "priority_queue.h"

#define ARGC_COUNT 3

#define MAX_LINE_LENGTH 256

/**
 * @brief Record struct.
 * - id:       Unique task identifier.
 * - start:    Time the task arrives in the system.
 * - length:   Duration of the task execution.
 * - priority: Priority of the task (higher = more urgent).
 */
typedef struct Task {
    int id;
    int start;
    int length;
    int priority;
} Task;

/**
 * @brief Integer hash function used to hash th Tasks id.
 * 
 * @param key The id to be hashed.
 * 
 * @return The id hash.
 */
unsigned long hash_id(const void *key) {
    int value = *(const int *)key;
    uint64_t h = (uint64_t)value;

    h = h ^ (h >> 23);
    h = h * 0x2127599bf4325c37UL;
    h = h ^ (h >> 47);

    return (unsigned long)h;
}

/**
 * @brief Task comparison function. Compatible with standard qsort() and suitable
 * for both hash table key comparison (equality check) and priority queue ordering.
 *
 * The comparison follows these rules:
 * - If a and b have the same task ID, the function returns 0.  
 *   This behavior enables correct equality checking when the function is used
 *   by a hash table.
 * - If the IDs differ, tasks are ordered primarily by descending priority.
 * - If priorities are equal, tasks are ordered by their start time.
 *
 * @param a Pointer to the first Task.
 * @param b Pointer to the second Task.
 *
 * @return A value < 0 if a should come before b,  
 *         a value > 0 if a should come after b,  
 *         0 if the two tasks have the same ID.
 */
int compare_task(const void *a, const void *b) {
    Task ta = *(const Task *)a;
    Task tb = *(const Task *)b;

    int compare_id = ta.id - tb.id;
    int compare_priority = ta.priority - tb.priority;
    int compare_start = ta.start - tb.start;

    if(compare_id == 0) {
        return 0;
    }

    if(compare_priority != 0) {
        return compare_priority;
    }

    return compare_start;
}

/**
 * @brief This function attempts to convert the input string into an integer.
 * The result is stored in the variable pointed to by value.
 *
 * @param s Pointer to the null-terminated string to convert.
 * @param value Pointer to an integer where the converted value will be stored.
 *
 * @return Returns 1 if the conversion is successful, 0 if the string is not a valid integer.
 */
int string_to_int(const char *s, int *value) {
    char *endptr;

    *value = (int)strtol(s, &endptr, 0);

    /**
     * If endptr == s then the string starts with a non-numeric character ("", "ABC", "A123");
     * If endptr != [\0, \\n] then the string contains non-numeric characters ("1A", "123ABC4")
    */
    if(endptr == s || (*endptr != '\0' && *endptr != '\n')) {
        return 0;
    }

    return 1;
}

/**
 * @brief Parses a CSV line containing task information in the format:
 * "ID,Start,Length,Priority" and allocates a new Task structure with the
 * parsed values.
 * 
 * @param line Null-terminated string containing the CSV line to parse.
 *             The string will be modified by strtok().
 * 
 * @return Pointer to a newly allocated Task structure on success,
 *         NULL if line is NULL, memory allocation fails, or parsing fails.
 */
Task *tokenize_csv(char *line) {
    if(!line) {
        return NULL;
    }

    Task *task = (Task *)malloc(sizeof(Task));
    if(!task) {
        return NULL;
    }

    char *token = strtok(line, ",");
    if(!token || !string_to_int(token, &(task->id))) {
        free(task);
        return NULL;
    }

    token = strtok(NULL, ",");
    if(!token || !string_to_int(token, &(task->start))) {
        free(task);
        return NULL;
    }

    token = strtok(NULL, ",");
    if(!token || !string_to_int(token, &(task->length))) {
        free(task);
        return NULL;
    }

    token = strtok(NULL, ",");
    if(!token || !string_to_int(token, &(task->priority))) {
        free(task);
        return NULL;
    }

    return task;
}


/**
 * @brief Simulate non-preemptive priority-based task scheduling.
 * 
 * Reads tasks from a CSV file and simulates their execution using a priority queue.
 * The simulator follows these rules:
 * 1. Tasks are executed in priority order (higher priority = executed first)
 * 2. Execution is non-preemptive (tasks run to completion once started)
 * 3. Only tasks that have arrived (Start <= current_time) can be executed
 * 4. If no tasks are ready, time advances by 1 unit
 * 5. Results are written to the output file in execution order
 * 
 * @param csv Input file stream containing task data in CSV format:
 *            "ID,Start,Length,Priority" (one task per line).
 *            The file must be already opened for reading.
 * @param out Output file stream where execution results will be written in CSV format:
 *            "ID,StartTime,EndTime" (one executed task per line).
 *            The file must be already opened for writing.
 * @param pq Priority queue used to manage task scheduling. Must be properly
 *           initialized with appropriate compare and hash functions for Task objects.
 */
void execute_tasks(FILE *csv, FILE *out, PriorityQueue *pq) {
    char line[MAX_LINE_LENGTH];
    int sym_time = 0;
    int end_exec = 0;
    Task *next_task = NULL;

    while(!end_exec || priority_queue_size(pq) > 0) {
        while(!end_exec) {
            if(!next_task) {
                if(fgets(line, MAX_LINE_LENGTH, csv)) {
                    next_task = tokenize_csv(line);
                    if(!next_task) {
                        continue;
                    }
                } else {
                    end_exec = 1;
                    continue;
                }
            }

            if(next_task->start <= sym_time) {
                priority_queue_push(pq, next_task);
                next_task = NULL;
            } else {
                break;
            }
        }

        if(priority_queue_size(pq) == 0) {
            sym_time++;
            continue;
        }

        Task *exec_task = priority_queue_top(pq);
        priority_queue_pop(pq);

        int start_time = sym_time;
        sym_time += exec_task->length;
        int end_time = sym_time;

        fprintf(out, "%d,%d,%d\n", exec_task->id, start_time, end_time);
        free(exec_task);
    }
}

/**
 * @brief This function prints the main_ex3 synopsis.
 * 
 * @param program_name The name of the executable (usually argv[0])
 */
void print_synopsis(const char *program_name) {
    fprintf(
        stderr,
        "Usage: %s <input_file> <output_file>\n\n",
        program_name
    );
}

/**
 * @brief Program entry point used to perform
 * the scheduling algorithm on given task list.
 *
 * Usage:
 *   ./bin/main_ex3 <input_file> <output_filer>
 *
 * - input_file:  Path to input file containing the task list.
 * - output_file: Path to output file where to save the execution task list.
 */
int main(int argc, char *argv[]) {
    if(argc != ARGC_COUNT) {
        print_synopsis(argv[0]);
        return 1;
    }

    const char *in_file_name = argv[1];
    const char *out_file_name = argv[2];

    PriorityQueue *pq = priority_queue_create(compare_task, hash_id);
    if(!pq) {
        fprintf(stderr, "Error: Cannot create the priority queue!\n\n");
        return 1;
    }

    FILE *in_file  = fopen(in_file_name,  "rt");
    if(!in_file) {
        fprintf(stderr, "Error: Cannot open '%s' input file!\n\n", in_file_name);
        priority_queue_free(pq);
        return 1;
    }

    FILE *out_file = fopen(out_file_name, "wt");
    if(!out_file) {
        fprintf(stderr, "Error: Cannot open '%s' output file!\n\n", out_file_name);
        fclose(in_file);
        priority_queue_free(pq);
        return 1;
    }

    clock_t tstart = clock();
    execute_tasks(in_file, out_file, pq);
    clock_t tend = clock();

    double elapsed = (double)(tend - tstart) / CLOCKS_PER_SEC;
    printf("Scheduling executed in %f seconds.\n\n", elapsed);

    fclose(out_file);
    fclose(in_file);

    priority_queue_free(pq);

    return 0;
}
