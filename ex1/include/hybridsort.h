#ifndef __HYBRIDSORT_H__
#define __HYBRIDSORT_H__

#include <stddef.h>

/**
 * @brief Sorts an array using a combination of quick sort and selection sort.
 *
 * @param base     Pointer to the array to be sorted.
 * @param nitems   Number of elements in the array.
 * @param size     Size in bytes of each element in the array.
 * @param k        Threshold: -if k = 0 the array will be sorted using quick sort,
 *                            -if k > nitems the array will be sorted using selection sort,
 *                            -otherwise the algorithm uses quick sort for sub arrays >= k
 *                             and selection sort for sub arrays < k.
 * @param compare  Pointer to the comparison function. This function returns
 *                 < 0 if the first element is less than the second,
 *                 = 0 if they are equal,
 *                 > 0 if the first element is greater than the second.
 */
void hybridsort(void *base, size_t nitems, size_t size, size_t k, int (*compare)(const void *, const void *));

#endif // __HYBRIDSORT_H__
