#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hybridsort.h"
#include "record_utils.h"

#define ARGC_COUNT 5

/**
 * @brief This function attempts to convert the input string s into an integer.
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
 * @brief This function prints the main_ex1 synopsis.
 * 
 * @param program_name The name of the executable (usually argv[0])
 */
void print_synopsis(const char *program_name) {
    fprintf(
        stderr,
        "Usage: %s <input_file> <output_file> <field> <k>\n\n"

        "\tfield: 1 = id | 2 = field1 | 3 = field2 | 4 = field3\n"
        "\tk:     Threshold for hybrid algorithm\n\n",
        program_name
    );
}

/**
 * @brief Program entry point used to sort all records from the binary file.
 *
 * Usage:
 *   ./bin/main_ex1 <input_file> <output_file> <field> <k>
 *
 * - input_file:   Path to input file.
 * - output_file:  Path to output file.
 * - field:        Selected field used to order the records.
 *   - 1 = id
 *   - 2 = field1
 *   - 3 = field2
 *   - 4 = field3
 * - k:            Threshold used to select the sorting algorithm.
 */
int main(int argc, char *argv[]) {
    FILE *in_file, *out_file;

    size_t num_recs;
    Record *records;

    RecordField selected_field;
    int k_thrs;
    int (*compare)(const void *, const void *);

    clock_t tstart, tend;
    double elapsed;

    if(argc != ARGC_COUNT) {
        print_synopsis(argv[0]);
        return 1;
    }

    const char *in_file_name = argv[1];
    const char *out_file_name = argv[2];

    in_file = fopen(in_file_name, "rb");
    if(!in_file) {
        fprintf(stderr, "Error: Could not open input file '%s' file!\n\n", in_file_name);
        return 1;
    }

    num_recs = num_of_record(in_file);
    if(num_recs == 0) {
        fprintf(stderr, "Error: The input file '%s' is empty or does not exist!\n\n", in_file_name);

        fclose(in_file);
        return 1;
    }

    records = (Record *)malloc(sizeof(Record) * num_recs);
    if(!records) {
        fprintf(stderr, "Error: Could not allocate memory!\n\n");

        fclose(in_file);
        return 1;
    }

    if(load_records_from_file(in_file, records, num_recs) != 0) {
        fprintf(stderr, "Error: Could not load all records into memory!\n\n");

        free(records);
        fclose(in_file);
        return 1;
    }

    fclose(in_file);

    if(!string_to_int(argv[3], (int *)&selected_field)) {
        fprintf(stderr, "Error: Please specify an integer filed:\n\n");
        print_synopsis(argv[0]);

        free(records);
        return 1;
    }

    compare = get_compare_function(selected_field);
    if(!compare) {
        fprintf(stderr, "Error: invalid field number:\n\n");
        print_synopsis(argv[0]);

        free(records);
        return 1;
    }

    if(!string_to_int(argv[4], &k_thrs) || k_thrs < 0) {
        fprintf(stderr, "Error: Please specify a positive integer k value:\n\n");
        print_synopsis(argv[0]);

        free(records);
        return 1;
    }

    // Test  hybridsort (with timer)
    printf("Sorting records with k=%d (field=%d)\n", k_thrs, selected_field);

    tstart = clock();
    hybridsort(records, num_recs, sizeof(Record), (size_t)k_thrs, compare);
    tend = clock();

    // Calculate the elapsed time
    elapsed = (double)(tend - tstart) / CLOCKS_PER_SEC;

    printf("Sorting completed in %f\n\n", elapsed);

    out_file = fopen(out_file_name, "wb");
    if(!out_file) {
        fprintf(stderr, "Error: Could not open output file '%s' file!\n\n", out_file_name);

        free(records);
        return 1;
    }

    if(save_records_to_file(out_file, records, num_recs) != 0) {
        fprintf(stderr, "Error: Could not save sorted records into the '%s' file!\n\n", out_file_name);

        fclose(out_file);
        free(records);
        return 1;
    }

    fclose(out_file);
    free(records);

    return 0;
}
