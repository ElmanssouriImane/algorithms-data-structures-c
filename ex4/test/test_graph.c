#include "graph.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define TEST(name) printf("\n=== Test: %s ===\n", name)

#define ASSERT_TRUE(expr, msg) {assert((expr) == 1); printf("PASSED: %s\n", msg);}
#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)
#define ASSERT_EQUALS(expected, actual, msg) ASSERT_TRUE(((expected) == (actual)), msg)
#define ASSERT_DOUBLE_EQUALS(expected, actual, msg) ASSERT_TRUE((-0.0001 < (expected) - (actual) && (expected) - (actual) < 0.0001), msg)
#define ASSERT_NOT_NULL(ptr, msg) ASSERT_TRUE(((ptr) != NULL), msg)
#define ASSERT_NULL(ptr, msg) ASSERT_TRUE(((ptr) == NULL), msg)

/**
 * @brief Tests utility functions
 */
int compare_int(const void *a, const void *b) {
    int ia = *(int *)a;
    int ib = *(int *)b;
    return ia - ib;
}

unsigned long hash_int(const void *key) {
    return (unsigned long)(*(int *)key);
}

static int double_compare(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if(-0.0001 < da - db && da - db < 0.0001) return 0;
    if(da < db) return -1;
    return 1;
}

static unsigned long double_hash(const void *key) {
    double d = *(const double *)key;
    unsigned long hash;
    memcpy(&hash, &d, sizeof(unsigned long));
    return hash;
}

int compare_string(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

unsigned long hash_string(const void *key) {
    const char *str = (const char *)key;
    unsigned long hash = 5381;
    int c;
    while((c = *str++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }
    return hash;
}

typedef struct {
    int id;
    char name[50];
    double value;
} Entity;

int entity_compare(const void *a, const void *b) {
    const Entity *ea = (const Entity *)a;
    const Entity *eb = (const Entity *)b;
    return ea->id - eb->id;
}

unsigned long entity_hash(const void *key) {
    const Entity *e = (const Entity *)key;
    return (unsigned long)(e->id);
}

void test_graph_creation(void) {
    TEST("Graph Creation");

    Graph g1 = graph_create(0, 0, compare_int, hash_int);
    ASSERT_NOT_NULL(g1, "Non-directed and non-labeled graph created!");
    ASSERT_FALSE(graph_is_directed(g1), "Graph not directed!");
    ASSERT_FALSE(graph_is_labelled(g1), "Graph not labeled!");
    graph_free(g1);

    Graph g2 = graph_create(1, 1, compare_int, hash_int);
    ASSERT_NOT_NULL(g2, "Directed and labeled graph created!");
    ASSERT_TRUE(graph_is_directed(g2), "Graph directed!");
    ASSERT_TRUE(graph_is_labelled(g2), "Graph labeled!");
    graph_free(g2);

    // Test invalid parameters
    Graph g3 = graph_create(0, 0, NULL, hash_int);
    ASSERT_NULL(g3, "Graph creation failed with compare = NULL!");

    Graph g4 = graph_create(0, 0, compare_int, NULL);
    ASSERT_NULL(g4, "Graph creation failed with hash = NULL!");

    Graph g5 = graph_create(0, 0, NULL, NULL);
    ASSERT_NULL(g5, "Graph creation failed with compare = NULL and hash = NULL!");
}

void test_add_nodes(void) {
    TEST("Add Nodes");

    Graph g = graph_create(0, 0, compare_int, hash_int);
    int nodes[] = {1, 2, 3, 4, 5};

    ASSERT_EQUALS(0, graph_num_nodes(g), "Empty graph!");

    ASSERT_TRUE(graph_add_node(g, &nodes[0]), "Node 1 added!");
    ASSERT_EQUALS(1, graph_num_nodes(g), "Graph has one node!");
    ASSERT_TRUE(graph_contains_node(g, &nodes[0]), "Node 1 present!");

    ASSERT_TRUE(graph_add_node(g, &nodes[1]), "Node 2 added!");
    ASSERT_TRUE(graph_add_node(g, &nodes[2]), "Node 3 added!");
    ASSERT_EQUALS(3, graph_num_nodes(g), "Graph has 3 nodes!");

    // Test duplicates
    ASSERT_FALSE(graph_add_node(g, &nodes[0]), "Duplicate node not added!");
    ASSERT_EQUALS(3, graph_num_nodes(g), "Number of nodes unchanged!");

    // Test invalid parameters
    ASSERT_FALSE(graph_add_node(NULL, &nodes[3]), "Adding node failed with graph = NULL!");
    ASSERT_FALSE(graph_add_node(g, NULL), "Adding node failed with node = NULL!");

    graph_free(g);
}

void test_remove_nodes(void) {
    TEST("Remove Nodes");

    Graph g = graph_create(0, 0, compare_int, hash_int);
    int nodes[] = {1, 2, 3};

    graph_add_node(g, &nodes[0]);
    graph_add_node(g, &nodes[1]);
    graph_add_node(g, &nodes[2]);

    ASSERT_TRUE(graph_remove_node(g, &nodes[1]), "Node 2 removed!");
    ASSERT_EQUALS(2, graph_num_nodes(g), "Graph has 2 nodes!");
    ASSERT_FALSE(graph_contains_node(g, &nodes[1]), "Node 2 not present anymore!");

    // Test removing inexistent node
    ASSERT_FALSE(graph_remove_node(g, &nodes[1]), "Removing inexistent node failed!");

    // Test invalid parameters
    ASSERT_FALSE(graph_remove_node(NULL, &nodes[0]), "Removing node failed with graph = NULL!");
    ASSERT_FALSE(graph_remove_node(g, NULL), "Removing node failed with node = NULL!");

    graph_free(g);
}

void test_add_edges_not_directed(void) {
    TEST("Add Edges - Not Directed Graph");

    Graph g = graph_create(0, 0, compare_int, hash_int);
    int nodes[] = {1, 2, 3};

    graph_add_node(g, &nodes[0]);
    graph_add_node(g, &nodes[1]);
    graph_add_node(g, &nodes[2]);

    ASSERT_TRUE(graph_add_edge(g, &nodes[0], &nodes[1], NULL), "Edge 1-2 added!");
    ASSERT_TRUE(graph_contains_edge(g, &nodes[0], &nodes[1]), "Edge 1->2 present!");
    ASSERT_TRUE(graph_contains_edge(g, &nodes[1], &nodes[0]), "Edge 1<-2 present!");
    ASSERT_EQUALS(1, graph_num_edges(g), "1 edge!");

    ASSERT_TRUE(graph_add_edge(g, &nodes[1], &nodes[2], NULL), "Edge 2-3 added!");
    ASSERT_EQUALS(2, graph_num_edges(g), "2 edges!");

    // Test edge already present
    ASSERT_FALSE(graph_add_edge(g, &nodes[1], &nodes[2], NULL), "Edge 2->3 already present!");
    ASSERT_FALSE(graph_add_edge(g, &nodes[1], &nodes[2], NULL), "Edge 2<-3 already present!")


    // Test inexistent nodes
    int node_new = 99;
    ASSERT_FALSE(graph_add_edge(g, &nodes[0], &node_new, NULL), "Adding edge to inexistent node failed!");

    // Test invalid parameters
    ASSERT_FALSE(graph_add_edge(NULL, &nodes[0], &nodes[1], NULL), "Adding edge failed with graph = NULL!");
    ASSERT_FALSE(graph_add_edge(g, NULL, &nodes[1], NULL), "Adding edge failed with node1 = NULL!");
    ASSERT_FALSE(graph_add_edge(g, &nodes[0], NULL, NULL), "Adding edge failed with node2 = NULL!");

    graph_free(g);
}

void test_add_edges_directed(void) {
    TEST("Add Edges - Directed Graph");

    Graph g = graph_create(0, 1, compare_int, hash_int);
    int nodes[] = {1, 2, 3};

    graph_add_node(g, &nodes[0]);
    graph_add_node(g, &nodes[1]);
    graph_add_node(g, &nodes[2]);

    ASSERT_TRUE(graph_add_edge(g, &nodes[0], &nodes[1], NULL), "Edge 1->2 added!");
    ASSERT_TRUE(graph_contains_edge(g, &nodes[0], &nodes[1]), "Edge 1->2 present!");
    ASSERT_FALSE(graph_contains_edge(g, &nodes[1], &nodes[0]), "Edge 1<-2 not present!");
    ASSERT_EQUALS(1, graph_num_edges(g), "1 edge!");

    ASSERT_TRUE(graph_add_edge(g, &nodes[1], &nodes[0], NULL), "Edge 1<-2 added!");
    ASSERT_EQUALS(2, graph_num_edges(g), "2 edges!");

    graph_free(g);
}

void test_self_loops_not_directed(void) {
    TEST("Self-Loops - Not Directed Graph");

    Graph g = graph_create(0, 0, compare_int, hash_int);
    int nodes[] = {1, 2, 3, 4};

    graph_add_node(g, &nodes[0]);
    graph_add_node(g, &nodes[1]);
    graph_add_node(g, &nodes[2]);
    graph_add_node(g, &nodes[3]);

    ASSERT_TRUE(graph_add_edge(g, &nodes[0], &nodes[0], NULL), "Self-loop 1->1 added!");
    ASSERT_TRUE(graph_contains_edge(g, &nodes[0], &nodes[0]), "Self-loop 1->1 present!");
    ASSERT_EQUALS(1, graph_num_edges(g), "1 edge!");
    ASSERT_EQUALS(1, graph_num_neighbors(g, &nodes[0]), "Node 1 has one neighbour!");

    ASSERT_TRUE(graph_add_edge(g, &nodes[1], &nodes[1], NULL), "Self-loop 2->2 added!");
    ASSERT_EQUALS(2, graph_num_edges(g), "2 total self-loops!");

    ASSERT_TRUE(graph_add_edge(g, &nodes[0], &nodes[1], NULL), "Normal edge 1-2 added!");
    ASSERT_EQUALS(3, graph_num_edges(g), "3 edges (2 self-loops + 1 normal)!");

    // Checks neighbors of node 1
    void **neighbors = graph_get_neighbors(g, &nodes[0]);
    int count = 0;
    int has_self = 0;
    for(int i = 0; neighbors[i] != NULL; i++) {
        count++;
        if(compare_int(neighbors[i], &nodes[0]) == 0) {
            has_self = 1;
        }
    }
    ASSERT_EQUALS(2, count, "Nodo 1 has 2 neighbors (including itself)!");
    ASSERT_TRUE(has_self, "Nodo 1 has itself as neighbour!");
    free(neighbors);

    // Test removing self-loop
    ASSERT_TRUE(graph_remove_edge(g, &nodes[0], &nodes[0]), "Self-loop 1->1 removed!");
    ASSERT_FALSE(graph_contains_edge(g, &nodes[0], &nodes[0]), "Self-loop not present anymore!");
    ASSERT_EQUALS(2, graph_num_edges(g), "2 Edges after removal (1 normal + 1 self-loop)!");

    // Test removing normal edge
    ASSERT_TRUE(graph_remove_edge(g, &nodes[0], &nodes[1]), "Normal edge 1-2 removed!");
    ASSERT_EQUALS(1, graph_num_edges(g), "1 Edge (self-loop)!");

    // Test remove inexistent self-loop
    ASSERT_FALSE(graph_remove_edge(g, &nodes[0], &nodes[0]), "Removing inexistent self-loop failed!");

    // Test add duplicate self-loop
    ASSERT_FALSE(graph_add_edge(g, &nodes[1], &nodes[1], NULL), "Self-loop 2->2 already present!");
    ASSERT_EQUALS(1, graph_num_edges(g), "1 Edge (self-loop)!");

    graph_free(g);
}

void test_self_loops_directed(void) {
    TEST("Self-Loops - Directed Graph");

    Graph g = graph_create(0, 1, compare_int, hash_int);
    int nodes[] = {1, 2, 3, 4};

    graph_add_node(g, &nodes[0]);
    graph_add_node(g, &nodes[1]);
    graph_add_node(g, &nodes[2]);
    graph_add_node(g, &nodes[3]);

    ASSERT_TRUE(graph_add_edge(g, &nodes[0], &nodes[0], NULL), "Self-loop 1->1 added!");
    ASSERT_TRUE(graph_contains_edge(g, &nodes[0], &nodes[0]), "Self-loop 1->1 present!");
    ASSERT_EQUALS(1, graph_num_edges(g), "1 edge!");
    ASSERT_EQUALS(1, graph_num_neighbors(g, &nodes[0]), "Node 1 has one neighbour!");

    ASSERT_TRUE(graph_add_edge(g, &nodes[1], &nodes[1], NULL), "Self-loop 2->2 added!");
    ASSERT_EQUALS(2, graph_num_edges(g), "2 total self-loops!");

    ASSERT_TRUE(graph_add_edge(g, &nodes[0], &nodes[1], NULL), "Normal edge 1->2 added!");
    ASSERT_EQUALS(3, graph_num_edges(g), "3 edges (2 self-loops + 1 normal)!");

    ASSERT_TRUE(graph_add_edge(g, &nodes[1], &nodes[0], NULL), "Normal edge 1<-2 added!");
    ASSERT_EQUALS(4, graph_num_edges(g), "4 edges (2 self-loops + 2 normal)!");

    // Checks neighbors of node 1
    void **neighbors = graph_get_neighbors(g, &nodes[0]);
    int count = 0;
    int has_self = 0;
    for(int i = 0; neighbors[i] != NULL; i++) {
        count++;
        if(compare_int(neighbors[i], &nodes[0]) == 0) {
            has_self = 1;
        }
    }
    ASSERT_EQUALS(2, count, "Nodo 1 has 2 neighbors (including itself)!");
    ASSERT_TRUE(has_self, "Nodo 1 has itself as neighbour!");
    free(neighbors);

    // Test removing self-loop
    ASSERT_TRUE(graph_remove_edge(g, &nodes[0], &nodes[0]), "Self-loop 1->1 removed!");
    ASSERT_FALSE(graph_contains_edge(g, &nodes[0], &nodes[0]), "Self-loop not present anymore!");
    ASSERT_EQUALS(3, graph_num_edges(g), "3 Edges after removal (2 normal + 1 self-loop)!");

    // Test removing normal edge
    ASSERT_TRUE(graph_remove_edge(g, &nodes[0], &nodes[1]), "Normal edge 1->2 removed!");
    ASSERT_EQUALS(2, graph_num_edges(g), "2 Edge (1 normal + 1 self-loop)!");

    graph_free(g);
}

void test_edge_labels(void) {
    TEST("Edge Labels");

    Graph g1 = graph_create(1, 0, compare_string, hash_string);
    char *cities[] = {"Rome", "Milan", "Naples"};
    int distances[] = {500, 600, 700};

    graph_add_node(g1, cities[0]);
    graph_add_node(g1, cities[1]);
    graph_add_node(g1, cities[2]);

    ASSERT_TRUE(graph_add_edge(g1, cities[0], cities[1], &distances[0]), "Edge Rome-Milan with label 500 added!");

    void *label = graph_get_label(g1, cities[0], cities[1]);
    ASSERT_NOT_NULL(label, "Label retrieved (Rome-Milan)!");
    ASSERT_EQUALS(500, *(int *)label, "Correct label value = 500!");

    // Test adding edge with NULL label
    ASSERT_FALSE(graph_add_edge(g1, cities[1], cities[2], NULL), "Adding edge with label = NULL failed!");
    
    // Test get_label on unlabeled graph
    Graph g2 = graph_create(0, 0, compare_int, hash_int);
    int nodes[] = {1, 2};

    graph_add_node(g2, &nodes[0]);
    graph_add_node(g2, &nodes[1]);
    graph_add_edge(g2, &nodes[0], &nodes[1], NULL);

    ASSERT_NULL(graph_get_label(g2, &nodes[0], &nodes[1]), "Getting label on unlabeled graph returns NULL!");

    // Test invalid parameters
    ASSERT_NULL(graph_get_label(NULL, cities[0], cities[1]), "Getting label with graph = NULL failed!");
    ASSERT_NULL(graph_get_label(g1, NULL, cities[1]), "Getting label with node1 = NULL failed!");
    ASSERT_NULL(graph_get_label(g1, cities[0], NULL), "Getting label with node2 = NULL failed!");

    graph_free(g1);
    graph_free(g2);
}

void test_remove_edges(void) {
    TEST("Remove Edges");

    Graph g = graph_create(0, 1, compare_int, hash_int);
    int nodes[] = {1, 2, 3};

    graph_add_node(g, &nodes[0]);
    graph_add_node(g, &nodes[1]);
    graph_add_node(g, &nodes[2]);
    graph_add_edge(g, &nodes[0], &nodes[1], NULL);
    graph_add_edge(g, &nodes[1], &nodes[2], NULL);

    ASSERT_EQUALS(2, graph_num_edges(g), "2 initial edges!");

    ASSERT_TRUE(graph_remove_edge(g, &nodes[0], &nodes[1]), "Edge 1->2 removed!");
    ASSERT_FALSE(graph_contains_edge(g, &nodes[0], &nodes[1]), "Edge 1-> not present anymore!");
    ASSERT_EQUALS(1, graph_num_edges(g), "1 remaining edge!");

    // Test removing inexistent edge
    ASSERT_FALSE(graph_remove_edge(g, &nodes[0], &nodes[1]), "Removing inexistent edge failed!");

    // Test parametri invalidi
    ASSERT_FALSE(graph_remove_edge(NULL, &nodes[1], &nodes[2]), "Removing edge with graph = NULL failed!");
    ASSERT_FALSE(graph_remove_edge(g, NULL, &nodes[2]), "Removing edge with node1 = NULL failed!");
    ASSERT_FALSE(graph_remove_edge(g, &nodes[1], NULL), "Removing edge with node2 = NULL failed!");

    graph_free(g);
}

void test_remove_node_with_edges(void) {
    TEST("Remove Node with Edges");

    Graph g = graph_create(0, 0, compare_int, hash_int);
    int nodes[] = {1, 2, 3, 4};

    for(int i = 0; i < 4; i++) {
        graph_add_node(g, &nodes[i]);
    }

    graph_add_edge(g, &nodes[0], &nodes[1], NULL);
    graph_add_edge(g, &nodes[1], &nodes[2], NULL);
    graph_add_edge(g, &nodes[1], &nodes[3], NULL);

    int initial_edges = graph_num_edges(g);
    ASSERT_EQUALS(3, initial_edges, "3 initial edges!");

    ASSERT_TRUE(graph_remove_node(g, &nodes[1]), "Node 2 removed!");
    ASSERT_EQUALS(3, graph_num_nodes(g), "3 nodes remaining!");

    // Every edge linked to node 2 must have been removed
    ASSERT_FALSE(graph_contains_edge(g, &nodes[0], &nodes[1]), "Edge 1-2 removed!");
    ASSERT_FALSE(graph_contains_edge(g, &nodes[1], &nodes[2]), "Edge 2-3 removed!");
    ASSERT_FALSE(graph_contains_edge(g, &nodes[1], &nodes[3]), "Edge 2-4 removed!");

    graph_free(g);
}

void test_neighbors(void) {
    TEST("Neighbors");

    Graph g = graph_create(0, 1, compare_int, hash_int);
    int nodes[] = {1, 2, 3, 4};

    for(int i = 0; i < 4; i++) {
        graph_add_node(g, &nodes[i]);
    }

    graph_add_edge(g, &nodes[0], &nodes[1], NULL);
    graph_add_edge(g, &nodes[0], &nodes[2], NULL);
    graph_add_edge(g, &nodes[0], &nodes[3], NULL);

    ASSERT_EQUALS(3, graph_num_neighbors(g, &nodes[0]), "Node 1 has 3 neighbors!");
    ASSERT_EQUALS(0, graph_num_neighbors(g, &nodes[1]), "Node 2 has 0 neighbors (directed graph)!");

    void **neighbors = graph_get_neighbors(g, &nodes[0]);
    ASSERT_NOT_NULL(neighbors, "Neighbors list retrieved!");

    int count = 0;
    for(int i = 0; neighbors[i] != NULL; i++) {
        count++;
    }
    ASSERT_EQUALS(3, count, "3 neighbors in the list!");
    free(neighbors);

    // Test invalid parameters
    ASSERT_EQUALS(0, graph_num_neighbors(NULL, &nodes[0]), "Getting neighbors number failed (0) with graph = NULL!");
    ASSERT_EQUALS(0, graph_num_neighbors(g, NULL), "Getting neighbors number failed (0) with node = NULL!");
    ASSERT_NULL(graph_get_neighbors(NULL, &nodes[0]), "Getting neighbors list failed with graph = NULL!");
    ASSERT_NULL(graph_get_neighbors(g, NULL), "Getting neighbors list failed with node = NULL!");

    graph_free(g);
}

void test_get_nodes_and_edges(void) {
    TEST("Get Nodes and Edges");

    Graph g1 = graph_create(0, 0, compare_int, hash_int);
    int nodes[] = {1, 2, 3};

    for(int i = 0; i < 3; i++) {
        graph_add_node(g1, &nodes[i]);
    }

    graph_add_edge(g1, &nodes[0], &nodes[1], NULL);
    graph_add_edge(g1, &nodes[1], &nodes[2], NULL);
    graph_add_edge(g1, &nodes[1], &nodes[1], NULL);

    void **node_list = graph_get_nodes(g1);
    ASSERT_NOT_NULL(node_list, "Nodes list retrieved!");

    int node_count = 0;
    for(int i = 0; node_list[i] != NULL; i++) {
        node_count++;
    }
    ASSERT_EQUALS(3, node_count, "3 nodes in the list!");
    free(node_list);

    Edge **edge_list = graph_get_edges(g1);
    ASSERT_NOT_NULL(edge_list, "Edges list retrieved!");

    int edge_count = 0;
    for(int i = 0; edge_list[i] != NULL; i++) {
        edge_count++;
        free(edge_list[i]);
    }
    ASSERT_EQUALS(3, edge_count, "3 edges in the list!");
    free(edge_list);

    // Test on empty graph
    Graph g2 = graph_create(0, 0, compare_int, hash_int);
    ASSERT_EQUALS(0, graph_num_nodes(g2), "0 nodes on empty graph!");
    ASSERT_EQUALS(0, graph_num_edges(g2), "0 edges on empty graph!");

    graph_free(g1);
    graph_free(g2);
}

void test_null_parameters(void) {
    TEST("NULL Parameters");

    ASSERT_FALSE(graph_is_directed(NULL), "is_directed with graph = NULL returns 0!");
    ASSERT_FALSE(graph_is_labelled(NULL), "is_labelled with graph = NULL returns 0!");
    ASSERT_EQUALS(0, graph_num_nodes(NULL), "num_nodes with graph = NULL returns 0!");
    ASSERT_EQUALS(0, graph_num_edges(NULL), "num_edges with graph = NULL returns 0!");
    ASSERT_NULL(graph_get_nodes(NULL), "get_nodes with graph = NULL returns NULL!");
    ASSERT_NULL(graph_get_edges(NULL), "get_nodes with graph = NULL returns NULL!");
    ASSERT_FALSE(graph_contains_node(NULL, &(int){1}), "contains_node with graph = NULL returns 0!");
    ASSERT_FALSE(graph_contains_edge(NULL, &(int){1}, &(int){2}), "contains_edge with graph = NULL returns 0!");

    // Test graph_free with NULL and already freed graph;
    graph_free(NULL);
    ASSERT_TRUE(1, "graph_free(NULL) executed successfully!");
}

void test_complex_graph(void) {
    TEST("Complex Graph Operations - Italian Cities Network");

    Graph g = graph_create(1, 0, compare_string, hash_string);
    char *cities[] = {"Rome", "Milan", "Naples", "Turin", "Florence", "Venice", "Bologna", "Genoa", "Palermo", "Bari"};
    int distances[] = {500, 600, 200, 300, 250, 400, 150, 180, 950, 430, 520, 270, 350, 480, 380, 560};

    // Creates the map graph
    for (int i = 0; i < 10; i++) {
        graph_add_node(g, cities[i]);
    }

    graph_add_edge(g, cities[0], cities[1], &distances[0]);  // Rome-Milan: 500
    graph_add_edge(g, cities[0], cities[2], &distances[1]);  // Rome-Naples: 600
    graph_add_edge(g, cities[0], cities[4], &distances[4]);  // Rome-Florence: 250
    graph_add_edge(g, cities[1], cities[3], &distances[2]);  // Milan-Turin: 200
    graph_add_edge(g, cities[1], cities[4], &distances[3]);  // Milan-Florence: 300
    graph_add_edge(g, cities[1], cities[5], &distances[5]);  // Milan-Venice: 400
    graph_add_edge(g, cities[1], cities[7], &distances[7]);  // Milan-Genoa: 180
    graph_add_edge(g, cities[2], cities[4], &distances[4]);  // Naples-Florence: 250
    graph_add_edge(g, cities[2], cities[8], &distances[8]);  // Naples-Palermo: 950
    graph_add_edge(g, cities[2], cities[9], &distances[9]);  // Naples-Bari: 430
    graph_add_edge(g, cities[3], cities[7], &distances[10]); // Turin-Genoa: 520
    graph_add_edge(g, cities[4], cities[6], &distances[6]);  // Florence-Bologna: 150
    graph_add_edge(g, cities[5], cities[6], &distances[11]); // Venice-Bologna: 270
    graph_add_edge(g, cities[6], cities[7], &distances[12]); // Bologna-Genoa: 350

    ASSERT_EQUALS(10, graph_num_nodes(g), "10 cities in the graph!");
    ASSERT_EQUALS(14, graph_num_edges(g), "14 edges in the graph!");

    // Checks cities connections
    ASSERT_EQUALS(5, graph_num_neighbors(g, cities[1]), "Milan has 5 connections!");
    ASSERT_EQUALS(3, graph_num_neighbors(g, cities[0]), "Rome has 3 connections!");
    ASSERT_EQUALS(4, graph_num_neighbors(g, cities[2]), "Naples has 4 connections!");
    ASSERT_EQUALS(4, graph_num_neighbors(g, cities[4]), "Florence has 4 connections!");
    ASSERT_EQUALS(1, graph_num_neighbors(g, cities[8]), "Palermo has 1 connection!");
    ASSERT_EQUALS(1, graph_num_neighbors(g, cities[9]), "Bari has 1 connection!");

    // Checks some connections label
    void *dist_rm = graph_get_label(g, cities[0], cities[1]);
    ASSERT_EQUALS(500, *(int *)dist_rm, "Rome-Milan: 500km!");

    void *dist_np = graph_get_label(g, cities[2], cities[8]);
    ASSERT_EQUALS(950, *(int *)dist_np, "Naples-Palermo: 950km!");

    void *dist_fb = graph_get_label(g, cities[4], cities[6]);
    ASSERT_EQUALS(150, *(int *)dist_fb, "Florence-Bologna: 150km!");

    // Checks bidirectionalicy
    void *dist1 = graph_get_label(g, cities[0], cities[1]);
    void *dist2 = graph_get_label(g, cities[1], cities[0]);
    ASSERT_TRUE(dist1 == dist2, "Labels points to the same data!");

    // Test removing Milan city
    int edges_before = graph_num_edges(g);
    ASSERT_TRUE(graph_remove_node(g, cities[1]), "Removed Milan!");
    ASSERT_EQUALS(9, graph_num_nodes(g), "9 cities remaining!");

    // Checks number of edges (after Milan removal)
    int edges_after = graph_num_edges(g);
    ASSERT_EQUALS(edges_before - 5, edges_after, "5 edges removed by removing Milan!");

    // Checks other cities connection (after Milan removal)
    ASSERT_FALSE(graph_contains_edge(g, cities[0], cities[1]), "Rome-Milan removed!");
    ASSERT_FALSE(graph_contains_edge(g, cities[3], cities[1]), "Turin-Milan removed!");
    ASSERT_FALSE(graph_contains_edge(g, cities[5], cities[1]), "Venice-Milan removed!");
    ASSERT_FALSE(graph_contains_edge(g, cities[4], cities[1]), "Florence-Milan removed!");
    ASSERT_FALSE(graph_contains_edge(g, cities[8], cities[1]), "Genoa-Milan removed!");

    // Checks Rome connection (after Milan removal)
    ASSERT_EQUALS(2, graph_num_neighbors(g, cities[0]), "Rome now has 2 connection!");

    // Adding new city
    char *new_city = "Cagliari";
    int new_dist1 = 670, new_dist2 = 400;

    ASSERT_TRUE(graph_add_node(g, new_city), "Cagliari added!");
    ASSERT_TRUE(graph_add_edge(g, new_city, cities[0], &new_dist1), "Cagliari-Rome: 670km!");
    ASSERT_TRUE(graph_add_edge(g, new_city, cities[8], &new_dist2), "Cagliari-Palermo: 400km!");

    ASSERT_EQUALS(10, graph_num_nodes(g), "10 cities after adding Cagliari!");
    ASSERT_EQUALS(2, graph_num_neighbors(g, new_city), "Cagliari has 2 connections!");

    // Test get_edges
    Edge **edges = graph_get_edges(g);
    ASSERT_NOT_NULL(edges, "Edges list retrieved!");

    int edge_count = 0;
    int total_distance = 0;
    for (int i = 0; edges[i] != NULL; i++) {
        edge_count++;
        total_distance += *(int *)(edges[i]->label);
        free(edges[i]);
    }

    ASSERT_EQUALS(11, edge_count, "11 edges in the list!");
    ASSERT_TRUE(total_distance > 0, "Total distance greater than 0!");

    free(edges);

    // Test get_neighbors for Rome
    void **roma_neighbors = graph_get_neighbors(g, cities[0]);
    ASSERT_NOT_NULL(roma_neighbors, "Rome neighbour cities retrieved!");

    int neighbor_count = 0;
    for (int i = 0; roma_neighbors[i] != NULL; i++) {
        neighbor_count++;
        ASSERT_TRUE(graph_contains_node(g, roma_neighbors[i]), "Rome neighbour verified!");
    }
    ASSERT_EQUALS(3, neighbor_count, "Roma has 3 neighbour cities in the list!");
    free(roma_neighbors);

    // Test updating label (remove and add again)
    int new_distance = 550;
    ASSERT_TRUE(graph_remove_edge(g, cities[0], cities[4]), "Removed Rome-Florence connection!");
    ASSERT_TRUE(graph_add_edge(g, cities[0], cities[4], &new_distance), "Added Rome-Florence connection with new distance!");

    void *updated_dist = graph_get_label(g, cities[0], cities[4]);
    ASSERT_EQUALS(550, *(int *)updated_dist, "Rome-Florence distance updated to 550km!");

    graph_free(g);
}

void test_int_graph(void) {
    TEST("Graph with int nodes");

    Graph g = graph_create(0, 0, compare_int, hash_int);
    ASSERT_NOT_NULL(g, "Graph created!");

    int nodes[] = {-10, -5, 0, 5, 10, 15, -3, 8};
    int n = 8;

    for (int i = 0; i < n; i++) {
        ASSERT_TRUE(graph_add_node(g, &nodes[i]), "Node added!");
    }

    ASSERT_EQUALS(n, graph_num_nodes(g), "All nodes added!");

    ASSERT_TRUE(graph_contains_node(g, &nodes[0]), "Contains negative node -10!");
    ASSERT_TRUE(graph_contains_node(g, &nodes[2]), "Contains zero node!");
    ASSERT_TRUE(graph_contains_node(g, &nodes[4]), "Contains positive node 10!");

    ASSERT_TRUE(graph_add_edge(g, &nodes[0], &nodes[1], NULL), "Edge -10 to -5 added!");
    ASSERT_TRUE(graph_add_edge(g, &nodes[2], &nodes[4], NULL), "Edge 0 to 10 added!");
    ASSERT_TRUE(graph_add_edge(g, &nodes[1], &nodes[3], NULL), "Edge -5 to 5 added!");
    ASSERT_TRUE(graph_add_edge(g, &nodes[4], &nodes[5], NULL), "Edge 10 to 15 added!");

    ASSERT_EQUALS(4, graph_num_edges(g), "4 edges added!");

    ASSERT_EQUALS(2, graph_num_neighbors(g, &nodes[1]), "Node -5 has 2 neighbors!");

    void **neighbors = graph_get_neighbors(g, &nodes[1]);
    ASSERT_NOT_NULL(neighbors, "Neighbors retrieved!");

    int neighbor_count = 0;
    for (int i = 0; neighbors[i] != NULL; i++) {
        neighbor_count++;
    }
    ASSERT_EQUALS(2, neighbor_count, "2 neighbors in list!");
    free(neighbors);

    ASSERT_TRUE(graph_remove_edge(g, &nodes[0], &nodes[1]), "Edge removed!");
    ASSERT_FALSE(graph_contains_edge(g, &nodes[0], &nodes[1]), "Edge no longer exists!");
    ASSERT_EQUALS(3, graph_num_edges(g), "3 edges remaining!");

    ASSERT_TRUE(graph_remove_node(g, &nodes[4]), "Node 10 removed!");
    ASSERT_EQUALS(n - 1, graph_num_nodes(g), "Node count decreased!");
    ASSERT_FALSE(graph_contains_edge(g, &nodes[2], &nodes[4]), "Edge to removed node gone!");

    graph_free(g);
}

void test_int_labeled_graph(void) {
    TEST("Graph with int nodes and int labels");

    Graph g = graph_create(1, 0, compare_int, hash_int);

    int nodes[] = {10, 20, 30, 40};
    int weights[] = {100, 200, 300, 400, 500};

    for (int i = 0; i < 4; i++) {
        graph_add_node(g, &nodes[i]);
    }

    graph_add_edge(g, &nodes[0], &nodes[1], &weights[0]);
    graph_add_edge(g, &nodes[1], &nodes[2], &weights[1]);
    graph_add_edge(g, &nodes[2], &nodes[3], &weights[2]);

    void *label = graph_get_label(g, &nodes[0], &nodes[1]);
    ASSERT_NOT_NULL(label, "Label retrieved!");
    ASSERT_EQUALS(100, *(int *)label, "Label value correct!");

    label = graph_get_label(g, &nodes[1], &nodes[2]);
    ASSERT_EQUALS(200, *(int *)label, "Second label correct!");

    graph_free(g);
}

void test_double_graph(void) {
    TEST("Graph with double nodes");

    Graph g = graph_create(0, 0, double_compare, double_hash);
    ASSERT_NOT_NULL(g, "Graph created!");

    double nodes[] = {-3.14, -1.5, 0.0, 1.414, 2.718, 3.14159, -0.5};
    int n = 7;

    for (int i = 0; i < n; i++) {
        ASSERT_TRUE(graph_add_node(g, &nodes[i]), "Double node added!");
    }

    ASSERT_EQUALS(n, graph_num_nodes(g), "All double nodes added!");

    ASSERT_TRUE(graph_contains_node(g, &nodes[0]), "Contains negative -3.14!");
    ASSERT_TRUE(graph_contains_node(g, &nodes[2]), "Contains zero!");
    ASSERT_TRUE(graph_contains_node(g, &nodes[5]), "Contains high precision 3.14159!");

    graph_add_edge(g, &nodes[0], &nodes[1], NULL);
    graph_add_edge(g, &nodes[2], &nodes[3], NULL);
    graph_add_edge(g, &nodes[3], &nodes[4], NULL);
    graph_add_edge(g, &nodes[4], &nodes[5], NULL);

    ASSERT_EQUALS(4, graph_num_edges(g), "4 edges added!");

    ASSERT_EQUALS(2, graph_num_neighbors(g, &nodes[4]), "Node 2.718 has 2 neighbors!");

    ASSERT_TRUE(graph_remove_node(g, &nodes[3]), "Removed node 1.414!");
    ASSERT_EQUALS(2, graph_num_edges(g), "Edges reduced after node removal!");

    graph_free(g);
}

void test_double_labeled_graph(void) {
    TEST("Graph with double nodes and double labels");

    Graph g = graph_create(1, 0, double_compare, double_hash);

    double nodes[] = {1.1, 2.2, 3.3, 4.4};
    double weights[] = {10.5, 20.5, 30.5};

    for (int i = 0; i < 4; i++) {
        graph_add_node(g, &nodes[i]);
    }

    graph_add_edge(g, &nodes[0], &nodes[1], &weights[0]);
    graph_add_edge(g, &nodes[1], &nodes[2], &weights[1]);
    graph_add_edge(g, &nodes[2], &nodes[3], &weights[2]);

    void *label = graph_get_label(g, &nodes[0], &nodes[1]);
    ASSERT_NOT_NULL(label, "Label retrieved!");
    ASSERT_DOUBLE_EQUALS(10.5, *(double *)label, "Label value correct!");

    void *label2 = graph_get_label(g, &nodes[1], &nodes[0]);
    ASSERT_TRUE(label == label2, "Same label from both directions!");

    graph_free(g);
}

void test_string_graph(void) {
    TEST("Graph with string nodes");

    Graph g = graph_create(0, 0, compare_string, hash_string);

    char *cities[] = {"Rome", "Milan", "Naples", "Turin", "Florence"};
    int n = 5;

    for (int i = 0; i < n; i++) {
        ASSERT_TRUE(graph_add_node(g, cities[i]), "City added!");
    }

    ASSERT_EQUALS(n, graph_num_nodes(g), "All cities added!");

    graph_add_edge(g, cities[0], cities[1], NULL); // Rome-Milan
    graph_add_edge(g, cities[0], cities[2], NULL); // Rome-Naples
    graph_add_edge(g, cities[1], cities[3], NULL); // Milan-Turin
    graph_add_edge(g, cities[1], cities[4], NULL); // Milan-Florence
    graph_add_edge(g, cities[0], cities[4], NULL); // Rome-Florence

    ASSERT_EQUALS(5, graph_num_edges(g), "5 edges in network!");

    ASSERT_TRUE(graph_contains_edge(g, cities[0], cities[1]), "Rome-Milan exists!");
    ASSERT_TRUE(graph_contains_edge(g, cities[1], cities[0]), "Milan-Rome exists!");

    ASSERT_EQUALS(3, graph_num_neighbors(g, cities[0]), "Rome has 3 connections!");
    ASSERT_EQUALS(3, graph_num_neighbors(g, cities[1]), "Milan has 3 connections!");

    void **neighbors = graph_get_neighbors(g, cities[0]);
    int count = 0;
    for (int i = 0; neighbors[i] != NULL; i++) {
        count++;
        ASSERT_TRUE(graph_contains_node(g, neighbors[i]), "Neighbor is valid node!");
    }
    ASSERT_EQUALS(3, count, "3 neighbors found!");
    free(neighbors);

    graph_free(g);
}

void test_string_labeled_graph(void) {
    TEST("Graph with string nodes and int distance labels");

    Graph g = graph_create(1, 0, compare_string, hash_string);

    char *cities[] = {"Paris", "London", "Berlin", "Madrid"};
    int distances[] = {344, 878, 1054, 1052, 1871};

    for (int i = 0; i < 4; i++) {
        graph_add_node(g, cities[i]);
    }

    graph_add_edge(g, cities[0], cities[1], &distances[0]); // Paris-London: 344km
    graph_add_edge(g, cities[0], cities[2], &distances[1]); // Paris-Berlin: 878km
    graph_add_edge(g, cities[0], cities[3], &distances[2]); // Paris-Madrid: 1054km
    graph_add_edge(g, cities[1], cities[2], &distances[3]); // London-Berlin: 1052km

    void *label = graph_get_label(g, cities[0], cities[1]);
    ASSERT_NOT_NULL(label, "Label retrieved!");
    ASSERT_EQUALS(344, *(int *)label, "Paris-London distance correct!");

    label = graph_get_label(g, cities[0], cities[2]);
    ASSERT_EQUALS(878, *(int *)label, "Paris-Berlin distance correct!");

    label = graph_get_label(g, cities[2], cities[0]);
    ASSERT_EQUALS(878, *(int *)label, "Berlin-Paris distance same!");

    graph_free(g);
}

void test_struct_graph(void) {
    TEST("Graph with Entity struct nodes");

    Graph g = graph_create(0, 0, entity_compare, entity_hash);

    Entity entities[] = {
        {1, "Server-A", 85.5},
        {2, "Server-B", 92.3},
        {3, "Server-C", 78.9},
        {4, "Server-D", 95.1},
        {5, "Server-E", 88.7}
    };
    int n = 5;

    for (int i = 0; i < n; i++) {
        ASSERT_TRUE(graph_add_node(g, &entities[i]), "Entity added!");
    }

    ASSERT_EQUALS(n, graph_num_nodes(g), "All entities added!");

    graph_add_edge(g, &entities[0], &entities[1], NULL);
    graph_add_edge(g, &entities[0], &entities[2], NULL);
    graph_add_edge(g, &entities[1], &entities[3], NULL);
    graph_add_edge(g, &entities[2], &entities[4], NULL);
    
    ASSERT_EQUALS(4, graph_num_edges(g), "4 connections!");

    ASSERT_TRUE(graph_contains_node(g, &entities[0]), "Contains Server-A!");

    ASSERT_EQUALS(2, graph_num_neighbors(g, &entities[0]), "Server-A has 2 connections!");

    void **neighbors = graph_get_neighbors(g, &entities[0]);
    int count = 0;
    for (int i = 0; neighbors[i] != NULL; i++) {
        Entity *e = (Entity *)neighbors[i];
        ASSERT_TRUE(e->id >= 1 && e->id <= 5, "Valid neighbor entity!");
        count++;
    }
    ASSERT_EQUALS(2, count, "2 neighbors found!");
    free(neighbors);

    graph_free(g);
}

int main(void) {
    printf(
        "\n"
        "+----------------------------------------+\n"
        "|        GRAPH LIBRARY UNIT TESTS        |\n"
        "+----------------------------------------+\n"
    );

    test_graph_creation();
    test_add_nodes();
    test_remove_nodes();
    test_add_edges_not_directed();
    test_add_edges_directed();
    test_self_loops_not_directed();
    test_self_loops_directed();
    test_edge_labels();
    test_remove_edges();
    test_remove_node_with_edges();
    test_neighbors();
    test_get_nodes_and_edges();
    test_null_parameters();
    test_complex_graph();

    test_int_graph();
    test_int_labeled_graph();

    test_double_graph();
    test_double_labeled_graph();

    test_string_graph();
    test_string_labeled_graph();

    test_struct_graph();

    printf(
        "\n"
        "+----------------------------------------+\n"
        "|           ALL TESTS PASSED!            |\n"
        "+----------------------------------------+\n"
        "\n"
    );

    return 0;
}
