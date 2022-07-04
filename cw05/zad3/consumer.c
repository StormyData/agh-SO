#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <string.h>

struct line_vector {
    size_t *sizes;
    char **data;
    size_t curr_size;
    size_t max_size;
};

void line_vector_init(struct line_vector *vector, size_t init_size) {
    vector->sizes = malloc(init_size * sizeof(size_t));
    vector->data = malloc(init_size * sizeof(char *));
    vector->curr_size = 0;
    vector->max_size = init_size;
    for (int i = 0; i < init_size; i++) {
        vector->sizes[i] = 0;
        vector->data[i] = NULL;
    }
}

void line_vector_free(struct line_vector *vector) {
    free(vector->sizes);
    free(vector->data);
}

void line_vector_expand(struct line_vector *vector) {
    size_t new_size = vector->max_size * 2;
    vector->sizes = realloc(vector->sizes, new_size * sizeof(size_t));
    vector->data = realloc(vector->data, new_size * sizeof(char *));
    for (size_t i = vector->max_size; i < new_size; i++) {
        vector->sizes[i] = 0;
        vector->data[i] = NULL;
    }
    vector->max_size = new_size;
}

void line_vector_append(struct line_vector *vector, char *data, size_t data_size) {
    if (vector->curr_size == vector->max_size)
        line_vector_expand(vector);
    vector->sizes[vector->curr_size] = data_size;
    vector->data[vector->curr_size] = data;
    vector->curr_size++;
}

//void write_to_line(char *file_name, char *data, size_t size, int line_num) {
//    FILE *file = fopen(file_name, "r+");
//
//    flock(fileno(file),LOCK_EX);
//    fseek(file, 0, SEEK_SET);
//    struct line_vector vector;
//
//
//    line_vector_init(&vector, line_num + 1);
//    size_t n = 0;
//    char *line = NULL;
//
//    while (getline(&line, &n, file) > 0) {
//
//        line_vector_append(&vector, line, strlen(line));
//        line = NULL;
//        n = 0;
//    }
//    if(vector.curr_size < line_num + 1)
//        vector.curr_size = line_num + 1;
//    vector.data[line_num] = realloc(vector.data[line_num], vector.sizes[line_num] + size + 1);
//    memcpy(vector.data[line_num] + vector.sizes[line_num], data, size);
//    vector.sizes[line_num] += size;
//
//    fseek(file, 0, SEEK_SET);
//    for (int i = 0; i < vector.curr_size; i++) {
//        if (vector.sizes[i] > 0) {
//            fwrite(vector.data[i], 1, vector.sizes[i], file);
//            free(vector.data[i]);
//        }
//        else
//        {
//            fwrite("\n",1,1,file);
//        }
//    }
//    fflush(file);
//    line_vector_free(&vector);
//    flock(fileno(file),LOCK_UN);
//    fclose(file);
//}
void write_to_line(char *file_name, char *data, size_t size, int line_num) {
    char* fname[strlen(file_name) + 512];
    sprintf(fname,"%s%d",file_name,line_num);
    FILE *file = fopen(fname, "a");
    flock(fileno(file),LOCK_EX);
    fwrite(data, 1, size, file);
    flock(fileno(file),LOCK_UN);
    fclose(file);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("this program requires 3 arguments, path to fifo, write file, N");
        exit(-1);
    }
    int n = atoi(argv[3]);
    FILE *named_fifo = fopen(argv[1], "r");
    size_t bytes_read = 0;
    char *buffer = malloc(sizeof(int) + n);
    while (1) {
        bytes_read = fread(buffer, 1, n + sizeof(int), named_fifo);
        if (bytes_read < sizeof(int))
            break;
        //printf("%d ", *((int *) buffer));
        //fwrite(buffer + sizeof(int), 1, bytes_read - sizeof(int), stdout);
        //printf("\n");

        fflush(stdout);

        int row_num = *((int *) buffer);

        write_to_line(argv[2], buffer + sizeof(int), bytes_read - sizeof(int), row_num);
    }
    free(buffer);
}
