#include "hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define TEST(name) printf("\n=== Test: %s ===\n", name)

#define ASSERT_TRUE(expr, msg) {assert((expr) == 1); printf("PASSED: %s\n", msg);}
#define ASSERT_FALSE(expr, msg) ASSERT_TRUE(!(expr), msg)
#define ASSERT_EQUALS(expected, actual, msg) ASSERT_TRUE(((expected) == (actual)), msg)
#define ASSERT_DOUBLE_EQUALS(expected, actual, msg) ASSERT_TRUE((-0.0001 < (expected) - (actual) && (expected) - (actual) < 0.0001), msg)
#define ASSERT_NOT_NULL(ptr, msg) ASSERT_TRUE(((ptr) != NULL), msg)
#define ASSERT_NULL(ptr, msg) ASSERT_TRUE(((ptr) == NULL), msg)

int int_compare(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;

    return ia - ib;
}

unsigned long int_hash(const void *key) {
    return (unsigned long)(*(const int *)key);
}

int double_compare(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if(-0.0001 < da - db && da - db < 0.0001) return 0;
    return (da > db) ? 1 : -1;
}

unsigned long double_hash(const void *key) {
    double d = *(const double *)key;
    uint64_t bits;
    memcpy(&bits, &d, sizeof(double));
    return (unsigned long)bits;
}

int string_compare(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

unsigned long string_hash(const void *key) {
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
} Person;

int person_compare(const void *a, const void *b) {
    const Person *pa = (const Person *)a;
    const Person *pb = (const Person *)b;
    return pa->id - pb->id;
}

unsigned long person_hash(const void *key) {
    const Person *p = (const Person *)key;
    return (unsigned long)(p->id);
}

void test_create_valid(void) {
    TEST("hash_table_create with valid parameters");

    HashTable *table = hash_table_create(string_compare, string_hash);

    ASSERT_NOT_NULL(table, "Table should be created!");
    ASSERT_EQUALS(0, hash_table_size(table), "Size should be 0!");

    hash_table_free(table);
}

void test_create_invalid(void) {
    TEST("hash_table_create with invalid parameters");

    HashTable *table1 = hash_table_create(NULL, string_hash);
    ASSERT_NULL(table1, "Table should be NULL with NULL compare function!");

    HashTable *table2 = hash_table_create(string_compare, NULL);
    ASSERT_NULL(table2, "Table should be NULL with NULL hash function!");

    HashTable *table3 = hash_table_create(NULL, NULL);
    ASSERT_NULL(table3, "Table should be NULL with both NULL functions!");
}

void test_put_and_get_simple(void) {
    TEST("hash_table_put and hash_table_get with simple values");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *key1 = "hello";
    char *value1 = "world";

    hash_table_put(table, key1, value1);
    ASSERT_EQUALS(1, hash_table_size(table), "Size should be 1 after one insertion!");

    void *result = hash_table_get(table, key1);
    ASSERT_EQUALS(value1, result, "Retrieved value should match inserted value!");

    hash_table_free(table);
}

void test_put_update_existing(void) {
    TEST("hash_table_put updating existing key");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *key = "name";
    char *value1 = "Alice";
    char *value2 = "Bob";

    hash_table_put(table, key, value1);
    ASSERT_EQUALS(1, hash_table_size(table), "Size should be 1 after first insertion!");

    hash_table_put(table, key, value2);
    ASSERT_EQUALS(1, hash_table_size(table), "Size should still be 1 after updating!");

    void *result = hash_table_get(table, key);
    ASSERT_EQUALS(value2, result, "Value should be updated!");

    hash_table_free(table);
}

void test_put_multiple_keys(void) {
    TEST("hash_table_put with multiple keys");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *keys[] = {"key1", "key2", "key3", "key4", "key5"};
    char *values[] = {"val1", "val2", "val3", "val4", "val5"};

    for(int k = 0; k < 5; k++) {
        hash_table_put(table, keys[k], values[k]);
    }

    ASSERT_EQUALS(5, hash_table_size(table), "Size should be 5!");

    for(int k = 0; k < 5; k++) {
        void *result = hash_table_get(table, keys[k]);
        ASSERT_EQUALS(values[k], result, "Each value should be retrievable!");
    }

    hash_table_free(table);
}

void test_put_invalid_params(void) {
    TEST("hash_table_put with invalid parameters");

    HashTable *table = hash_table_create(string_compare, string_hash);
    char *key = "test";
    char *value = "value";

    hash_table_put(NULL, key, value);
    hash_table_put(table, NULL, value);

    ASSERT_EQUALS(0, hash_table_size(table), "Size should remain 0 with invalid parameters!");

    hash_table_free(table);
}

void test_get_non_existent(void) {
    TEST("hash_table_get with non-existent key");

    HashTable *table = hash_table_create(string_compare, string_hash);

    hash_table_put(table, "exists", "value");

    void *result = hash_table_get(table, "does_not_exist");
    ASSERT_NULL(result, "Should return NULL for non-existent key!");

    hash_table_free(table);
}

void test_get_invalid_params(void) {
    TEST("hash_table_get with invalid parameters");

    HashTable *table = hash_table_create(string_compare, string_hash);

    void *result1 = hash_table_get(NULL, "key");
    ASSERT_NULL(result1, "Should return NULL with NULL table!");

    void *result2 = hash_table_get(table, NULL);
    ASSERT_NULL(result2, "Should return NULL with NULL key!");

    hash_table_free(table);
}

void test_contains_key(void) {
    TEST("hash_table_contains_key");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *key1 = "present";
    char *key2 = "absent";

    hash_table_put(table, key1, "value");

    ASSERT_TRUE(hash_table_contains_key(table, key1), "Should contain existing key!");
    ASSERT_FALSE(hash_table_contains_key(table, key2), "Should not contain non-existent key!");

    hash_table_free(table);
}

void test_contains_key_invalid_params(void) {
    TEST("hash_table_contains_key with invalid parameters");

    HashTable *table = hash_table_create(string_compare, string_hash);

    ASSERT_FALSE(hash_table_contains_key(NULL, "key"), "Should return 0 with NULL table!");
    ASSERT_FALSE(hash_table_contains_key(table, NULL), "Should return 0 with NULL key!");

    hash_table_free(table);
}

void test_remove_existing(void) {
    TEST("hash_table_remove with existing key");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *key = "to_remove";
    hash_table_put(table, key, "value");
    ASSERT_EQUALS(1, hash_table_size(table), "Size should be 1!");

    hash_table_remove(table, key);
    ASSERT_EQUALS(0, hash_table_size(table), "Size should be 0 after removal!");
    ASSERT_FALSE(hash_table_contains_key(table, key), "Key should not be present after removal!");

    hash_table_free(table);
}

void test_remove_non_existent(void) {
    TEST("hash_table_remove with non-existent key");

    HashTable *table = hash_table_create(string_compare, string_hash);

    hash_table_put(table, "exists", "value");
    ASSERT_EQUALS(1, hash_table_size(table), "Size should be 1!");

    hash_table_remove(table, "does_not_exist");
    ASSERT_EQUALS(1, hash_table_size(table), "Size should remain 1!");

    hash_table_free(table);
}

void test_remove_from_chain(void) {
    TEST("hash_table_remove from collision chain");

    HashTable *table = hash_table_create(int_compare, int_hash);

    // Force collisions by using keys that hash to same bucket
    int keys[] = {0, 23, 46}; // These will collide in initial capacity (23)
    int values[] = {100, 200, 300};

    for(int k = 0; k < 3; k++) {
        hash_table_put(table, &keys[k], &values[k]);
    }

    ASSERT_EQUALS(3, hash_table_size(table), "Size should be 3!");

    hash_table_remove(table, &keys[1]);
    ASSERT_EQUALS(2, hash_table_size(table), "Size should be 2 after removal!");
    ASSERT_FALSE(hash_table_contains_key(table, &keys[1]), "Middle key should be removed!");
    ASSERT_TRUE(hash_table_contains_key(table, &keys[0]), "First key should still exist!");
    ASSERT_TRUE(hash_table_contains_key(table, &keys[2]), "Last key should still exist!");

    hash_table_free(table);
}

void test_remove_invalid_params(void) {
    TEST("hash_table_remove with invalid parameters");

    HashTable *table = hash_table_create(string_compare, string_hash);
    char *key = "test";
    hash_table_put(table, key, "value");

    hash_table_remove(NULL, key);
    hash_table_remove(table, NULL);

    ASSERT_EQUALS(1, hash_table_size(table), "Size should remain 1 with invalid parameters!");

    hash_table_free(table);
}

void test_size(void) {
    TEST("hash_table_size");

    HashTable *table = hash_table_create(string_compare, string_hash);

    ASSERT_EQUALS(0, hash_table_size(table), "Size should be 0 initially!");

    hash_table_put(table, "k1", "v1");
    ASSERT_EQUALS(1, hash_table_size(table), "Size should be 1!");

    hash_table_put(table, "k2", "v2");
    ASSERT_EQUALS(2, hash_table_size(table), "Size should be 2!");

    hash_table_remove(table, "k1");
    ASSERT_EQUALS(1, hash_table_size(table), "Size should be 1 after removal!");

    hash_table_free(table);
}

void test_size_invalid_params(void) {
    TEST("hash_table_size with NULL table");

    ASSERT_EQUALS(0, hash_table_size(NULL), "Size should be 0 for NULL table!");
}

void test_keyset(void) {
    TEST("hash_table_keyset");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *keys[] = {"apple", "banana", "cherry"};
    for(int k = 0; k < 3; k++) {
        hash_table_put(table, keys[k], "value");
    }

    void **keyset = hash_table_keyset(table);
    ASSERT_NOT_NULL(keyset, "Keyset should not be NULL!");

    // Verify all keys are present
    int key_count = 0;
    for(int k = 0; keyset[k] != NULL; k++) {
        key_count++;

        int found = 0;
        for(int j = 0; j < 3; j++) {
            if(strcmp((char *)keyset[k], keys[j]) == 0) {
                found = 1;
                break;
            }
        }

        ASSERT_TRUE(found, "Each key in keyset should be valid!");
    }

    ASSERT_EQUALS(3, key_count, "Keyset should contain 3 keys!");

    free(keyset);
    hash_table_free(table);
}

void test_keyset_empty(void) {
    TEST("hash_table_keyset on empty table");

    HashTable *table = hash_table_create(string_compare, string_hash);

    void **keyset = hash_table_keyset(table);
    ASSERT_NOT_NULL(keyset, "Keyset should not be NULL even for empty table!");
    ASSERT_NULL(keyset[0], "First element should be NULL for empty table!");

    free(keyset);
    hash_table_free(table);
}

void test_keyset_invalid_params(void) {
    TEST("hash_table_keyset with NULL table");

    void **keyset = hash_table_keyset(NULL);
    ASSERT_NULL(keyset, "Keyset should be NULL for NULL table!");
}

void test_resize(void) {
    TEST("hash_table automatic resize");

    HashTable *table = hash_table_create(string_compare, string_hash);
    int initial_capacity;
    memcpy(&initial_capacity, (uint8_t *)table + sizeof(HashTable **), sizeof(int));

    // Insert enough elements to trigger resize (> 75% load factor)
    int num_elements = (int)(.75f * (float)initial_capacity) + 5;
    char **keys = malloc(sizeof(char *) * (size_t)num_elements);

    for(int k = 0; k < num_elements; k++) {
        keys[k] = malloc(20);
        snprintf(keys[k], 20, "key_%d", k);
        hash_table_put(table, keys[k], keys[k]);
    }

    int curr_capacity;
    memcpy(&curr_capacity, (uint8_t *)table + sizeof(HashTable **), sizeof(int));

    ASSERT_TRUE(curr_capacity > initial_capacity, "Capacity should increase after resize!");
    ASSERT_EQUALS(num_elements, hash_table_size(table), "All elements should be present after resize!");

    // Verify all elements are still accessible after resize
    for(int k = 0; k < num_elements; k++) {
        void *result = hash_table_get(table, keys[k]);
        ASSERT_NOT_NULL(result, "All keys should be accessible after resize!");
    }

    for(int k = 0; k < num_elements; k++) {
        free(keys[k]);
    }
    free(keys);
    hash_table_free(table);
}

void test_collision_handling(void) {
    TEST("hash_table collision handling");

    HashTable *table = hash_table_create(int_compare, int_hash);

    // Create keys that will collide
    int keys[] = {0, 23, 46, 69}; // Will collide in bucket 0 with capacity 23
    int values[] = {100, 200, 300, 400};

    for(int k = 0; k < 4; k++) {
        hash_table_put(table, &keys[k], &values[k]);
    }

    ASSERT_EQUALS(4, hash_table_size(table), "All elements should be inserted!");

    // Verify all elements are retrievable
    for(int k = 0; k < 4; k++) {
        void *result = hash_table_get(table, &keys[k]);
        ASSERT_EQUALS(&values[k], result, "All colliding elements should be retrievable!");
    }

    hash_table_free(table);
}

void test_empty_table_operations(void) {
    TEST("Operations on empty table");

    HashTable *table = hash_table_create(string_compare, string_hash);

    ASSERT_EQUALS(0, hash_table_size(table), "Empty table size should be 0!");
    ASSERT_NULL(hash_table_get(table, "any_key"), "Get on empty table should return NULL!");
    ASSERT_FALSE(hash_table_contains_key(table, "any_key"), "Contains on empty table should return 0!");

    hash_table_remove(table, "any_key");
    ASSERT_EQUALS(0, hash_table_size(table), "Remove on empty table should not crash!");

    hash_table_free(table);
}

void test_free_null_table(void) {
    TEST("hash_table_free with NULL table");

    hash_table_free(NULL);
    ASSERT_TRUE(1, "Freeing NULL table should not crash!");
}

void test_large_dataset(void) {
    TEST("hash_table with large dataset");

    HashTable *table = hash_table_create(int_compare, int_hash);

    int num_elements = 1000;
    int *keys = malloc(sizeof(int) * (size_t)num_elements);
    int *values = malloc(sizeof(int) * (size_t)num_elements);

    for(int k = 0; k < num_elements; k++) {
        keys[k] = k;
        values[k] = k * 10;
        hash_table_put(table, &keys[k], &values[k]);
    }

    ASSERT_EQUALS(num_elements, hash_table_size(table), "All 1000 elements should be inserted!");

    // Verify random access
    for(int k = 0; k < num_elements; k += 100) {
        void *result = hash_table_get(table, &keys[k]);
        ASSERT_EQUALS((intptr_t)&values[k], (intptr_t)result, "Random access should work on large dataset!");
    }

    free(keys);
    free(values);
    hash_table_free(table);
}

void test_null_value(void) {
    TEST("hash_table with NULL value");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *key = "null_value_key";
    hash_table_put(table, key, NULL);

    ASSERT_EQUALS(1, hash_table_size(table), "NULL value should be insertble!");
    ASSERT_TRUE(hash_table_contains_key(table, key), "Key with NULL value should be present!");

    void *result = hash_table_get(table, key);
    ASSERT_NULL(result, "NULL value should be retrievable as NULL!");

    hash_table_free(table);
}

void test_int_keys_and_values(void) {
    TEST("Hash table with int keys and int values");

    HashTable *table = hash_table_create(int_compare, int_hash);
    ASSERT_NOT_NULL(table, "Table should be created!");

    int keys[] = {0, 10, 20, 30, -40, 50};
    int values[] = {-1, 100, -200, 300, 400, 500};

    for(int i = 0; i < 6; i++) {
        hash_table_put(table, &keys[i], &values[i]);
    }

    ASSERT_EQUALS(6, hash_table_size(table), "Size should be 5!");

    for(int i = 0; i < 6; i++) {
        int *result = (int *)hash_table_get(table, &keys[i]);
        ASSERT_NOT_NULL(result, "Value should be found!");
        ASSERT_EQUALS(values[i], *result, "Value should match!");
    }

    hash_table_free(table);
}

void test_int_update_value(void) {
    TEST("Update int value in hash table");

    HashTable *table = hash_table_create(int_compare, int_hash);

    int key = 42;
    int value1 = 100;
    int value2 = 200;

    hash_table_put(table, &key, &value1);
    int *result1 = (int *)hash_table_get(table, &key);
    ASSERT_EQUALS(value1, *result1, "First value should match!");

    hash_table_put(table, &key, &value2);
    int *result2 = (int *)hash_table_get(table, &key);
    ASSERT_EQUALS(value2, *result2, "Updated value should match!");
    ASSERT_EQUALS(1, hash_table_size(table), "Size should remain 1!");

    hash_table_free(table);
}

void test_double_keys_and_values(void) {
    TEST("Hash table with double keys and double values");

    HashTable *table = hash_table_create(double_compare, double_hash);
    ASSERT_NOT_NULL(table, "Table should be created!");

    double keys[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    double values[] = {10.5, 20.5, 30.5, 40.5, 50.5};

    for(int i = 0; i < 5; i++) {
        hash_table_put(table, &keys[i], &values[i]);
    }

    ASSERT_EQUALS(5, hash_table_size(table), "Size should be 5!");

    for(int i = 0; i < 5; i++) {
        double *result = (double *)hash_table_get(table, &keys[i]);
        ASSERT_NOT_NULL(result, "Value should be found!");
        ASSERT_DOUBLE_EQUALS(values[i], *result, "Value should match!");
    }

    hash_table_free(table);
}

void test_double_precision(void) {
    TEST("Hash table with high precision double values");

    HashTable *table = hash_table_create(double_compare, double_hash);

    double keys[] = {3.141592653589793, 2.718281828459045, 1.414213562373095};
    double values[] = {100.123456789, 200.987654321, 300.555555555};

    for(int i = 0; i < 3; i++) {
        hash_table_put(table, &keys[i], &values[i]);
    }

    for(int i = 0; i < 3; i++) {
        double *result = (double *)hash_table_get(table, &keys[i]);
        ASSERT_NOT_NULL(result, "Value should be found!");
        ASSERT_DOUBLE_EQUALS(values[i], *result, "High precision value should match!");
    }

    hash_table_free(table);
}

void test_string_keys_various_values(void) {
    TEST("Hash table with string keys and various value types");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *key1 = "name";
    char *key2 = "age";
    char *key3 = "city";

    char *value1 = "Mario Rossi";
    int value2 = 30;
    char *value3 = "Roma";

    hash_table_put(table, key1, value1);
    hash_table_put(table, key2, &value2);
    hash_table_put(table, key3, value3);

    ASSERT_EQUALS(3, hash_table_size(table), "Size should be 3!");

    char *result1 = (char *)hash_table_get(table, key1);
    ASSERT_TRUE(strcmp(result1, value1) == 0, "String value should match!");

    int *result2 = (int *)hash_table_get(table, key2);
    ASSERT_EQUALS(value2, *result2, "Int value should match!");

    char *result3 = (char *)hash_table_get(table, key3);
    ASSERT_TRUE(strcmp(result3, value3) == 0, "String value should match!");

    hash_table_free(table);
}

void test_string_long_keys(void) {
    TEST("Hash table with long string keys");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *long_key = "this_is_a_very_long_key_name_to_test_hash_function_performance";
    char *value = "test_value";

    hash_table_put(table, long_key, value);

    char *result = (char *)hash_table_get(table, long_key);
    ASSERT_NOT_NULL(result, "Value should be found!");
    ASSERT_TRUE(strcmp(result, value) == 0, "Value should match!");

    hash_table_free(table);
}

void test_string_empty_key(void) {
    TEST("Hash table with empty string key");

    HashTable *table = hash_table_create(string_compare, string_hash);

    char *empty_key = "";
    char *value = "empty_key_value";

    hash_table_put(table, empty_key, value);

    char *result = (char *)hash_table_get(table, empty_key);
    ASSERT_NOT_NULL(result, "Value should be found!");
    ASSERT_TRUE(strcmp(result, value) == 0, "Value should match!");

    hash_table_free(table);
}

void test_struct_person_keys(void) {
    TEST("Hash table with Person struct keys");

    HashTable *table = hash_table_create(person_compare, person_hash);

    Person p1 = {1, "Alice", 85.5};
    Person p2 = {2, "Bob", 90.0};
    Person p3 = {3, "Charlie", 78.5};

    char *grade1 = "B";
    char *grade2 = "A";
    char *grade3 = "C";

    hash_table_put(table, &p1, grade1);
    hash_table_put(table, &p2, grade2);
    hash_table_put(table, &p3, grade3);

    ASSERT_EQUALS(3, hash_table_size(table), "Size should be 3!");

    char *result1 = (char *)hash_table_get(table, &p1);
    ASSERT_TRUE(strcmp(result1, grade1) == 0, "Grade for Alice should match!");

    char *result2 = (char *)hash_table_get(table, &p2);
    ASSERT_TRUE(strcmp(result2, grade2) == 0, "Grade for Bob should match!");

    char *result3 = (char *)hash_table_get(table, &p3);
    ASSERT_TRUE(strcmp(result3, grade3) == 0, "Grade for Charlie should match!");

    hash_table_free(table);
}

void test_struct_person_values(void) {
    TEST("Hash table with Person struct values");

    HashTable *table = hash_table_create(int_compare, int_hash);

    int id1 = 101;
    int id2 = 102;
    int id3 = 103;

    Person p1 = {1, "Alice", 85.5};
    Person p2 = {2, "Bob", 90.0};
    Person p3 = {3, "Charlie", 78.5};

    hash_table_put(table, &id1, &p1);
    hash_table_put(table, &id2, &p2);
    hash_table_put(table, &id3, &p3);

    ASSERT_EQUALS(3, hash_table_size(table), "Size should be 3!");

    Person *result1 = (Person *)hash_table_get(table, &id1);
    ASSERT_EQUALS(1, result1->id, "Person id should match!");
    ASSERT_TRUE(strcmp(result1->name, "Alice") == 0, "Person name should match!");
    ASSERT_DOUBLE_EQUALS(85.5, result1->value, "Person value should match!");

    hash_table_free(table);
}

void test_struct_complex_scenario(void) {
    TEST("Complex scenario with mixed struct operations");

    HashTable *table = hash_table_create(person_compare, person_hash);

    Person people[] = {
        {1, "Alice", 85.5},
        {2, "Bob", 90.0},
        {3, "Charlie", 78.5},
        {4, "Diana", 92.5},
        {5, "Eve", 88.0}
    };

    char *grades[] = {"B", "A", "C", "A+", "B+"};

    for(int i = 0; i < 5; i++) {
        hash_table_put(table, &people[i], grades[i]);
    }

    ASSERT_EQUALS(5, hash_table_size(table), "Size should be 5!");

    // Update one grade
    char *new_grade = "A-";
    hash_table_put(table, &people[2], new_grade);
    ASSERT_EQUALS(5, hash_table_size(table), "Size should still be 5 after update!");

    char *result = (char *)hash_table_get(table, &people[2]);
    ASSERT_TRUE(strcmp(result, new_grade) == 0, "Updated grade should match!");

    // Remove one person
    hash_table_remove(table, &people[1]);
    ASSERT_EQUALS(4, hash_table_size(table), "Size should be 4 after removal!");
    ASSERT_FALSE(hash_table_contains_key(table, &people[1]), "Removed person should not be found!");

    hash_table_free(table);
}

int main(void) {
    printf(
        "\n"
        "+----------------------------------------+\n"
        "|         HASH TABLE UNIT TESTS          |\n"
        "+----------------------------------------+\n"
    );

    test_create_valid();
    test_create_invalid();

    test_put_and_get_simple();
    test_put_update_existing();
    test_put_multiple_keys();
    test_put_invalid_params();

    test_get_non_existent();
    test_get_invalid_params();

    test_contains_key();
    test_contains_key_invalid_params();

    test_remove_existing();
    test_remove_non_existent();
    test_remove_from_chain();
    test_remove_invalid_params();

    test_size();
    test_size_invalid_params();

    test_keyset();
    test_keyset_empty();
    test_keyset_invalid_params();

    test_resize();
    test_collision_handling();
    test_empty_table_operations();
    test_free_null_table();
    test_large_dataset();
    test_null_value();

    test_int_keys_and_values();

    test_double_keys_and_values();
    test_double_precision();

    test_string_keys_various_values();
    test_string_long_keys();
    test_string_empty_key();

    test_struct_person_keys();
    test_struct_person_values();
    test_struct_complex_scenario();

    printf(
        "\n"
        "+----------------------------------------+\n"
        "|          ALL TESTS PASSED!             |\n"
        "+----------------------------------------+\n"
        "\n"
    );

    return 0;
}
