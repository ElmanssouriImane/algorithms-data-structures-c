#include "graph.h"
#include "hash_table.h"

#include <stdlib.h>
#include <string.h>

typedef struct graph {
    HashTable *nodes;
    int labelled;
    int directed;
    int (*compare)(const void *, const void *);
    unsigned long (*hash)(const void *);
} *Graph;

typedef struct ListNode {
    Edge ed;
    struct ListNode *next;
} ListNode;

typedef struct {
    ListNode *head;
    int size;
    int (*compare)(const void *, const void *);
} List;

static List *list_create(int (*compare)(const void *, const void *)) {
    if(!compare) {
        return NULL;
    }

    List *l = (List *)malloc(sizeof(List));
    if(!l) {
        return NULL;
    }

    l->head = NULL;
    l->size = 0;
    l->compare = compare;

    return l;
}

static int list_size(List *l) {
    if(!l) {
        return 0;
    }

    return l->size;
}

static int list_push(List *l, Edge *ed) {
    if(!l || !ed) {
        return 0;
    }

    ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
    if(!new_node) {
        return 0;
    }

    new_node->ed = *ed;
    new_node->next = l->head;

    l->head = new_node;
    l->size++;

    return 1;
}

static int list_contains(List *l, const void *node) {
    if(!l || !node) {
        return 0;
    }

    ListNode *curr = l->head;
    while(curr) {
        if(l->compare(curr->ed.dest, node) == 0) {
            return 1;
        }

        curr = curr->next;
    }

    return 0;
}

static void *list_get(List *l, const void *node) {
    if(!l || !node) {
        return NULL;
    }

    ListNode *curr = l->head;
    while(curr) {
        if(l->compare(curr->ed.dest, node) == 0) {
            return curr->ed.label;
        }

        curr = curr->next;
    }

    return NULL;
}

static int list_remove(List *l, const void *node) {
    if(!l || !node) {
        return 0;
    }

    ListNode *prev = NULL;
    ListNode *curr = l->head;
    while(curr) {
        if(l->compare(curr->ed.dest, node) == 0) {
            if(prev) {
                prev->next = curr->next;
            } else {
                l->head = curr->next;
            }

            free(curr);
            l->size--;
            return 1;
        }

        prev = curr;
        curr = curr->next;
    }

    return 0;
}

static void list_free(List **l) {
    if(!l || !*l) {
        return;
    }

    ListNode *tmp = (*l)->head;
    while(tmp) {
        (*l)->head = (*l)->head->next;
        free(tmp);
        tmp = (*l)->head;
    }

    free(*l);
    *l = NULL;
}

Graph graph_create(int labelled, int directed, int (*compare)(const void *, const void *), unsigned long (*hash)(const void *)) {
    struct graph *gr;

    if(!compare || !hash) {
        return NULL;
    }

    gr = (struct graph *)malloc(sizeof(struct graph));
    if(!gr) {
        return NULL;
    }

    gr->nodes = hash_table_create(compare, hash);
    if(!gr->nodes) {
        free(gr);
        return NULL;
    }

    gr->labelled = labelled;
    gr->directed = directed;
    gr->compare = compare;
    gr->hash = hash;

    return gr;
}

int graph_is_directed(const Graph gr) {
    if(!gr) {
        return 0;
    }

    return gr->directed;
}

int graph_is_labelled(const Graph gr) {
    if(!gr) {
        return 0;
    }

    return gr->labelled;
}

int graph_add_node(Graph gr, const void *node) {
    if(!gr || !node) {
        return 0;
    }

    if(hash_table_contains_key(gr->nodes, node)) {
        return 0;
    }

    List *adjacency = list_create(gr->compare);
    if(!adjacency) {
        return 0;
    }

    hash_table_put(gr->nodes, node, adjacency);

    return 1;
}

int graph_contains_node(const Graph gr, const void *node) {
    if(!gr || !node) {
        return 0;
    }

    return hash_table_contains_key(gr->nodes, node);
}

int graph_remove_node(Graph gr, const void *node) {
    if(!gr || !node) {
        return 0;
    }

    if(!hash_table_contains_key(gr->nodes, node)) {
        return 0;
    }

    void **nodes = hash_table_keyset(gr->nodes);
    if(!nodes) {
        return 0;
    }

    for(size_t k = 0; nodes[k] != NULL; k++) {
        void *other = nodes[k];
        if(gr->compare(other, node) == 0) {
            continue;
        }

        List *other_adjacency = (List *)hash_table_get(gr->nodes, other);
        list_remove(other_adjacency, node);
    }
    free(nodes);

    List *adjacency = (List *)hash_table_get(gr->nodes, node);
    list_free(&adjacency);

    hash_table_remove(gr->nodes, node);

    return 1;
}

int graph_num_nodes(const Graph gr) {
    if(!gr) {
        return 0;
    }

    return hash_table_size(gr->nodes);
}

void **graph_get_nodes(const Graph gr) {
    if(!gr) {
        return NULL;
    }

    return hash_table_keyset(gr->nodes);
}

int graph_add_edge(Graph gr, const void *node1, const void *node2, const void *label) {
    if(!gr || !node1 || !node2) {
        return 0;
    }

    if(gr->labelled && !label) {
        return 0;
    } else if(!gr->labelled && label) {
        return 0;
    }

    if(!hash_table_contains_key(gr->nodes, node1)) {
        return 0;
    }
    if(!hash_table_contains_key(gr->nodes, node2)) {
        return 0;
    }

    List *node1_adjacency = (List *)hash_table_get(gr->nodes, node1);
    if(list_contains(node1_adjacency, node2)) {
        return 0;
    }

    Edge ed1= {
        .source = (void *)node1,
        .dest = (void *)node2,
        .label = (void *)label
    };

    list_push(node1_adjacency, &ed1);

    if(!graph_is_directed(gr) && gr->compare(node1, node2) != 0) {
        Edge ed2 = {
            .source = (void *)node2,
            .dest = (void *)node1,
            .label = (void *)label
        };

        List *node2_adjacency = (List *)hash_table_get(gr->nodes, node2);
        list_push(node2_adjacency, &ed2);
    }

    return 1;
}

int graph_contains_edge(const Graph gr, const void *node1, const void *node2) {
    if(!gr || !node1 || !node2) {
        return 0;
    }

    if(!hash_table_contains_key(gr->nodes, node1)) {
        return 0;
    }
    if(!hash_table_contains_key(gr->nodes, node2)) {
        return 0;
    }

    List *node1_adjacency = (List *)hash_table_get(gr->nodes, node1);

    return list_contains(node1_adjacency, node2);
}

int graph_remove_edge(Graph gr, const void *node1, const void *node2) {
    if(!gr || !node1 || !node2) {
        return 0;
    }

    if(!hash_table_contains_key(gr->nodes, node1)) {
        return 0;
    }
    if(!hash_table_contains_key(gr->nodes, node2)) {
        return 0;
    }

    List *node1_adjacency = (List *)hash_table_get(gr->nodes, node1);
    if(!list_contains(node1_adjacency, node2)) {
        return 0;
    }
    list_remove(node1_adjacency, node2);

    if(!graph_is_directed(gr) && gr->compare(node1, node2) != 0) {
        List  *node2_adjacency = (List *)hash_table_get(gr->nodes, node2);
        list_remove(node2_adjacency, node1);
    }

    return 1;
}

int graph_num_edges(const Graph gr) {
    if(!gr) {
        return 0;
    }

    void **nodes = hash_table_keyset(gr->nodes);
    if(!nodes) {
        return 0;
    }

    int num = 0;
    int self_loops = 0;
    for(size_t k = 0; nodes[k] != NULL; k++) {
        List *adjacency = (List *)hash_table_get(gr->nodes, nodes[k]);
        num += list_size(adjacency);

        if(list_contains(adjacency, nodes[k])) {
            self_loops++;
        }
    }
    free(nodes);

    if(!graph_is_directed(gr)) {
        num = (num - self_loops) / 2 + self_loops;
    }

    return num;
}

Edge **graph_get_edges(const Graph gr) {
    if(!gr) {
        return NULL;
    }

    void **nodes = hash_table_keyset(gr->nodes);
    if(!nodes) {
        return NULL;
    }

    int num_nodes = hash_table_size(gr->nodes);
    if(num_nodes <= 0) {
        free(nodes);
        return NULL;
    }

    int total_edges = graph_num_edges(gr);
    if(total_edges <= 0) {
        free(nodes);
        return NULL;
    }

    Edge **edges = (Edge **)malloc((size_t)(total_edges + 1) * sizeof(Edge *));
    if(!edges) {
        free(nodes);
        return NULL;
    }

    int edge_index = 0;
    for(int k = 0; k < num_nodes; k++) {
        List *adjacency = (List *)hash_table_get(gr->nodes, nodes[k]);

        for(ListNode *curr = adjacency->head; curr; curr = curr->next) {
            if(graph_is_directed(gr) || gr->compare(nodes[k], curr->ed.dest) >= 0) {
                edges[edge_index] = (Edge *)malloc(sizeof(Edge));
                memcpy(edges[edge_index++], &curr->ed, sizeof(Edge));
            }
        }
    }
    free(nodes);

    edges[edge_index] = NULL;

    return edges;
}

void *graph_get_label(const Graph gr, const void *node1, const void *node2) {
    if(!gr || !node1 || !node2) {
        return NULL;
    }

    if(!graph_is_labelled(gr)) {
        return NULL;
    }

    if(!hash_table_contains_key(gr->nodes, node1)) {
        return NULL;
    }
    if(!hash_table_contains_key(gr->nodes, node2)) {
        return NULL;
    }

    List *node1_adjacency = (List *)hash_table_get(gr->nodes, node1);
    if(!list_contains(node1_adjacency, node2)) {
        return NULL;
    }

    return list_get(node1_adjacency, node2);
}

void **graph_get_neighbors(const Graph gr, const void *node) {
    if(!gr || !node) {
        return NULL;
    }

    if(!hash_table_contains_key(gr->nodes, node)) {
        return NULL;
    }

    List *adjacency = (List *)hash_table_get(gr->nodes, node);
    int adjacency_size = list_size(adjacency);

    void **neighbors = (void **)malloc(sizeof(void *) * (size_t)(adjacency_size + 1));
    if(!neighbors) {
        return NULL;
    }

    int k = 0;
    ListNode *curr = adjacency->head;
    while(k < adjacency_size) {
        neighbors[k] = curr->ed.dest;

        k++;
        curr = curr->next;
    }

    neighbors[k] = NULL;

    return neighbors;
}

int graph_num_neighbors(const Graph gr, const void *node) {
    if(!gr || !node) {
        return 0;
    }

    if(!hash_table_contains_key(gr->nodes, node)) {
        return 0;
    }

    List *adjacency = (List *)hash_table_get(gr->nodes, node);

    return list_size(adjacency);
}

void graph_free(Graph gr) {
    if(!gr) {
        return;
    }

    void **nodes = hash_table_keyset(gr->nodes);
    if(!nodes) {
        return;
    }

    for(size_t k = 0; nodes[k] != NULL; k++) {
        List *adjacency = (List *)hash_table_get(gr->nodes, nodes[k]);
        list_free(&adjacency);
    }
    free(nodes);
    hash_table_free(gr->nodes);

    free(gr);
}
