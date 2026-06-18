#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

/**
 * @brief Hash table with opaque structure.
 * 
 * Implementation details are hidden. Use the library's public functions
 * to interact with this structure.
 */
typedef struct HashTable HashTable;

/**
 * @brief Create a new empty hash table. 
 *
 * @param f1 key comparison function (must return 0 when keys are equal).
 * @param f2 Hash function mapping a key to an unsigned long.
 * 
 * @return Pointer to a newly allocated Hashtable, or NULL or error. 
 */
HashTable *hash_table_create(int (*f1)(const void *, const void *),unsigned long (*f2)(const void *));

/**
 * @brief Insert or update a (key, value) pair.
 * 
 * If the key already exists, its associated value is replaced with the new one.
 * Keys and values are stored as raw pointers; no copies are made and the hash
 * table does not take ownership of the memory they point to.
 * If the number of stored elements exceeds the load factor threshold,
 * the table is resized exponentially and all bucket indices are recalculated
 * accordingly.
 *
 * @param table Hash table instance 
 * @param key Pointer to the key to insert.
 * @param value Pointer to the value to associate with the key.
 * 
 * @warning When updating an existing key, the old value pointer is lost and 
 * cannot be freed by the HashTable. To avoid memory leaks when updating, 
 * the caller must first retrieve the old value with hash_table_get(), free it,
 * and only then call hash_table_put() with the new value. Alternatively, use
 * hash_table_remove() before inserting the new pair.
 */
void hash_table_put(HashTable *table, const void *key, const void *value);

/**
 * @brief Get the value associated with the key 
 *
 * @param table Hash table instance
 * @param key Pointer to the key to search for.
 * 
 * @return Pointer to the associated value, or NULL if the key is not present.
 */
void *hash_table_get(const HashTable *table, const void *key);

/**
 * @brief Check if a key is present in the table.
 *
 * @param table Hash table instance
 * @param Key Pointer to the key to search for.
 * 
 * @return 1 if the key exists, 0 otherwise 
 */
int hash_table_contains_key(const HashTable *table, const void *key);

/**
 * @brief Remove the entry with the given key, if it exists.
 *
 * @param table Hash table instance.
 * @param Key Pointer to the key to remove.
 */
void hash_table_remove(HashTable *table, const void *key);

/**
 * @brief Get the number of entries stored in the table.
 *
 * @param table Hash table instance. 
 * 
 * @return Number of (key, value) pairs, or 0 if table is NULL.
 */
int hash_table_size(const HashTable *table);

/**
 * @brief Get an array containing all keys in the table.
 * The returned array is terminated by a NULL pointer and must be freed by the caller using free().
 *
 * @param table Hash table instance.
 * 
 * @return Dynamically allocated array of key pointers, or NULL on error.
 */
void **hash_table_keyset(const HashTable *table);

/**
 * @brief Free all memory used by the hash table.
 * This function frees all buckets, the internal bucket array and the HashTable structure itself
 *
 * @param table Hash table instance to destroy
 * 
 * @warning The caller MUST free all keys and values before calling this function.
 * Use hash_table_keyset() to retrieve all keys, then iterate through them to get
 * the associated values with hash_table_get(), and free both keys and values before
 * calling hash_table_free(). Failure to do so will result in memory leaks. 
 */
void hash_table_free(HashTable *table);

#endif //__HASH_TABLE_H__
