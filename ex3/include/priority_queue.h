#ifndef __PRIORITY_QUEUE__
#define __PRIORITY_QUEUE__

#include "hash_table.h"

/**
 * @brief Priority queue with opaque structure.
 * 
 * Implementation details are hidden. Use the library's public functions
 * to interact with this structure.
 */
typedef struct PriorityQueue PriorityQueue;

/**
 * @brief Allocate and initialize an empty priority queue.
 *
 * @param f1 Total ordering comparison function (qsort-style: <0, 0, >0).
 * @param f2 Hash function for elements, used by the internal position table.
 * 
 * @return Pointer to a newly created PriorityQueue, or NULL on allocation failure.
 */
PriorityQueue *priority_queue_create(int (*f1)(const void *, const void *), unsigned long (*f2)(const void *));

/**
 * @brief Insert an element into the priority queue.
 *
 * @param pq   Priority queue instance.
 * @param data Element to insert.
 * 
 * @return 1 on success, 0 on allocation or internal error.
 * 
 * @warning The element pointer is stored as-is; ownership remains with the caller.
 */
int priority_queue_push(PriorityQueue *pq, void *data);

/**
 * @brief Check whether an element is present in the priority queue.
 *
 * @param pq   Priority queue instance.
 * @param data Element to search for.
 * 
 * @return 1 if the element is present, 0 otherwise.
 */
int priority_queue_contains(const PriorityQueue *pq, const void *data);

/**
 * @brief Retrieve, without removing it, the highest-priority element.
 *
 * @param pq Priority queue instance.
 * 
 * @return Pointer to the top element, or NULL if the queue is empty.
 */
void *priority_queue_top(const PriorityQueue *pq);

/**
 * @brief Remove the highest-priority element from the priority queue.
 *
 * @param pq Priority queue instance.
 */
void priority_queue_pop(PriorityQueue *pq);

/**
 * @brief Remove a specific element from the priority queue, if present.
 *
 * Uses the internal position table to locate the element in logarithmic time.
 *
 * @param pq   Priority queue instance.
 * @param data Element to remove.
 * 
 * @return 1 if the element was found and removed, 0 if it was not present.
 */
int priority_queue_remove(PriorityQueue *pq, const void *data);

/**
 * @brief Get the number of elements stored in the priority queue.
 *
 * @param pq Priority queue instance.
 * 
 * @return Current number of elements.
 */
int priority_queue_size(const PriorityQueue *pq);

/**
 * @brief Destroy a priority queue and release its internal resources.
 *
 * @param pq Priority queue instance to destroy (may be NULL).
 * 
 * @warning The pointed elements are not freed; the caller is responsible for
 * managing their lifetime.
 */
void priority_queue_free(PriorityQueue *pq);

#endif // __PRIORITY_QUEUE__
