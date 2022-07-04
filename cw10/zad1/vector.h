#pragma once

#include <malloc.h>

struct MyVector{
    void** data;
    int len;
    int max_len;
};

void MyVectorInit(struct MyVector* vector, int start_size)
{
    if(start_size <= 0)
        start_size = 1;
    vector->len = 0;
    vector->max_len = start_size;
    vector->data = malloc(start_size * sizeof (void*));
}
void MyVectorAppend(struct MyVector* vector, void* value)
{
    if(vector->len == vector->max_len)
    {
        int new_len = vector->max_len * 2;
        vector->data = realloc(vector->data, new_len * sizeof(void*));
        vector->max_len = new_len;
    }
    vector->data[vector->len] = value;
    vector->len++;
}
int MyVectorRemoveAndReplaceWithLast(struct MyVector* vector, int index)
{
    if(vector->len <= 0)
    {
        fprintf(stderr, "tried to remove element from empty vector");
        return 0;
    }
    if(index < 0 || index >= vector->len)
    {
        fprintf(stderr, "tried to remove element at invalid index");
        return 0;
    }
    vector->data[index] = vector->data[vector->len - 1];
    vector->len--;
    return 1;
}
void MyVectorDestroy(struct MyVector* vector)
{
    free(vector->data);
    vector->data = NULL;
    vector->max_len = 0;
    vector->len = 0;
}
