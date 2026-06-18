#include "record_utils.h"

#include <string.h>
#include <inttypes.h>

size_t num_of_record(FILE *file) {
    size_t num;
    long ftell_ret;

    if(!file) {
        return 0;
    }

    if(fseek(file, 0, SEEK_END) != 0) {
        return 0;
    }

    if((ftell_ret = ftell(file)) == -1L) {
        return 0;
    }

    num = (size_t)ftell_ret / sizeof(Record);

    rewind(file);

    return num;
}

int read_record(FILE *file, size_t i, Record *record) {    
    if(!file || !record) {
        return -1;
    }

    if(fseek(file, (long)(i * sizeof(Record)), SEEK_SET) != 0) {
        return -1;
    }

    if(fread(record, sizeof(Record), 1, file) != 1) {
        return -1;
    }

    return 0;
}

int write_record(FILE *file, const Record *record) {
    if(!file || !record) {
        return -1;
    }

    if(fwrite(record, sizeof(Record), 1, file) != 1) {
        return -1;
    }

    return 0;
}

int load_records_from_file(FILE *file, Record *records, size_t len) {
    if(!file || !records) {
        return -1;
    }

    if(fread(records, sizeof(Record), len, file) != len) {
        return -1;
    }

    return 0;
}

int save_records_to_file(FILE *file, const Record *records, size_t len) {
    if(!file || !records) {
        return -1;
    }

    if(fwrite(records, sizeof(Record), len, file) != len) {
        return -1;
    }

    return 0;
}

void print_record(const Record *record) {
    if(!record) {
        return;
    }

    printf(
        "ID: %" PRIu64 " FD1: %.2f FD2: %" PRId64 " FD3: %s\n",
        record->id,
        record->field1, record->field2, record->field3
    );
}

int compare_id(const void *a, const void *b) {
    const Record *ra = (const Record *)a;
    const Record *rb = (const Record *)b;

    if(ra->id < rb->id) {
        return -1;
    } else if(ra->id > rb->id) {
        return 1;
    }

    return 0;
}

int compare_field1(const void *a, const void *b) {
    const Record *ra = (const Record *)a;
    const Record *rb = (const Record *)b;

    if(ra->field1 < rb->field1) {
        return -1;
    } else if(ra->field1 > rb->field1) {
        return 1;
    }

    return 0;
}

int compare_field2(const void *a, const void *b) {
    const Record *ra = (const Record *)a;
    const Record *rb = (const Record *)b;

    if(ra->field2 < rb->field2) {
        return -1;
    } else if(ra->field2 > rb->field2) {
        return 1;
    }

    return 0;
}

int compare_field3(const void *a, const void *b) {
    const Record *ra = (const Record *)a;
    const Record *rb = (const Record *)b;

    return memcmp(ra->field3, rb->field3, FLD_3_LEN * sizeof(char));
}

int (*get_compare_function(RecordField field))(const void *, const void*) {
    switch(field) {
        case FLD_ID:
            return compare_id;
            break;

        case FLD_1:
            return compare_field1;
            break;

        case FLD_2:
            return compare_field2;
            break;

        case FLD_3:
            return compare_field3;
            break;

        default:
            return NULL;
    }
}
