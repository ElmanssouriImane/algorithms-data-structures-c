#ifndef __RECORD_UTILS_H__
#define __RECORD_UTILS_H__

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#define FLD_3_LEN 16

#pragma pack(push, 1)

/**
 * @brief Record struct (36 byte without padding).
 * - Byte  0- 7: unsigned 64-bit
 * - Byte  8-11: float
 * - Byte  8-11: float
 * - Byte 12-19: signed 64-bit
 * - Byte 20-35: string 16 char (null-terminated)
 */
typedef struct {
    uint64_t id;
    float field1;
    int64_t field2;
    char field3[FLD_3_LEN];
} Record;

#pragma pack(pop)

/**
 * @brief Enum used to select the field compare function
 */
typedef enum {
    FLD_ID=1, FLD_1=2, FLD_2=3, FLD_3=4
} RecordField;

/**
 * @brief Calculate the number of records in the file.
 *
 * This function uses fseek() to move the file position indicator
 * to the end of the file, then ftell() to obtain the file size
 * in bytes. The size is divided by sizeof(Record) to compute
 * the number of records stored in the file.
 * Before returning, the file position indicator is repositioned
 * at the beginning of the file using rewind(), ensuring that
 * subsequent read operations start from the beginning.
 *
 * @param file File opened in binary mode.
 *
 * @return The number of records of type Record.
 *         Returns 0 if the file is empty or an error occurs.
 */
size_t num_of_record(FILE *file);

/**
 * @brief Read a single indexed record from a binary file at index i.
 * 
 * This function uses fseek() to set the file position indicator at the
 * specified index, then fread() to get the record value.
 * 
 * @param file File opened in binary mode.
 * @param i Record index in the binary file.
 * @param record Pointer to the record to fill.
 * 
 * @return 0 on success, -1 on EOF or error
 */
int read_record(FILE *file, size_t i, Record *record);

/**
 * @brief Write a single record into a binary file.
 * 
 * @param file File opened in binary (append) mode.
 * @param record Pointer to the record to write.
 * 
 * @return 0 on success, -1 on EOF or error.
 */
int write_record(FILE *file, const Record *record);

/**
 * @brief Load all records from file into memory.
 * 
 * @param file File opened in binary mode.
 * @param records Pointer to the record array where to load data.
 * @param len Record array length.
 * 
 * @return 0 on success, -1 on error.
 */
int load_records_from_file(FILE *file, Record *records, size_t len);

/**
 * @brief Save all records into the binary file.
 * 
 * @param file File opened in binary mode.
 * @param records Record array to write.
 * @param len Record array length.
 * 
 * @return 0 on success, -1 on error.
 */
int save_records_to_file(FILE *file, const Record *records, size_t count);

/**
 * @brief Print record data to stdout.
 * 
 * @param record Record pointer to print
 */
void print_record(const Record *record);

/**
 * @brief Record fields compare functions. Compatible with standard qsort().
 * 
 * @param a First element.
 * @param b second element.
 * 
 * @return A value < 0 if a < b, a value > 0 if a > b, 0 if a == b.
 */
int compare_id(const void *a, const void *b);
int compare_field1(const void *a, const void *b);
int compare_field2(const void *a, const void *b);
int compare_field3(const void *a, const void *b);

/**
 * @brief Returns the compare function for specified field.
 * 
 * @param field Value used to select the compare function.
 * 
 * @return Function pointer to the selected compare function.
 */
int (*get_compare_function(RecordField field))(const void *, const void*);

#endif // __RECORD_UTILS_H__
