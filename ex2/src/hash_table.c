#include "hash_table.h"

#include <stdlib.h>

/**
 * @brief Used as first value to allocates the buckets
 * array when the HashTable is created.
 */
#define HT_INITIAL_BUCKETS 23

/**
 * @brief Used as threshold value to increment the
 * HashTable buckets array capacity.
 */
#define HT_LOAD_FACTOR .75f

/**
 * @brief Hash table node used for separate chaining.
 * Each node stores a (key, value) pair and pointer to the next node
 * in the same bucket.
 */
typedef struct HashBucket {
    void *key;
    void *value;
    struct HashBucket *next;
} HashBucket;

/**
 * @brief Generic hash table using separate chaining.
 * The table stores pointers to keys and values (no deep copies).
 * A user-provided compare function and hash function are used
 * to handle arbitrary key types
 */
typedef struct HashTable {
    HashBucket **buckets;
    int capacity;
    int size;
    int (*compare)(const void *a, const void *b);
    unsigned long (*hash)(const void *key);
} HashTable;

/** 
 * @brief Compute the bucket index for a given key.
 * Uses the table hash function and maps the result into [0, num_buckets -1].
 * 
 * @param table The HashTable that contains the buckets array.
 * @param key The key value used to compute the index.
 * 
 * @return The key buckets array index.
 */
static int get_bucket_index(const HashTable *table, const void *key) {
    unsigned long hash = table->hash(key);
    return (int)(hash % (unsigned long)table->capacity);
}

/**
 * @brief Increase the HashTable buckets array capacity.
 * 
 * @param table The HashTable where to resize the buckets array.
 * 
 * @return 1 on success, 0 on error.
 */
static int hash_table_resize(HashTable *table) {
    int new_capacity = table->capacity * 2;

    if(new_capacity < table->capacity) {
        return 0;
    }

    HashBucket **new_buckets = (HashBucket **)calloc((size_t)new_capacity, sizeof(HashBucket *));
    if(!new_buckets) {
        return 0;
    }

    for(int k = 0; k < table->capacity; k++) {
        HashBucket *bucket = table->buckets[k];
        while(bucket) {
            HashBucket *next = bucket->next;
            int new_index = (int)(table->hash(bucket->key) % (unsigned long)new_capacity);

            bucket->next = new_buckets[new_index];
            new_buckets[new_index] = bucket;

            bucket = next;
        }
    }

    free(table->buckets);
    table->buckets = new_buckets;
    table->capacity = new_capacity;

    return 1;
}

HashTable *hash_table_create(int (*f1)(const void*, const void*), unsigned long (*f2)(const void*)) {
    if(!f1 || !f2 ) {
        return NULL;
    }

    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    if(!table) {
        return NULL;
    }

    table->buckets = (HashBucket **)calloc((size_t)HT_INITIAL_BUCKETS, sizeof(HashBucket *));
    if(!table->buckets) {
        free(table);
        return NULL;
    }

    table->capacity = HT_INITIAL_BUCKETS;
    table->size = 0;
    table->compare = f1;
    table->hash = f2;

    return table;
}

void hash_table_put(HashTable *table, const void *key, const void *value) {
    if(!table || !key) {
        return;
    }

    if(table->size >= (int)(HT_LOAD_FACTOR * (float)table->capacity)) {
        if(!hash_table_resize(table)) {
            return;
        }
    }

    int index = get_bucket_index(table, key);
    HashBucket *bucket = table->buckets[index];
    while(bucket) {
        if(table->compare(bucket->key, key) == 0) {
            bucket->value = (void *)value;
            return;
        }
        bucket = bucket->next;
    }

    HashBucket *new_bucket = (HashBucket *)malloc(sizeof(HashBucket));
    if(!new_bucket) {
        return; 
    }

    new_bucket->key = (void *)key;       
    new_bucket->value = (void *)value;
    new_bucket->next = table->buckets[index];

    table->buckets[index] = new_bucket;
    table->size++;
}

void *hash_table_get(const HashTable *table, const void *key) {
    if(!table || !key ) { 
        return NULL; 
    }

    int index = get_bucket_index(table, key); 
    HashBucket *bucket = table->buckets[index]; 
    while(bucket) {
        if(table->compare(bucket->key, key) == 0) {
            return bucket->value; 
        }
        bucket = bucket->next; 
    }

    return NULL; 
}

int hash_table_contains_key(const HashTable *table, const void *key) {
    if(!table || !key) {
        return 0;
    }

    int index = get_bucket_index(table, key); 
    HashBucket *bucket = table->buckets[index]; 
    while(bucket) {
        if(table->compare(bucket->key, key) == 0) {
            return 1;
        }
        bucket = bucket->next;
    }

    return 0;
}

void hash_table_remove(HashTable *table, const void *key) {
    if(!table || !key) {
        return;
    }

    int index = get_bucket_index(table, key); 
    HashBucket *curr = table->buckets[index];
    HashBucket *prev = NULL;
    while(curr) {
        if(table->compare(curr->key, key) == 0) {
            if(!prev) {
                table->buckets[index] = curr->next;
            } else {
                prev->next = curr->next;
            }

            free(curr);
            table->size--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

int hash_table_size(const HashTable *table) {
    if(!table) {
        return 0;
    }

    return table->size;
}

void **hash_table_keyset(const HashTable *table) {
    if(!table) {
        return NULL;
    }

    int table_size = hash_table_size(table);
    void **keyset = (void **)malloc(sizeof(void *) * (size_t)(table_size + 1));
    if(!keyset) {
        return NULL;
    }

    int keyset_count = 0;
    for(int k = 0; k < table->capacity; k++) {
        HashBucket *bucket = table->buckets[k];

        while(bucket) {
            keyset[keyset_count++] = bucket->key;
            bucket = bucket->next;
        }
    }

    keyset[keyset_count] = NULL;

    return keyset;
}

void hash_table_free(HashTable *table) {
    if(!table) {
        return;
    }

    for(int k = 0; k < table->capacity; k++) {
        HashBucket *bucket = table->buckets[k];

        while(bucket) {
            HashBucket *tmp = bucket->next;
            free(bucket);
            bucket = tmp;
        }
    }

    free(table->buckets);
    free(table);
}
