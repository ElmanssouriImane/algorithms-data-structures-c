#ifndef __BFS_H__
#define __BFS_H__

#include "graph.h"

/**
 * @brief Performs a breadth-first visit (BFV) on a graph starting from a given node.
 * 
 * This function traverses the graph level by level, visiting all neighbors of a node
 * before moving to the next level. The algorithm uses a queue to maintain the visit
 * order and a hash table for O(1) visited node lookup, achieving O(N + M) time complexity.
 * 
 * @param gr The graph to visit.
 * @param start The starting node for the BFS traversal.
 * @param compare Function pointer to compare nodes.
 * @param hash Function pointer to compute node hashes.
 * 
 * @return A dynamically allocated NULL-terminated array of node pointers in BFS order,
 *         or NULL on error. The caller is responsible for freeing the returned array.
 */
void **breadth_first_visit(Graph gr, void *start, int (*compare)(const void *, const void *), unsigned long (*hash)(const void *));

#endif // __BFS_H__
