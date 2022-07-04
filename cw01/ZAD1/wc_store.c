#include <unistd.h>
#include "wc_store.h"
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

struct allocation_table *create_allocation_table() {
    struct allocation_table *table = malloc(sizeof(struct allocation_table));
    table->size = 0;
    table->real_size = 5;
    table->table = malloc(5 * sizeof(char *));
    return table;
}

const char *count_words_lines_chars(char *file_path) {

    char* tmp_file_path = "tmpfile";//TODO:
    char* format = "echo \"$(wc -w %1$s)\" ' ' \"(wc -l %1$s)\" ' ' \"wc -c %1$s\" >> %2$s";

    char* command = malloc(strlen(format) + 3 * strlen(file_path) + strlen(tmp_file_path)  + 1);
    sprintf(command,format,file_path,tmp_file_path);
    system(command);
    free(command);
    return tmp_file_path;
}

int store_file(struct allocation_table *table,const char *file_path) {

    int fd = open(file_path, O_RDONLY);
    struct stat buff;
    fstat(fd, &buff);
    char *file_buff = malloc(buff.st_size);
    read(fd, file_buff, buff.st_size);
    close(fd);

    if (table->size + 1 > table->real_size) {
        size_t new_size = (size_t)(table->real_size * 1.2) + 1;
        char **new_table_ptr = realloc(table->table, new_size * sizeof(char *));
        if (!new_table_ptr)
            return -1;
        table->table = new_table_ptr;
        table->real_size = new_size;
    }
    table->table[table->size] = file_buff;
    table->size++;
    return table->size - 1;
}

/*
 * free_block frees a block from the table, and shifts the table back
 * */
int free_block(struct allocation_table *table, size_t index) {
    if (index >= table->size) {
        return -1;
    }
    free(table->table[index]);
    for (int i = index; i < table->size - 1; i++) {
        table->table[i] = table->table[i + 1];
    }
    table->size--;
    return 0;
}

void free_allocation_table(struct allocation_table *table) {
    for (int i = 0; i < table->size;i++) {
        free(table->table[i]);
    }
    free(table);
}