#ifndef __GRAPH_H__
#define __GRAPH_H__

/**
 * @brief Graph structure holding nodes and edges.
 */
typedef struct graph *Graph;

/**
 * @brief Edge structure connecting two nodes.
 */
typedef struct edge {
    void *source;
    void *dest;
    void *label;
} Edge;

/**
 * @brief Creates and initializes a new Graph instance.
 *
 * @param labelled 1 if edges should carry labels, 0 otherwise.
 * @param directed 1 if graph is directed, 0 otherwise.
 * @param compare Function pointer to compare nodes.
 * @param hash Function pointer to compute node hashes.
 * 
 * @return Pointer to the newly created Graph, or NULL on failure.
 */
Graph graph_create(int labelled, int directed, int (*compare)(const void *, const void *), unsigned long (*hash)(const void *));

/**
 * @brief Frees all memory associated with a Graph.
 *
 * @param gr Graph to free.
 */
void graph_free(Graph gr);

/**
 * @brief Checks if a graph is directed.
 * 
 * @param gr Graph instance.
 * 
 * @return 1 if directed, 0 otherwise.
 */
int graph_is_directed(const Graph gr);

/**
 * @brief Checks if a graph is labelled.
 * 
 * @param gr Graph instance.
 * 
 * @return 1 if labelled, 0 otherwise.
 */
int graph_is_labelled(const Graph gr);

/**
 * @brief Adds a new node to the graph.
 *
 * @param gr Graph instance.
 * @param node Pointer to node data.
 * 
 * @return 1 on success, 0 on error or if node is already present.
 */
int graph_add_node(Graph gr, const void *node);

/**
 * @brief Checks if a node exists in the graph.
 * 
 * @param gr Graph instance.
 * @param node Node to check.
 * 
 * @return 1 if node exists, 0 otherwise.
 */
int graph_contains_node(const Graph gr, const void *node);

/**
 * @brief Removes a node and all its edges from the graph.
 * 
 * @param gr Graph instance.
 * @param node Node to remove.
 * 
 * @return 1 on success, 0 on error.
 */
int graph_remove_node(Graph gr, const void *node);

/**
 * @brief Returns the number of nodes in the graph.
 * 
 * @param gr Graph instance.
 * 
 * @return Count of nodes.
 */
int graph_num_nodes(const Graph gr);

/**
 * @brief Retrieves all nodes in the graph.
 * 
 * @param gr Graph instance.
 * 
 * @return Array of node pointers (NULL-terminated), NULL on error.
 */
void **graph_get_nodes(const Graph gr);

/**
 * @brief Adds an edge between two nodes.
 *
 * @param gr Graph instance.
 * @param node1 Source node.
 * @param node2 Destination node.
 * @param label Optional label (NULL if not labelled).
 * 
 * @return 1 on success, 0 on error or if the edge already exist.
 */
int graph_add_edge(Graph gr, const void *node1, const void *node2, const void *label);

/**
 * @brief Checks if an edge exists between two nodes.
 * 
 * @param gr Graph instance.
 * @param node1 Source node.
 * @param node2 Destination node.
 * 
 * @return 1 if edge exists, 0 otherwise.
 */
int graph_contains_edge(const Graph gr, const void *node1, const void *node2);

/**
 * @brief Removes an edge between two nodes.
 * 
 * @param gr Graph instance.
 * @param node1 Source node.
 * @param node2 Destination node.
 * 
 * @return 1 on success, 0 on error.
 */
int graph_remove_edge(Graph gr, const void *node1, const void *node2);

/**
 * @brief Returns the number of edges in the graph.
 * 
 * @param gr Graph instance.
 * 
 * @return Count of edges.
 */
int graph_num_edges(const Graph gr);

/**
 * @brief Retrieves all edges in the graph.
 * 
 * @param gr Graph instance.
 * 
 * @return Array of Edge pointers (NULL-terminated), NULL on error.
 */
Edge **graph_get_edges(const Graph gr);

/**
 * @brief Gets the label of an edge between two nodes.
 * 
 * @param gr Graph instance.
 * @param node1 Source node.
 * @param node2 Destination node.
 * 
 * @return Pointer to label, NULL on error or if no label exists.
 */
void *graph_get_label(const Graph gr, const void *node1, const void *node2);

/**
 * @brief Retrieves all neighbors of a given node.
 *
 * @param gr Graph instance.
 * @param node Node whose neighbors are requested.
 * 
 * @return Array of neighbor node pointers (NULL-terminated), NULL on error.
 */
void **graph_get_neighbors(const Graph gr, const void *node);

/**
 * @brief Returns the number of neighbors of a node.
 * 
 * @param gr Graph instance.
 * @param node Node to check.
 * 
 * @return Count of neighbors.
 */
int graph_num_neighbors(const Graph gr, const void *node);

#endif //__GRAPH_H__
