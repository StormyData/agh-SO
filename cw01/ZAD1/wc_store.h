#pragma once
#include <stddef.h>

struct allocation_table {
    size_t real_size;
    size_t size;
    char **table;
};


struct allocation_table *create_allocation_table();

const char *count_words_lines_chars(char *file_path);

int store_file(struct allocation_table *table,const char *file_path);

int free_block(struct allocation_table *table, size_t index);

void free_allocation_table(struct allocation_table *table);