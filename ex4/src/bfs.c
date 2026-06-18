#include "bfs.h"

#include <stdlib.h>

#include "hash_table.h"

void **breadth_first_visit(Graph gr, void *start, int (*compare)(const void *, const void *), unsigned long (*hash)(const void *)) {
    if(!gr || !start || !compare || !hash) {
        return NULL;
    }

    if(!graph_contains_node(gr, start)) {
        return NULL;
    }

    int num_nodes = graph_num_nodes(gr);
    if(num_nodes <= 0 || graph_num_edges(gr) <= 0) {
        return NULL;
    }

    void **queue = (void **)malloc((size_t)num_nodes * sizeof(void *));
    int queue_head = 0, queue_tail = 0;
    if(!queue) {
        return NULL;
    }

    void **visited = (void **)malloc((size_t)(num_nodes + 1) * sizeof(void *));
    int visited_size = 0;
    if(!visited) {
        free(queue);
        return NULL;
    }

    HashTable *visited_table = hash_table_create(compare, hash);
    if(!visited_table) {
        free(queue);
        free(visited);
        return NULL;
    }

    queue[queue_tail++] = start;
    hash_table_put(visited_table, start, NULL);
    while(queue_tail != queue_head) {
        void *current = queue[queue_head++];
        visited[visited_size++] = current;

        void **neighbors = graph_get_neighbors(gr, current);
        if(!neighbors) {
            free(queue);
            free(visited);
            return NULL;
        }

        for(int k = 0; neighbors[k] != NULL; k++) {
            if(!hash_table_contains_key(visited_table, neighbors[k])) {
                queue[queue_tail++] = neighbors[k];
                hash_table_put(visited_table, neighbors[k], NULL);
            }
        }
        free(neighbors);
    }
    hash_table_free(visited_table);
    free(queue);

    visited[visited_size] = NULL;

    return visited;
}
