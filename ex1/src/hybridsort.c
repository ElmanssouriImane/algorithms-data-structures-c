#include "hybridsort.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Swaps two memory blocks.
 * 
 * @param a    First memory block pointer.
 * @param b    Second memory block pointer.
 * @param size The size, in bytes, of each memory block.
 */
static void swap(void *a, void *b, size_t size) {
    void *temp = malloc(size); 

    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);

    free(temp);
}

/**
 * @brief Selection sort algorithm (used for small size arrays).
 * 
 * @param base    Pointer to the array to be sorted.
 * @param nitems  Number of elements in the array.
 * @param size    Size in bytes of each element in the array.
 * @param compare Comparison function used to determine ordering between
 *                elements. Must follow the standard qsort comparator contract.
 */
static void selection_sort(void *base, size_t nitems, size_t size, int (*compare)(const void *, const void *)) {
    if(nitems < 1 || size == 0 || !base || !compare) {
        return; 
    }

    for(size_t i = 0; i < nitems - 1; i++) { // ciclo per trovare il minimo 
        size_t min_idx = i; // parte dicendo che il minimo è qui 

        for(size_t j = i + 1; j < nitems; j++) { // scorre il resto dell'array cercando se c'è un elemento iù picccolo 
            void *elem_j = (uint8_t *)base + j * size; // elemento j 
            void *elem_min = (uint8_t *)base + min_idx * size; // elemento minimo attuale 

            if(compare(elem_j, elem_min) < 0) { // se trova un elemento più piccolo -> aggiorna min 
                min_idx = j; // aggirono il minimo 
            }
        }

        if(min_idx != i) {  // ilminimo è già al posto giusto? 
            void *elem_i = (uint8_t *)base + i * size; // posizione i 
            void *elem_min = (uint8_t *)base + min_idx * size; // posizione del minimo 
            swap(elem_i, elem_min, size); // scambio i due elementi ota il minimo è nella posszione gisuta 
        }
    }
}
// per ogni posizione i cerchi il minimo nel resto e lo scambi con i 
/**
 * @brief Selects a pivot using the median-of-three strategy for quick sort.
 *
 * It Chooses a pivot by taking the median value among the elements
 * at positions 'low', 'mid', and 'high'. It rearranges these three elements
 * so that: -the smallest is at 'low',
 *          -the median is at 'mid',
 *          -the largest is at 'high'.
 *
 * @param base    Pointer to the array being sorted.
 * @param low     Index of the lowest element in the current segment.
 * @param high    Index of the highest element in the current segment.
 * @param size    Size in bytes of each array element.
 * @param compare Comparison function used to determine ordering between
 *                elements. Must follow the standard qsort comparator contract.
 *
 * @return The index of the chosen pivot (always 'mid' after rearrangement).
 */
static int median_of_three(void *base, int low, int high, size_t size, int (*compare)(const void *, const void *)) {
    int mid = low + (high - low) / 2;
    void *el_low = (uint8_t *)base + (size_t)low * size;
    void *el_mid = (uint8_t *)base + (size_t)mid * size;
    void *el_high = (uint8_t *)base + (size_t)high * size;

    if(compare(el_low, el_mid) > 0) {
        swap(el_low, el_mid, size);
    }
    if(compare(el_low, el_high) > 0) {
        swap(el_low, el_high, size);
    }
    if(compare(el_mid, el_high) > 0) {
        swap(el_mid, el_high, size);
    }

    return mid;
}

/**
 * @brief Rearranges elements in the array segment such that elements
 * smaller than the pivot come before it, and elements greater come after.
 *
 * @param base    Pointer to the array being sorted.
 * @param low     Index of the lowest element in the current segment.
 * @param high    Index of the highest element in the current segment.
 * @param size    Size in bytes of each array element.
 * @param compare Comparison function used to determine ordering between
 *                elements. Must follow the standard qsort comparator contract.
 *
 * @return The final index position of the pivot element after partitioning.
 */
static int hoare_partition(void *base, int low, int high, size_t size, int (*compare)(const void *, const void *)) {
    int mid = median_of_three(base, low, high, size, compare);
    uint8_t *pivot_copy = malloc(size); memcpy(pivot_copy, (uint8_t *)base + (size_t)mid * size, size);
    int i = low - 1, j = high + 1;

    for(;;) {
        do {
            i++;
        } while(compare((uint8_t *)base + (size_t)i * size, pivot_copy) < 0);

        do {
            j--;
        } while(compare((uint8_t *)base + (size_t)j * size, pivot_copy) > 0);

        if(i >= j) {
            break;
        }

        swap((uint8_t *)base + (size_t)i * size, (uint8_t *)base + (size_t)j * size, size);
    }

    free(pivot_copy);
    return j;
}

/**
 * @brief Recursive helper function for QuickSort algorithm.
 *
 * @param base    Pointer to the array being sorted.
 * @param low     Index of the lowest element in the current segment.
 * @param high    Index of the highest element in the current segment.
 * @param size    Size in bytes of each array element.
 * @param compare Comparison function used to determine ordering between
 *                elements. Must follow the standard qsort comparator contract.
 */
static void quick_sort_rec(void *base, int low, int high, size_t size, int (*compare)(const void *, const void *)) {
    if(low >= high) {
        return;
    }

    int pivot_idx = hoare_partition(base, low, high, size, compare);

    quick_sort_rec(base, low, pivot_idx, size, compare);
    quick_sort_rec(base, pivot_idx + 1, high, size, compare);
}

/**
 * @brief Sorts an array using the QuickSort algorithm.
 *
 * @param base    Pointer to the array to be sorted.
 * @param nitems  Number of elements in the array.
 * @param size    Size in bytes of each element in the array.
 * @param compare Comparison function used to determine ordering between
 *                elements. Must follow the standard qsort comparator contract.
 */
static void quick_sort(void *base, size_t nitems, size_t size, int (*compare)(const void *, const void *)) {
    if(nitems < 1 || size == 0 || base == NULL || compare == NULL) {
        return;
    }

    quick_sort_rec(base, 0, (int)nitems - 1, size, compare);
}

/**
 * @brief Recursive helper function for hybridsort algorithm.
 *
 * @param base    Pointer to the array being sorted.
 * @param low     Index of the lowest element in the current segment.
 * @param high    Index of the highest element in the current segment.
 * @param size    Size in bytes of each array element.
 * @param k       Threshold value: used to select the sorting algorithm.
 * @param compare Comparison function used to determine ordering between
 *                elements. Must follow the standard qsort comparator contract.
 */
static void hybridsort_rec(void *base, int low, int high, size_t size, size_t k, int (*compare) (const void *, const void *)) {
    size_t subarray_size = (size_t)(high - low + 1);

    // SelectionSort algorithm
    if(subarray_size < k) {
        void *sub_base = (uint8_t *)base + ( size_t)low * size;
        selection_sort(sub_base, subarray_size, size, compare);

        return;
    }

    // QuickSort algorithm
    if(low >= high) {
        return;
    }

    int pivot_idx = hoare_partition(base, low, high, size, compare);

    hybridsort_rec(base, low, pivot_idx, size, k, compare);
    hybridsort_rec(base, pivot_idx + 1, high, size, k, compare);
}

void hybridsort(void *base, size_t nitems, size_t size, size_t k, int (*compare)(const void*, const void *)) {
    if(nitems <= 1 || size < 1 || !base || !compare) {
        return;
    }

    if(k == 0) {
        quick_sort(base, nitems, size, compare);
        return;
    }

    if(k > nitems) {
        selection_sort(base, nitems, size, compare);
        return; 
    }

    hybridsort_rec(base, 0, (int)nitems - 1, size, k, compare);
}
