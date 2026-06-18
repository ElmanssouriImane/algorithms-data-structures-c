#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "graph.h"
#include "bfs.h"

#define ARGC_COUNT 4

#define MAX_LINE_LEN 256

/**
 * @brief Comparison function for strings.
 * 
 * Used by the graph library to compare node identifiers (city names).
 * 
 * @param a First string pointer
 * @param b Second string pointer
 * 
 * @return strcmp result: 0 if equal, <0 if a<b, >0 if a>b
 */
int compare_string(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
} 

/**
 * @brief Hash function for strings (djb2 algorithm).
 * 
 * Computes hash value for string keys used in hash table implementation.
 * 
 * @param key String pointer
 * 
 * @return Unsigned long hash value
 */
unsigned long hash_string(const void *key) {
    const char *str = (const char *)key;
    unsigned long hash = 5381;
    int c;
    while((c = *str++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }
    return hash;
}

/**
 * @brief Converts string to float with validation.
 * 
 * Validates that entire string represents a valid float number.
 * 
 * @param s Input string
 * @param value Pointer to store parsed float
 * 
 * @return 1 if successful conversion, 0 if invalid
 */
int string_to_float(const char *s, float *value) {
    char *endptr;

    *value = strtof(s, &endptr);

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
 * @brief Convert a string to lowercase in place.
 *
 * The conversion is performed in place by modifying the original string.
 * Characters outside the ASCII uppercase range are left unchanged.
 *
 * @param s Pointer to a null-terminated string to be converted.
 */
void str_to_lower(char *s) {
    while(*s) {
        if(*s >= 'A' && *s <= 'Z') {
            *s += 'a' - 'A';
        }
        s++;
    }
}

/**
 * @brief Reads CSV file and builds graph from Italian cities distances.
 * 
 * Parses italian_dist_graph.csv format: place1,place2,distance\n
 * Adds nodes and undirected labelled edges (distance as float label).
 * 
 * @param csv_name Path to CSV file
 * @param gr Graph structure to populate
 * 
 * @return 1 on success, 0 on error (file not found, parse error)
 */
int read_csv(const char *csv_name, Graph gr) {
    FILE *csv;
    char line[MAX_LINE_LEN];

    if(!csv_name || !gr) {
        return 0;
    }

    csv = fopen(csv_name, "rt");
    if(!csv) {
        fprintf(stderr, "Error: Cannot open '%s'\n", csv_name);
        return 0; 
    }

    while(fgets(line, MAX_LINE_LEN, csv)) {
        char *token = strtok(line, ",");
        if(!token) {
            continue;
        }
        char *start = token;
        if(!graph_contains_node(gr, start)) {
            start = strdup(token);
            graph_add_node(gr, start);
        }

        token = strtok(NULL, ",");
        if(!token) {
            continue;
        }
        char *end = token;
        if(!graph_contains_node(gr, end)) {
            end = strdup(token);
            graph_add_node(gr, end);
        }

        token = strtok(NULL, ",");
        if(!token) {
            continue;
        }
        if(!graph_contains_edge(gr, start, end)) {
            float *dist_lbl = (float *)malloc(sizeof(float));
            if(!string_to_float(token, dist_lbl)) {
                free(dist_lbl);
                continue;
            }

            graph_add_edge(gr, strdup(start), strdup(end), dist_lbl);
        }
    }

    fclose(csv);

    return 1;
}

/**
 * @brief Frees all dynamically allocated memory associated with a graph.
 *
 * This function:
 * - Retrieves all edges using graph_get_edges() and frees, for each edge:
 *   - the label string,
 *   - the source and destination identifiers,
 *   - the edge structure itself;
 * - Frees the edges array;
 * - Frees the graph internal structures via graph_free();
 * - Retrieves all nodes using graph_get_nodes() and frees each node;
 * - Frees the nodes array.
 *
 * After this function returns, all memory associated with the graph
 * is completely deallocated.
 *
 * @param gr Graph to be deallocated.
 */
void free_graph_content(Graph gr) {
    void **nodes = graph_get_nodes(gr);
    Edge **edges = graph_get_edges(gr);

    for(int k = 0; edges[k] != NULL; k++) {
        free(edges[k]->label);
        free(edges[k]->source);
        free(edges[k]->dest);
        free(edges[k]);
    }
    free(edges);

    graph_free(gr);

    for(int k = 0; nodes[k] != NULL; k++) {
        free(nodes[k]);
    }

    free(nodes);
}

/** @brief Main program entry point.
 * 
 * Expected arguments: ./main_ex4 <csv_file> <start_city> <output_file>
 * Performs BFS traversal and saves visit order to output file.
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * 
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[]) {
    if(argc != ARGC_COUNT) {
        fprintf(stderr, "Usage: %s <csv_file> <start_city> <output_file>\n", argv[0]);
        return 1;
    }

    const char *csv_file = argv[1];
    char *start_city = argv[2];
    const char *result_file = argv[3];

    Graph gr = graph_create(1, 0, compare_string, hash_string);
    if(!gr) {
        fprintf(stderr, "Error: Could not create the graph!\n\n");
        return 1;
    }

    if(!read_csv(csv_file, gr)) {
        fprintf(stderr, "Error: Could not read the provided CSV file!\n\n");

        free_graph_content(gr);
        return 1;
    }

    str_to_lower(start_city);
    if(!graph_contains_node(gr, start_city)) {
        fprintf(stderr, "Error: City '%s' not found\n\n", start_city);

        free_graph_content(gr);
        return 1;
    }

    printf("Visiting (BFS) the cities graph!\n");

    clock_t tstart = clock();
    void **visited = breadth_first_visit(gr, start_city, compare_string, hash_string);
    clock_t tend = clock();

    if(!visited) {
        fprintf(stderr, "Error: Cannot visit the graph!\n\n");

        free_graph_content(gr);
        return 1;
    }

    double elapsed = (double)(tend - tstart) / CLOCKS_PER_SEC;

    printf("Breadth First Visit completed in %f\n\n", elapsed);

    FILE *out = fopen(result_file, "wt");
    if(!out) {
        fprintf(stderr, "Error: Could not open the '%s' file!\n\n", result_file);

        free(visited);
        free_graph_content(gr);
        return 1;
    }

    for(int k = 0; visited[k] != NULL; k++) {
        fprintf(out, "%s\n", (const char *)visited[k]);
    }
    fclose(out);
    free(visited);

    free_graph_content(gr);

    return 0;
}
