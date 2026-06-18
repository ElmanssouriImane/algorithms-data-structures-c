#include "hybridsort.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TEST(name) printf("\n=== Test: %s ===\n", name)

#define ASSERT_TRUE(expr, msg) {assert((expr) == 1); printf("PASSED: %s\n", msg);}
#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)
#define ASSERT_EQUALS(expected, actual, msg) ASSERT_TRUE(((expected) == (actual)), msg)
#define ASSERT_NOT_NULL(ptr, msg) ASSERT_TRUE(((ptr) != NULL), msg)
#define ASSERT_NULL(ptr, msg) ASSERT_TRUE(((ptr) == NULL), msg)

int int_compare(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;

    return ia - ib;
}

int int_compare_desc(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;

    return ib - ia;
}

int double_compare(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    double cmp = da - db;

    if(cmp < 0) {
        return -1;
    }
    if(cmp > 0) {
        return 1;
    }

    return 0;
}

int string_compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

typedef struct {
    int id;
    char name[32];
} Person;

int person_compare(const void *a, const void *b) {
    return ((Person *)a)->id - ((Person *)b)->id;
}

int is_sorted_int(int *arr, size_t n) {
    for(size_t k = 0; k < n - 1; k++) {
        if(arr[k] > arr[k + 1]) {
            return 0;
        }
    }
    return 1;
}

int is_strictly_sorted_int(int *arr, size_t n) {
    for(size_t k = 0; k < n - 1; k++) {
        if(arr[k] >= arr[k + 1]) {
            return 0;
        }
    }
    return 1;
}

int is_sorted_int_desc(int *arr, size_t n) {
    for(size_t k = 0; k < n - 1; k++) {
        if(arr[k] < arr[k + 1]) {
            return 0;
        }
    }
    return 1;
}

int is_strictly_sorted_int_desc(int *arr, size_t n) {
    for(size_t k = 0; k < n - 1; k++) {
        if(arr[k] <= arr[k + 1]) {
            return 0;
        }
    }
    return 1;
}

int is_strictly_sorted_double(double *arr, size_t n) {
    for(size_t k = 0; k < n - 1; k++) {
        if(arr[k] >= arr[k + 1]) {
            return 0;
        }
    }
    return 1;
}

int is_strictly_sorted_string(char **arr, size_t n) {
    for(size_t k = 0; k < n - 1; k++) {
        if(strcmp(arr[k], arr[k + 1]) >= 0) {
            return 0;
        }
    }
    return 1;
}

int is_strictly_sorted_person(Person *arr, size_t n) {
    for(size_t k = 0; k < n - 1; k++) {
        if(arr[k].id >= arr[k + 1].id) {
            return 0;
        }
    }
    return 1;
}

void test_invalid_params(void) {
    TEST("hybridsort with invalid parameters");

    int arr[] = {5, 2, 8, 1, 9};
    size_t n = 5;

    hybridsort(NULL, n, sizeof(int), 0, int_compare);
    ASSERT_TRUE(1, "NULL base should not crash!");

    hybridsort(arr, n, sizeof(int), 0, NULL);
    ASSERT_TRUE(1, "NULL compare should not crash!");

    hybridsort(arr, n, 0, 0, int_compare);
    ASSERT_TRUE(1, "Zero size should not crash!");

    hybridsort(NULL, n, 0, 0, NULL);
    ASSERT_TRUE(1, "All NULL parameters should not crash!");
}

void test_empty_and_single_element(void) {
    TEST("hybridsort with empty and single element arrays");

    int arr0[] = {3, 1, 2};
    hybridsort(arr0, 0, sizeof(int), 0, int_compare);
    ASSERT_TRUE(arr0[0] == 3 && arr0[1] == 1 && arr0[2] == 2, "Elements should remain unchanged!");

    int arr1[] = {42};
    hybridsort(arr1, 1, sizeof(int), 0, int_compare);
    ASSERT_EQUALS(42, arr1[0], "Single element should remain unchanged!");
}

void test_already_sorted(void) {
    TEST("hybridsort with already sorted array");

    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t n = 10;

    hybridsort(arr, n, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "Already sorted array should remain sorted!");
}

void test_reverse_sorted(void) {
    TEST("hybridsort with reverse sorted array");

    int arr[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    size_t n = 10;

    hybridsort(arr, n, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "Reverse sorted array should be sorted!");
}

void test_duplicates(void) {
    TEST("hybridsort with duplicate elements");

    int arr[] = {5, 2, 8, 2, 9, 5, 1, 8, 5};
    size_t n = 9;

    hybridsort(arr, n, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_sorted_int(arr, n), "Array with duplicates should be sorted!");
}

void test_all_same(void) {
    TEST("hybridsort with all identical elements");

    int arr[] = {7, 7, 7, 7, 7, 7, 7};
    size_t n = 7;

    hybridsort(arr, n, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_sorted_int(arr, n), "Array with all same elements should remain unchanged!");
}

void test_two_elements(void) {
    TEST("hybridsort with two elements");

    int arr1[] = {2, 1};
    hybridsort(arr1, 2, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr1, 2), "Two unsorted elements should be sorted!");

    int arr2[] = {1, 2};
    hybridsort(arr2, 2, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr2, 2), "Two sorted elements should remain sorted!");
}

void test_quicksort_only(void) {
    TEST("hybridsort with k=0 (pure quicksort)");

    int arr[] = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50};
    size_t n = 10;

    hybridsort(arr, n, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "k=0 should use quicksort and sort correctly!");
}

void test_selection_sort_only(void) {
    TEST("hybridsort with k>nitems (pure selection sort)");

    int arr[] = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50};
    size_t n = 10;

    hybridsort(arr, n, sizeof(int), 100, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "k>nitems should use selection sort and sort correctly!");
}

void test_hybrid_small_k(void) {
    TEST("hybridsort with small k threshold");

    int arr[] = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50, 23, 67, 89, 13};
    size_t n = 14;

    hybridsort(arr, n, sizeof(int), 3, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "Small k threshold should sort correctly!");
}

void test_hybrid_medium_k(void) {
    TEST("hybridsort with medium k threshold");

    int arr[] = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50, 23, 67, 89, 13};
    size_t n = 14;

    hybridsort(arr, n, sizeof(int), 7, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "Medium k threshold should sort correctly!");
}

void test_hybrid_k_equal_nitems(void) {
    TEST("hybridsort with k=nitems");

    int arr[] = {5, 2, 8, 1, 9, 3};
    size_t n = 6;

    hybridsort(arr, n, sizeof(int), n, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "k=nitems should sort correctly!");
}

void test_double_type(void) {
    TEST("hybridsort with double type");

    double arr[] = {3.14, 2.71, 1.41, 9.81, 6.28, 0.577};
    size_t n = 6;

    hybridsort(arr, n, sizeof(double), 0, double_compare);
    ASSERT_TRUE(is_strictly_sorted_double(arr, n), "Double array should be sorted correctly!");
}

void test_string_type(void) {
    TEST("hybridsort with string type");

    char *arr[] = {"zebra", "alpha", "mike", "bravo", "charlie", "delta"};
    size_t n = 6;

    hybridsort(arr, n, sizeof(char *), 0, string_compare);
    ASSERT_TRUE(is_strictly_sorted_string(arr, n), "String array should be sorted correctly!");
}

void test_struct_type(void) {
    TEST("hybridsort with struct type");

    Person arr[] = {
        {5, "Alice"},
        {2, "Bob"},
        {8, "Charlie"},
        {1, "David"},
        {9, "Eve"}
    };
    size_t n = 5;

    hybridsort(arr, n, sizeof(Person), 0, person_compare);
    ASSERT_TRUE(is_strictly_sorted_person(arr, n), "Struct array should be sorted correctly!");
}

void test_descending_order(void) {
    TEST("hybridsort with descending order");

    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    size_t n = 9;

    hybridsort(arr, n, sizeof(int), 0, int_compare_desc);
    ASSERT_TRUE(is_strictly_sorted_int_desc(arr, n), "Array should be sorted in descending order!");
}

void test_large_array(void) {
    TEST("hybridsort with large array (1000 elements)");

    int *arr = malloc(1000 * sizeof(int));
    srand((unsigned int)time(NULL));

    for(int k = 0; k < 1000; k++) {
        arr[k] = rand() % 10000;
    }

    hybridsort(arr, 1000, sizeof(int), 10, int_compare);
    ASSERT_TRUE(is_sorted_int(arr, 1000), "Large array should be sorted correctly!");

    free(arr);
}

void test_large_array_quicksort(void) {
    TEST("hybridsort with large array using pure quicksort");

    int *arr = malloc(1000 * sizeof(int));
    srand((unsigned int)time(NULL) + 1);

    for(int k = 0; k < 1000; k++) {
        arr[k] = rand() % 10000;
    }

    hybridsort(arr, 1000, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_sorted_int(arr, 1000), "Large array with quicksort should be sorted correctly!");

    free(arr);
}

void test_large_array_selection_sort(void) {
    TEST("hybridsort with medium array using pure selection sort");

    int *arr = malloc(100 * sizeof(int));
    srand((unsigned int)time(NULL) + 2);

    for(int k = 0; k < 100; k++) {
        arr[k] = rand() % 1000;
    }

    hybridsort(arr, 100, sizeof(int), 200, int_compare);
    ASSERT_TRUE(is_sorted_int(arr, 100), "Array with selection sort should be sorted correctly!");

    free(arr);
}

void test_negative_numbers(void) {
    TEST("hybridsort with negative numbers");

    int arr[] = {-5, 10, -3, 0, 7, -12, 8, -1, 3};
    size_t n = 9;

    hybridsort(arr, n, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "Array with negative numbers should be sorted correctly!");
}

void test_mixed_positive_negative(void) {
    TEST("hybridsort with mixed positive and negative numbers");

    int arr[] = {100, -50, 0, -100, 50, -25, 75, -75, 25};
    size_t n = 9;

    hybridsort(arr, n, sizeof(int), 4, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "Mixed positive/negative array should be sorted correctly!");
}

void test_random_small_arrays(void) {
    TEST("hybridsort with multiple random small arrays");

    srand((unsigned int)time(NULL) + 3);

    for(int test = 0; test < 10; test++) {
        int size = 5 + rand() % 10;
        int *arr = malloc((size_t)size * sizeof(int));

        for(int k = 0; k < size; k++) {
            arr[k] = rand() % 100;
        }

        int k = rand() % (size + 2);
        hybridsort(arr, (size_t)size, sizeof(int), (size_t)k, int_compare);

        if(!is_sorted_int(arr, (size_t)size)) {
            free(arr);
            ASSERT_TRUE(0, "Random small array should be sorted correctly!");
            return;
        }

        free(arr);
    }

    ASSERT_TRUE(1, "All random small arrays should be sorted correctly!");
}

void test_nearly_sorted(void) {
    TEST("hybridsort with nearly sorted array");

    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 9};
    size_t n = 10;

    hybridsort(arr, n, sizeof(int), 3, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "Nearly sorted array should be sorted correctly!");
}

void test_alternating_pattern(void) {
    TEST("hybridsort with alternating high-low pattern");

    int arr[] = {1, 10, 2, 9, 3, 8, 4, 7, 5, 6};
    size_t n = 10;

    hybridsort(arr, n, sizeof(int), 5, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr, n), "Alternating pattern should be sorted correctly!");
}

void test_many_duplicates(void) {
    TEST("hybridsort with many duplicate values");

    int arr[] = {1, 1, 1, 2, 2, 2, 3, 3, 3, 1, 2, 3};
    size_t n = 12;

    hybridsort(arr, n, sizeof(int), 4, int_compare);
    ASSERT_TRUE(is_sorted_int(arr, n), "Array with many duplicates should be sorted correctly!");
}

void test_three_elements_all_cases(void) {
    TEST("hybridsort with three elements - all permutations");

    int arr1[] = {1, 2, 3};
    hybridsort(arr1, 3, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr1, 3), "Permutation 1-2-3 should be sorted!");

    int arr2[] = {1, 3, 2};
    hybridsort(arr2, 3, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr2, 3), "Permutation 1-3-2 should be sorted!");

    int arr3[] = {2, 1, 3};
    hybridsort(arr3, 3, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr3, 3), "Permutation 2-1-3 should be sorted!");

    int arr4[] = {2, 3, 1};
    hybridsort(arr4, 3, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr4, 3), "Permutation 2-3-1 should be sorted!");

    int arr5[] = {3, 1, 2};
    hybridsort(arr5, 3, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr5, 3), "Permutation 3-1-2 should be sorted!");

    int arr6[] = {3, 2, 1};
    hybridsort(arr6, 3, sizeof(int), 0, int_compare);
    ASSERT_TRUE(is_strictly_sorted_int(arr6, 3), "Permutation 3-2-1 should be sorted!");
}

void test_different_k_values_same_array(void) {
    TEST("hybridsort with different k values on same input");

    int original[] = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50};
    size_t n = 10;

    for(size_t k = 0; k <= 15; k++) {
        int arr[10];
        memcpy(arr, original, sizeof(original));

        hybridsort(arr, n, sizeof(int), k, int_compare);

        if(!is_strictly_sorted_int(arr, n)) {
            ASSERT_TRUE(0, "Different k values should all produce sorted arrays!");
            return;
        }
    }

    ASSERT_TRUE(1, "All k values should produce correctly sorted arrays!");
}

int main(void) {
    printf(
        "\n"
        "+----------------------------------------+\n"
        "|         HYBRIDSORT UNIT TESTS          |\n"
        "+----------------------------------------+\n"
    );

    test_invalid_params();
    test_empty_and_single_element();
    test_already_sorted();
    test_reverse_sorted();
    test_duplicates();
    test_all_same();
    test_two_elements();

    test_quicksort_only();
    test_selection_sort_only();
    test_hybrid_small_k();
    test_hybrid_medium_k();
    test_hybrid_k_equal_nitems();

    test_double_type();
    test_string_type();
    test_struct_type();
    test_descending_order();

    test_large_array();
    test_large_array_quicksort();
    test_large_array_selection_sort();

    test_negative_numbers();
    test_mixed_positive_negative();
    test_random_small_arrays();
    test_nearly_sorted();
    test_alternating_pattern();
    test_many_duplicates();
    test_three_elements_all_cases();
    test_different_k_values_same_array();

    printf(
        "\n"
        "+----------------------------------------+\n"
        "|          ALL TESTS PASSED!             |\n"
        "+----------------------------------------+\n"
        "\n"
    );

    return 0;
}
