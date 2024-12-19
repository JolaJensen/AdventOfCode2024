#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FREE_SPACE_BLOCK -1

char *read_disk_map(const char *filename, size_t *dm_length) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    size_t chunk_size = 1024;
    size_t buffer_size = chunk_size;
    *dm_length = 0;

    char *buffer = (char *) malloc (buffer_size);
    if (!buffer) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    size_t bytes_read;
    while ((bytes_read = fread(buffer + *dm_length, 1, chunk_size, file)) > 0) {
        *dm_length += bytes_read;
        if (*dm_length + chunk_size >= buffer_size) {
            buffer_size *= 2;
            char *new_buffer = (char *) realloc (buffer, buffer_size);
            if (!new_buffer) {
                perror("Error reallocating memory");
                free(buffer);
                fclose(file);
                return NULL;
            }
            buffer = new_buffer;
        }
    }

    if (ferror(file)) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[*dm_length] = '\0';
    fclose(file);

    /* Disregard free space after last data block if given (by mistake). */
    if (*dm_length % 2 == 0) {
        *dm_length = *dm_length - 1;
    }

#ifdef DEBUG
    printf("%zu\n", *dm_length);
#endif

    return buffer;
}

int *convert_to_digits(const char *disk_map, size_t dm_length, size_t *count) {
    int *dm_in_digits = (int *) malloc (dm_length * sizeof(int));
    if (!dm_in_digits) {
        perror("Error allocating memory");
        return NULL;
    }

    *count = 0;
    for (size_t i = 0; i < dm_length; i++) {
        char c = disk_map[i];
        int digit;
        sscanf(&c, "%d", &digit);
        dm_in_digits[i] = digit;
        *count += digit;
    }

#ifdef DEBUG
    for (int i = 0; i < dm_length; i++) {
        printf("%d", dm_in_digits[i]);
    }
    printf("\n");
    printf("%zu\n", *count);
#endif

    return dm_in_digits; 
}

int *file_compact(int *dm_in_digits, size_t dm_length, size_t count) {
    int *individual_blocks = (int *) malloc (count * sizeof(int));
    if (!individual_blocks) {
        perror("Error allocating memory");
        return NULL;
    }

    size_t ID = 0;
    size_t index_dm = 0;
    size_t index_ib = 0;

    while (index_dm < dm_length && index_ib < count) {
        int i = dm_in_digits[index_dm];
        int j = index_ib + i;
        for (; index_ib < j && index_ib < count; index_ib++) {
            individual_blocks[index_ib] = ID;
        }
        index_dm++;
        if (index_dm < dm_length) {
            i = dm_in_digits[index_dm];
            j = index_ib + i;
            for (; index_ib < j && index_ib < count; index_ib++) {
                individual_blocks[index_ib] = FREE_SPACE_BLOCK;
            }
            ID++;
            index_dm++;
        }
    }

#ifdef DEBUG
    for (size_t i = 0; i < count; i++) {
        printf("%d", individual_blocks[i]);
    }
    printf("\n");
#endif

    long int first = 0;
    long int last = count - 1;

    while (first < last) {
        while (individual_blocks[first] != FREE_SPACE_BLOCK && first < last) {
            first++;
        }
        while (individual_blocks[last] == FREE_SPACE_BLOCK && first < last) {
            last--;
        }
        if (first < last) {
            individual_blocks[first] = individual_blocks[last];
            individual_blocks[last] = FREE_SPACE_BLOCK;
            first++;
            last--;
        }
    }

#ifdef DEBUG
    printf("%ld %ld\n", first, last);

    for (size_t i = 0; i < count; i++) {
        printf("%d", individual_blocks[i]);
    }
    printf("\n");
#endif

    return individual_blocks;
}

unsigned long long compute_checksum(int *individual_blocks, size_t count) {
    unsigned long long checksum = 0;
    size_t i;
    
    for (i = 0; i < count && individual_blocks[i] != FREE_SPACE_BLOCK; i++) {
        checksum += (unsigned long long)i * individual_blocks[i];
    }

#ifdef DEBUG
    printf("%zu %d %d\n", i, individual_blocks[i - 1], individual_blocks[i]);
    printf("%llu\n", checksum);
#endif

    return checksum;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t dm_length = 0;
    char *disk_map = read_disk_map(argv[1], &dm_length); 

    if (!disk_map) {
        return EXIT_FAILURE;
    }

    size_t count = 0;
    int *dm_in_digits = convert_to_digits(disk_map, dm_length, &count);

    if (!dm_in_digits) {
        return EXIT_FAILURE;        
    }

    int *individual_blocks = file_compact(dm_in_digits, dm_length, count);

    if (!individual_blocks) {
        return EXIT_FAILURE;
    }

    unsigned long long checksum = compute_checksum(individual_blocks, count);
    printf("%llu\n", checksum);

    free(disk_map);
    free(dm_in_digits);
    free(individual_blocks);

    return EXIT_SUCCESS;
}