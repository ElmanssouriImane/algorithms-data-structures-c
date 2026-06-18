#include "priority_queue.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#define INDEX_OFFSET 1

#define PQ_INIT_DATA_SIZE 32

/**
 * @brief Priority queue based on a ternary heap with position tracking.
 *
 * Stores opaque pointers and maintains a hash table mapping each element
 * to its index in the heap, enabling efficient membership tests and
 * removal of arbitrary elements.
 */
typedef struct PriorityQueue {
    void **data;
    HashTable *pos_table;
    intptr_t capacity;
    intptr_t size;
    int (*compare)(const void *, const void *);
} PriorityQueue;

/**
 * @brief Swaps two pointer values.
 * 
 * @param a First void pointer.
 * @param b Second void pointer.
 */
static void swap(void **a, void **b) {
    void *tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * @brief Calculates the parent index in a ternary heap.
 * 
 * @param i The index of the current node.
 * 
 * @return The index of the parent node.
 */
static intptr_t parent(intptr_t i) {
    return (i - 1) / 3;
}

/**
 * @brief This function moves an element up the ternary heap until the heap
 * property is satisfied. It compares the element with its parent and swaps
 * them if necessary, updating the position table accordingly.
 * 
 * @param pq Pointer to the priority queue.
 * @param i Index of the element to heapify upward.
 */
static void heapify_up(PriorityQueue *pq, intptr_t i) {
    intptr_t par = parent(i);

    while(i > 0 && pq->compare(pq->data[par], pq->data[i]) < 0) {
        hash_table_put(pq->pos_table, pq->data[i], (void *)(par + INDEX_OFFSET));
        hash_table_put(pq->pos_table, pq->data[par], (void *)(i + INDEX_OFFSET));

        swap(&pq->data[i], &pq->data[par]);

        i = par;
        par = parent(i);
    }
}

/**
 * @brief This function moves an element down the ternary heap by comparing
 * it with its three children and swapping with the largest child if necessary.
 * The process continues until the heap property is satisfied. The position
 * table is updated after each swap.
 * 
 * @param pq Pointer to the priority queue.
 * @param i Index of the element to heapify downward.
 */
static void heapify_down(PriorityQueue *pq, intptr_t i) {
    intptr_t largest, child1, child2, child3;

    for(;;) {
        largest = i;
        child1 = 3 * i + 1;
        child2 = 3 * i + 2;
        child3 = 3 * i + 3;

        if(child1 < pq->size && pq->compare(pq->data[child1], pq->data[largest]) > 0) {
            largest = child1;
        }
        if(child2 < pq->size && pq->compare(pq->data[child2], pq->data[largest]) > 0) {
            largest = child2;
        }
        if(child3 < pq->size && pq->compare(pq->data[child3], pq->data[largest]) > 0) {
            largest = child3;
        }

        if(largest == i) {
            break;
        }

        hash_table_put(pq->pos_table, pq->data[i], (void *)(largest + INDEX_OFFSET));
        hash_table_put(pq->pos_table, pq->data[largest], (void *)(i + INDEX_OFFSET));

        swap(&pq->data[i], &pq->data[largest]);

        i = largest;
    }
}

PriorityQueue *priority_queue_create(int (*f1)(const void *,const void *), unsigned long (*f2)(const void *)) {
    if(!f1 || !f2) {
        return NULL;
    }

    PriorityQueue *pq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    if(!pq) {
        return NULL;
    }

    pq->data = (void **)malloc(sizeof(void *) * PQ_INIT_DATA_SIZE);
    if(!pq->data) {
        free(pq);
        return NULL;
    }

    pq->pos_table = hash_table_create(f1, f2);
    if(!pq->pos_table) {
        free(pq->data);
        free(pq);
        return NULL;
    }

    pq->capacity = PQ_INIT_DATA_SIZE;
    pq->size = 0;
    pq->compare = f1;

    return pq;
}

int priority_queue_push(PriorityQueue *pq, void *data) {
    if(!pq || !data) {
        return 0;
    }

    if(hash_table_contains_key(pq->pos_table, data)) {
        return 0;
    }

    if(pq->size == pq->capacity) {
        intptr_t new_capacity = pq->capacity * 2;

        if(new_capacity < pq->capacity) {
            return 0;
        }

        void **new_data = (void **)realloc(pq->data, (size_t)new_capacity * sizeof(void *));
        if(!new_data) {
            return 0;
        }

        pq->capacity = new_capacity;
        pq->data = new_data;
    }

    pq->data[pq->size] = data;
    hash_table_put(pq->pos_table, data, (void *)(pq->size + INDEX_OFFSET));
    pq->size++;

    heapify_up(pq, pq->size - 1);

    return 1;
}

int priority_queue_contains(const PriorityQueue *pq, const void *data) {
    if(!pq || !data) {
        return 0; 
    }

    return hash_table_contains_key(pq->pos_table, data);
}

void *priority_queue_top(const PriorityQueue *pq) {
    if(!pq || pq->size == 0) {
        return NULL;
    }

    return *pq->data;
}

void priority_queue_pop(PriorityQueue *pq) {
    if(!pq || pq->size == 0) {
        return;
    }

    hash_table_remove(pq->pos_table, pq->data[0]);

    if(pq->size == 1) {
        pq->size = 0;
        return;
    }

    swap(&pq->data[0], &pq->data[pq->size - 1]);
    hash_table_put(pq->pos_table, pq->data[0], (void *)(0 + INDEX_OFFSET));
    pq->size--;

    heapify_down(pq, 0);
}

int priority_queue_remove(PriorityQueue *pq, const void *data) {
    if(!pq || !data || pq->size == 0) {
        return 0; 
    }

    if(!hash_table_contains_key(pq->pos_table, data)) {
        return 0;
    }

    intptr_t removed_queue_index = (intptr_t)hash_table_get(pq->pos_table, data) - INDEX_OFFSET;
    hash_table_remove(pq->pos_table, pq->data[removed_queue_index]);

    if(removed_queue_index == pq->size - 1) {
        pq->size--;
        return 1;
    }

    swap(&pq->data[removed_queue_index], &pq->data[pq->size - 1]);
    hash_table_put(pq->pos_table, pq->data[removed_queue_index], (void *)(removed_queue_index + INDEX_OFFSET));
    pq->size--;

    intptr_t par = parent(removed_queue_index);
    if(removed_queue_index > 0 && pq->compare(pq->data[removed_queue_index], pq->data[par]) > 0) {
        heapify_up(pq, removed_queue_index);
    } else {
        heapify_down(pq, removed_queue_index);
    }

    return 1;
}

int priority_queue_size(const PriorityQueue *pq) {
    if(!pq) {
        return 0;
    }

    return (int)pq->size;
}

void priority_queue_free(PriorityQueue *pq) {
    if(!pq) {
        return;
    }

    free(pq->data);
    hash_table_free(pq->pos_table);
    free(pq);
}
