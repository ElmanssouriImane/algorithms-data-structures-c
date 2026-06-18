#include "priority_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) printf("\n=== Test: %s ===\n", name)

#define ASSERT_TRUE(expr, msg) {assert((expr) == 1); printf("PASSED: %s\n", msg);}

#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)
#define ASSERT_EQUALS(expected, actual, msg) ASSERT_TRUE(((expected) == (actual)), msg)
#define ASSERT_DOUBLE_EQUALS(expected, actual, msg) ASSERT_TRUE((-0.0001 < (expected) - (actual) && (expected) - (actual) < 0.0001), msg)
#define ASSERT_NOT_NULL(ptr, msg) ASSERT_TRUE(((ptr) != NULL), msg)
#define ASSERT_NULL(ptr, msg) ASSERT_TRUE(((ptr) == NULL), msg)

static int int_compare(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    if(ia < ib) return -1;
    if(ia > ib) return 1;
    return 0;
}

static unsigned long int_hash(const void *key) {
    return (unsigned long)(*(const int *)key);
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

static int string_compare(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

static unsigned long string_hash(const void *key) {
    const char *str = (const char *)key;
    unsigned long hash = 5381;
    int c;

    while((c = *str++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }

    return hash;
}

typedef struct {
    int x;
    int y;
    int distance;
} Point2D;

static int point_compare(const void *a, const void *b) {
    const Point2D *pa = (const Point2D *)a;
    const Point2D *pb = (const Point2D *)b;
    if(pa->distance < pb->distance) return -1;
    if(pa->distance > pb->distance) return 1;
    return 0;
}

static unsigned long point_hash(const void *key) {
    const Point2D *p = (const Point2D *)key;
    return (unsigned long)(p->x * 1000 + p->y);
}

void test_create_valid(void) {
    TEST("priority_queue_create with valid parameters");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);

    ASSERT_NOT_NULL(pq, "Priority queue should be created!");
    ASSERT_EQUALS(0, priority_queue_size(pq), "New queue size should be 0!");
    ASSERT_NULL(priority_queue_top(pq), "Top of empty queue should be NULL!");

    priority_queue_free(pq);
}

void test_create_invalid(void) {
    TEST("priority_queue_create with invalid parameters");

    PriorityQueue *pq1 = priority_queue_create(NULL, int_hash);
    ASSERT_NULL(pq1, "Queue should be NULL with NULL compare function!");

    PriorityQueue *pq2 = priority_queue_create(int_compare, NULL);
    ASSERT_NULL(pq2, "Queue should be NULL with NULL hash function!");

    PriorityQueue *pq3 = priority_queue_create(NULL, NULL);
    ASSERT_NULL(pq3, "Queue should be NULL with both functions NULL!");
}

void test_push_and_top_simple(void) {
    TEST("priority_queue_push and priority_queue_top with simple values");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    ASSERT_NOT_NULL(pq, "Priority queue should be created!");

    int a = 5, b = 1, c = 7;

    ASSERT_TRUE(priority_queue_push(pq, &a), "First push should succeed!");
    ASSERT_EQUALS(1, priority_queue_size(pq), "Size should be 1 after first push!");
    ASSERT_EQUALS(&a, priority_queue_top(pq), "Top should be first and only element!");

    ASSERT_TRUE(priority_queue_push(pq, &b), "Second push should succeed!");
    ASSERT_EQUALS(2, priority_queue_size(pq), "Size should be 2 after second push!");
    ASSERT_EQUALS(&a, priority_queue_top(pq), "Top should still be max element 5!");

    ASSERT_TRUE(priority_queue_push(pq, &c), "Third push should succeed!");
    ASSERT_EQUALS(3, priority_queue_size(pq), "Size should be 3 after third push!");
    ASSERT_EQUALS(&c, priority_queue_top(pq), "Top should be max element 7!");

    priority_queue_free(pq); 
}

void test_push_invalid_and_duplicated(void) {
    TEST("priority_queue_push with invalid parameters and duplicates");

    int x = 10; 
    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    ASSERT_NOT_NULL(pq, "Priority queue should be created!");

    priority_queue_push(NULL, &x);
    priority_queue_push(pq, NULL);

    ASSERT_EQUALS(0, priority_queue_size(pq), "Size should remain 0 with invalid push parameters!");

    ASSERT_TRUE(priority_queue_push(pq, &x), "First push of element should succeed!");
    ASSERT_EQUALS(1, priority_queue_size(pq), "Size should be 1 after first push!");
    ASSERT_TRUE(priority_queue_contains(pq, &x), "Queue should contain pushed element!");

    ASSERT_FALSE(priority_queue_push(pq, &x), "Second push of same element should be ignored!");
    ASSERT_EQUALS(1, priority_queue_size(pq), "Size should remain 1 after duplicate push!");

    priority_queue_free(pq);
}

void test_contains(void) {
    TEST("priority_queue_contains");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    ASSERT_NOT_NULL(pq, "Priority queue should be created!");

    int a = 3, b = 8;

    ASSERT_EQUALS(0, priority_queue_contains(pq, &a), "Empty queue should not contain element!");
    ASSERT_EQUALS(0, priority_queue_contains(NULL, &a), "Contains should be 0 with NULL queue!");
    ASSERT_EQUALS(0, priority_queue_contains(pq, NULL), "Contains should be 0 with NULL data!");

    priority_queue_push(pq, &a);
    ASSERT_TRUE(priority_queue_contains(pq, &a), "Queue should contain pushed element!");
    ASSERT_FALSE(priority_queue_contains(pq, &b), "Queue should not contain non-inserted element!");

    priority_queue_free(pq);
}

void test_pop_behaviour(void) {
    TEST("priority_queue_pop behaviour on empty and non-empty queue");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    ASSERT_NOT_NULL(pq, "Priority queue should be created!");

    /* Pop on empty queue should be safe. */
    priority_queue_pop(pq); 
    ASSERT_EQUALS(0, priority_queue_size(pq), "Size should still be 0 after pop on empty queue!");
    ASSERT_NULL(priority_queue_top(pq), "Top should be NULL on empty queue!");

    int a = 2, b = 9, c = 4;

    priority_queue_push(pq, &a);
    priority_queue_push(pq, &b);
    priority_queue_push(pq, &c);
    ASSERT_EQUALS(3, priority_queue_size(pq), "Size should be 3 after three pushes!");
    ASSERT_EQUALS(&b, priority_queue_top(pq), "Top should be max element 9!");

    priority_queue_pop(pq); 
    ASSERT_EQUALS(2, priority_queue_size(pq), "Size should be 2 after one pop!");
    ASSERT_FALSE(priority_queue_contains(pq, &b), "Queue should not contain removed max element!");
    
    void *new_top = priority_queue_top(pq); 
    ASSERT_NOT_NULL(new_top, "Top should not be NULL after first pop!");

    int *new_top_int = (int *)new_top; 
    ASSERT_TRUE((*new_top_int == 4 || *new_top_int == 2), "New top should be one of remaining elements!");

    priority_queue_pop(pq);
    priority_queue_pop(pq);
    ASSERT_EQUALS(0, priority_queue_size(pq), "Size should be 0 after popping all elements!");
    ASSERT_NULL(priority_queue_top(pq), "Top should be NULL when queue is empty!");

    priority_queue_free(pq);
}

void test_remove(void) {
    TEST("priority_queue_remove with various cases"); 

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    ASSERT_NOT_NULL(pq, "Priority queue should be created!");

    int a = 5, b = 10, c = 7;

    ASSERT_EQUALS(0, priority_queue_remove(NULL, &a), "Remove should fail with NULL queue!");
    ASSERT_EQUALS(0, priority_queue_remove(pq, NULL), "Remove should fail with NULL data!");
    ASSERT_EQUALS(0, priority_queue_remove(pq, &a), "Remove should return 0 for non-existing element!");

    priority_queue_push(pq, &a);
    priority_queue_push(pq, &b);
    priority_queue_push(pq, &c);
    ASSERT_EQUALS(3, priority_queue_size(pq), "Size should be 3 before removals!");

    ASSERT_EQUALS(1, priority_queue_remove(pq, &c), "Remove existing middle element should return 1!");
    ASSERT_FALSE(priority_queue_contains(pq, &c), "Queue should not contain removed element!");
    ASSERT_EQUALS(2, priority_queue_size(pq), "Size should be 2 after removing one element!");

    ASSERT_EQUALS(1, priority_queue_remove(pq, &b), "Remove max element should return 1!");
    ASSERT_FALSE(priority_queue_contains(pq, &b), "Queue should not contain removed max element!");
    ASSERT_EQUALS(1, priority_queue_size(pq), "Size should be 1 after removing second element!");

    ASSERT_EQUALS(1, priority_queue_remove(pq, &a), "Remove last remaining element should return 1!");
    ASSERT_EQUALS(0, priority_queue_size(pq), "Size should be 0 after removing all elements!");
    ASSERT_NULL(priority_queue_top(pq), "Top should be NULL after removing all elements!");

    priority_queue_free(pq); 
}

void test_size(void) {
    TEST("priority_queue_size");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    ASSERT_NOT_NULL(pq, "Priority queue should be created!");

    ASSERT_EQUALS(0, priority_queue_size(pq), "Size should be 0 initially!");

    int a = 1, b = 2;
    priority_queue_push(pq, &a);
    ASSERT_EQUALS(1, priority_queue_size(pq), "Size should be 1 after one push!");

    priority_queue_push(pq, &b);
    ASSERT_EQUALS(2, priority_queue_size(pq), "Size should be 2 after two pushes!");

    priority_queue_pop(pq);
    ASSERT_EQUALS(1, priority_queue_size(pq), "Size should be 1 after one pop!");

    priority_queue_free(pq);
}

void test_size_invalid_params(void) {
    TEST("priority_queue_size with NULL queue");

    ASSERT_EQUALS(0, priority_queue_size(NULL), "Size should be 0 for NULL queue!");
}

void test_heap_property_maintained(void) {
    TEST("Verify ternary heap property is maintained");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    int values[] = {15, 3, 9, 21, 7, 12, 5, 18};

    for(int i = 0; i < 8; i++) {
        priority_queue_push(pq, &values[i]);
    }

    int prev = 1e6;
    while(priority_queue_size(pq) > 0) {
        int *current = (int *)priority_queue_top(pq);
        ASSERT_TRUE(*current <= prev, "Elements should come out in descending order!");
        prev = *current;
        priority_queue_pop(pq);
    }

    priority_queue_free(pq);
}

void test_remove_various_positions(void) {
    TEST("priority_queue_remove from root, leaf, and middle");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    int vals[] = {50, 20, 30, 10, 5, 15, 25};

    for(int i = 0; i < 7; i++) {
        priority_queue_push(pq, &vals[i]);
    }

    // Remove root (max element)
    ASSERT_TRUE(priority_queue_remove(pq, &vals[0]), "Remove root should succeed!");
    ASSERT_EQUALS(6, priority_queue_size(pq), "Size should decrease after root removal!");

    priority_queue_free(pq);
}

void test_negative_values(void) {
    TEST("priority_queue with negative integers");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    int a = -10, b = -5, c = -20;

    priority_queue_push(pq, &a);
    priority_queue_push(pq, &b);
    priority_queue_push(pq, &c);

    ASSERT_EQUALS(&b, priority_queue_top(pq), "Top should be max (-5)!");

    priority_queue_free(pq);
}

void test_many_equal_elements(void) {
    TEST("priority_queue with many equal priority elements");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    int vals[100];

    for(int i = 0; i < 50; i++) {
        vals[i] = 42; // Same value
        priority_queue_push(pq, &vals[i]);
    }

    ASSERT_EQUALS(1, priority_queue_size(pq), "Should accept 1 distinct pointer!");

    priority_queue_free(pq);
}

void test_push_resize_capacity(void) {
    TEST("priority_queue automatic resize");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    ASSERT_NOT_NULL(pq, "Priority queue should be created!");

    int count = 32 * 3;
    int *values = (int *)malloc((size_t)count * sizeof(int));
    ASSERT_NOT_NULL(values, "Values array should be allocated!");

    for(int i = 0; i < count; i++) {
        values[i] = i;
        priority_queue_push(pq, &values[i]);
    }

    ASSERT_EQUALS(count, priority_queue_size(pq), "All elements should be inserted!");
    int *top_val = (int *)priority_queue_top(pq);
    ASSERT_NOT_NULL(top_val, "Top should not be NULL after many pushes!");
    ASSERT_EQUALS(count - 1, *top_val, "Top should be maximum integer pushed!");

    free(values);
    priority_queue_free(pq);
}

void test_free_null_queue(void) {
    TEST("priority_queue_free with NULL queue");

    priority_queue_free(NULL);
    ASSERT_TRUE(1, "Freeing NULL queue should not crash!");
}

void test_int_values(void) {
    TEST("Priority Queue with integers");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);
    ASSERT_NOT_NULL(pq, "Priority queue should be created!");

    int values[] = {5, 10, -3, 8, 1, -15, 7, 0};
    int n = 8;

    for(int i = 0; i < n; i++) {
        priority_queue_push(pq, &values[i]);
    }

    ASSERT_EQUALS(n, priority_queue_size(pq), "Size should match number of insertions!");

    int *top = (int *)priority_queue_top(pq);
    ASSERT_NOT_NULL(top, "Top should not be NULL!");
    ASSERT_EQUALS(10, *top, "Top should be maximum value!");

    priority_queue_free(pq);
}

void test_int_sequential_pop(void) {
    TEST("Priority Queue with integers - sequential pop in order");

    PriorityQueue *pq = priority_queue_create(int_compare, int_hash);

    int values[] = {20, 5, 30, 15, 10, 25};
    int expected[] = {30, 25, 20, 15, 10, 5};
    int n = 6;

    for(int i = 0; i < n; i++) {
        priority_queue_push(pq, &values[i]);
    }

    for(int i = 0; i < n; i++) {
        int *top = (int *)priority_queue_top(pq);
        ASSERT_NOT_NULL(top, "Top should not be NULL!");
        ASSERT_EQUALS(expected[i], *top, "Elements should come out in descending order!");
        priority_queue_pop(pq);
    }

    ASSERT_EQUALS(0, priority_queue_size(pq), "Queue should be empty after popping all!");

    priority_queue_free(pq);
}

void test_double_precision_values(void) {
    TEST("Priority Queue with double precision values");

    PriorityQueue *pq = priority_queue_create(double_compare, double_hash);

    double values[] = {3.14159, 2.71828, 1.41421, -4.66920, 0.57721};

    for(int i = 0; i < 5; i++) {
        priority_queue_push(pq, &values[i]);
    }

    double *top = (double *)priority_queue_top(pq);
    ASSERT_NOT_NULL(top, "Top should not be NULL!");
    ASSERT_DOUBLE_EQUALS(3.14159, *top, "Top should be maximum double value!");

    priority_queue_free(pq);
}

void test_double_sorted_extraction(void) {
    TEST("Priority Queue with doubles - sorted extraction");

    PriorityQueue *pq = priority_queue_create(double_compare, double_hash);

    double values[] = {1.5, -3.7, 2.2, 5.1, -0.8, 4.3};
    double expected[] = {5.1, 4.3, 2.2, 1.5, -0.8, -3.7};

    for(int i = 0; i < 6; i++) {
        priority_queue_push(pq, &values[i]);
    }

    for(int i = 0; i < 6; i++) {
        double *top = (double *)priority_queue_top(pq);
        ASSERT_DOUBLE_EQUALS(expected[i], *top, "Doubles should come out in descending order!");
        priority_queue_pop(pq);
    }

    priority_queue_free(pq);
}

void test_string_alphabetical_order(void) {
    TEST("Priority Queue with strings - alphabetical ordering");

    PriorityQueue *pq = priority_queue_create(string_compare, string_hash);

    char *words[] = {"zebra", "apple", "mango", "banana", "orange"};

    for(int i = 0; i < 5; i++) {
        priority_queue_push(pq, words[i]);
    }

    char *top = (char *)priority_queue_top(pq);
    ASSERT_TRUE(strcmp(top, "zebra") == 0, "Top should be 'zebra' (lexicographically largest)!");

    priority_queue_free(pq);
}

void test_string_sequential_extraction(void) {
    TEST("Priority Queue with strings - sequential extraction");

    PriorityQueue *pq = priority_queue_create(string_compare, string_hash);

    char *words[] = {"dog", "cat", "elephant", "ant", "bear"};
    char *expected[] = {"elephant", "dog", "cat", "bear", "ant"};

    for(int i = 0; i < 5; i++) {
        priority_queue_push(pq, words[i]);
    }

    for(int i = 0; i < 5; i++) {
        char *top = (char *)priority_queue_top(pq);
        ASSERT_TRUE(strcmp(top, expected[i]) == 0, "Strings should come out in descending order!");
        priority_queue_pop(pq);
    }

    priority_queue_free(pq);
}

void test_string_empty_and_long(void) {
    TEST("Priority Queue with empty and long strings");

    PriorityQueue *pq = priority_queue_create(string_compare, string_hash);

    char *empty = "";
    char *short_str = "hi";
    char *long_str = "this_is_a_very_long_string_for_testing_priority_queue";

    priority_queue_push(pq, empty);
    priority_queue_push(pq, short_str);
    priority_queue_push(pq, long_str);

    ASSERT_EQUALS(3, priority_queue_size(pq), "Size should be 3!");

    char *top = (char *)priority_queue_top(pq);
    ASSERT_TRUE(strcmp(top, long_str) == 0, "Long string should be on top!");

    priority_queue_free(pq);
}

void test_string_remove_specific(void) {
    TEST("Priority Queue with strings - remove specific string");

    PriorityQueue *pq = priority_queue_create(string_compare, string_hash);

    char *words[] = {"first", "second", "third", "fourth"};

    for(int i = 0; i < 4; i++) {
        priority_queue_push(pq, words[i]);
    }

    ASSERT_TRUE(priority_queue_remove(pq, words[1]), "Remove 'second' should succeed!");
    ASSERT_FALSE(priority_queue_contains(pq, words[1]), "Should not contain 'second'!");
    ASSERT_EQUALS(3, priority_queue_size(pq), "Size should be 3!");

    priority_queue_free(pq);
}

void test_struct_point_distance(void) {
    TEST("Priority Queue with Point2D struct - distance ordering");

    PriorityQueue *pq = priority_queue_create(point_compare, point_hash);

    Point2D p1 = {3, 4, 5};
    Point2D p2 = {1, 1, 1};
    Point2D p3 = {5, 12, 13};
    Point2D p4 = {0, 0, 0};

    priority_queue_push(pq, &p1);
    priority_queue_push(pq, &p2);
    priority_queue_push(pq, &p3);
    priority_queue_push(pq, &p4);

    Point2D *top = (Point2D *)priority_queue_top(pq);
    ASSERT_EQUALS(13, top->distance, "Farthest point should be on top!");

    priority_queue_free(pq);
}

int main(void) {
    printf (
        "\n"
        "+----------------------------------------+\n"
        "|        PRIORITY QUEUE UNIT TESTS       |\n"
        "+----------------------------------------+\n"
    ); 

    test_create_valid();
    test_create_invalid(); 
    
    test_push_and_top_simple();
    test_push_invalid_and_duplicated(); 

    test_contains(); 

    test_pop_behaviour(); 
    test_remove(); 

    test_size(); 
    test_size_invalid_params();

    test_heap_property_maintained();
    test_remove_various_positions();
    test_negative_values();
    test_many_equal_elements();

    test_push_resize_capacity(); 
    test_free_null_queue();

    test_int_values();
    test_int_sequential_pop();

    test_double_precision_values();
    test_double_sorted_extraction();

    test_string_alphabetical_order();
    test_string_sequential_extraction();
    test_string_empty_and_long();
    test_string_remove_specific();

    test_struct_point_distance();

    printf(
        "\n"
        "+----------------------------------------+\n"
        "|           ALL PQ TESTS PASSED!         |\n"
        "+----------------------------------------+\n"
        "\n"
    ); 

    return 0; 
}
