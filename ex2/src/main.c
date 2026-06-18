#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "hash_table.h"

#define ARGC_COUNT 3

#define MAX_LINE_LEN 1024

/**
 * @brief String hash function used as an example.
 * 
 * @param key The string to be hashed.
 * 
 * @return The string hash.
 */
unsigned long string_hash(const void *key) {
    const unsigned char* str = key;
    unsigned long hash = 5381;
    int c;

    while((c = *str++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }

    return hash;
}

/**
 * @brief Strings compare function. Compatible with standard qsort().
 * 
 * @param a First element.
 * @param b Second element.
 * 
 * @return A value < 0 if a < b, a value > 0 if a > b, 0 if a == b.
 */
int string_compare(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

/**
 * @brief Converts all alphanumeric character of the given line in uppercase
 * and replaces all other character with a space, then splits the words
 * and puts it into the given HashTable. It considers words with minimum
 * length specified.
 * 
 * @param line The string to convert.
 * @param table The hash table where to put the words.
 * @param min_word_len Minimum word length to consider
 */
void parse_words(char *line, HashTable *table, int min_word_len) {
    for(char *c = line; *c; c++) {
        unsigned char uc = (unsigned char)*c;

        if(!isalnum((int)uc)) {
            *c = ' ';
        } else if(isupper((int)uc)) {
            *c = (char)tolower((int)uc);
        }
    }

    char *token = strtok(line, " ");
    while(token) {
        if(strlen(token) >= (size_t)min_word_len) {
            size_t *count = (size_t *)hash_table_get(table, token);
            if(!count) {
                char *key = strdup(token);
                count = calloc(1, sizeof(size_t)); 

                hash_table_put(table, key, count);
            }
            
            (*count)++;
        }

        token = strtok(NULL, " ");
    }
}

/**
 * @brief This function analyzes the hash table to identify the word that appears
 * the most times across all buckets. It prints the resulting word to the screen
 * along with its frequency.
 *
 * @param table Pointer to the hash table to analyze.
 */
void count_max_freq(HashTable *table) {
    void **keys = hash_table_keyset(table);
    size_t max_count;
    char *max_key;

    if(!keys) {
        fprintf(stderr, "Error: Cannot retrieve HashTable keys!\n");
        return;
    }

    if(!keys[0]) {
        fprintf(stderr, "Error: The hash table is empty!\n");
        free(keys);
        return;
    }

    max_key = keys[0];
    max_count = *(size_t *)hash_table_get(table, keys[0]);;

    for(size_t k = 1; keys[k] != NULL; k++) {
        size_t value = *(size_t *)hash_table_get(table, keys[k]);

        if(value > max_count) {
            max_key = keys[k];
            max_count = value;
        }
    }

    printf("%s -> %zu\n", max_key, max_count);

    free(keys);
}

/**
 * @brief This function completely deallocates the hash table content by first
 * retrieving all keys using hash_table_keyset(), then iterating through each key to:
 * - Retrieve the corresponding value (frequency counter);
 * - Remove the key-value bucket from the table;
 * - Free the key string and value memory.
 * Finally, it frees the keyset array..
 *
 * @param table Pointer to the HashTable to free.
 */
void free_hash_table_content(HashTable *table) {
    void **keys = hash_table_keyset(table);
    for(size_t k = 0; keys[k] != NULL; k++) {
        size_t *value = (size_t *)hash_table_get(table, keys[k]);
        hash_table_remove(table, keys[k]);

        free(keys[k]);
        free(value);
    }

    free(keys);
}

/**
 * @brief This function attempts to convert the input string into an integer.
 * The result is stored in the variable pointed to by value.
 *
 * @param s Pointer to the null-terminated string to convert.
 * @param value Pointer to an integer where the converted value will be stored.
 *
 * @return Returns 1 if the conversion is successful, 0 if the string is not a valid integer.
 */
int string_to_int(const char *s, int *value) {
    char *endptr;

    *value = (int)strtol(s, &endptr, 0);

    /**
     * If endptr == s then the string starts with a non-numeric character ("", "ABC", "A123");
     * If endptr != [\0, \\n] then the string contains non-numeric characters ("1A", "123ABC4")
    */
    if(endptr == s || (*endptr != '\0' && *endptr != '\n')) {
        return 0;
    }

    return 1;
}

/**
 * @brief This function prints the main_ex2 synopsis.
 * 
 * @param program_name The name of the executable (usually argv[0])
 */
void print_synopsis(const char *program_name) {
    fprintf(
        stderr,
        "Usage: %s <input_file> <word_length>\n\n",
        program_name
    );
}

/**
 * @brief Program entry point used to read the text file and
 * count the words occurrences.
 *
 * Usage:
 *   ./bin/main_ex2 <input_file> <word_length>
 *
 * - input_file:  Path to input file.
 * - word_length: Minimum word length to consider.
 */
int main(int argc, char *argv[]) {
    char line[MAX_LINE_LEN];
    int min_len;

    if(argc != ARGC_COUNT) {
        print_synopsis(argv[0]);
        return 1;
    }

    const char *in_file_name = argv[1];

    FILE *file = fopen(in_file_name, "rt");
    if(!file) {
        fprintf(stderr, "Error: Could not open input file '%s' file!\n\n", in_file_name);
        return 1;
    }

    if(!string_to_int(argv[2], &min_len) || min_len < 0) {
        fprintf(stderr, "Error: The min_length value must be a positive integer!\n\n");
        fclose(file);
        return 1;
    }

    HashTable *table = hash_table_create(string_compare, string_hash);
    if(!table) {
        fprintf(stderr, "Error: Could not create the hash table!\n\n");
        fclose(file);
        return 1;
    }

    while(fgets(line, MAX_LINE_LEN, file)) {
        parse_words(line, table, min_len);
    }
    fclose(file);

    count_max_freq(table);

    free_hash_table_content(table);
    hash_table_free(table);

    return 0;
}
