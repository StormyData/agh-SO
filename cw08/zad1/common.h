#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
struct pgm_img
{
    u_int8_t * tab;
    int width;
    int height;
};


void load_pgm(char* path, struct pgm_img* img)
{
    FILE* file = fopen(path, "r");
    fscanf(file,"%d", &img->width);
    fscanf(file,"%d", &img->height);
    int M;
    fscanf(file, "%d", &M);
    if(M > 255)
    {
        fprintf(stderr, "Image has values outside of range");
        exit(-1);
    }
    img->tab = realloc(img->tab, img->width * img->height * sizeof(u_int8_t));
    for(int y = 0; y < img->height; y++)
    {
        for(int x = 0; x < img->width; x++)
        {
            int tmp;
            fscanf(file, "%d", &tmp);
            img->tab[y * img->width + x] = tmp;
        }
    }

    fclose(file);
}

void store_pgm(char* path, struct pgm_img* img)
{
    FILE* file = fopen(path, "w");
    int M = 0;
    for(int i=0;i< img->width * img->height;i++)
    {
        if(M < img->tab[i])
            M = img->tab[i];
    }

    fprintf(file, "%d %d\n%d\n",img->width, img->height, M);
    for(int y = 0; y < img->height; y++)
    {
        for(int x = 0; x < img->width; x++)
        {
            fprintf(file, "%d ", img->tab[y * img->width + x]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

void timespec_diff(struct timespec* start, struct timespec* end, struct timespec* result)
{
    result->tv_sec = end->tv_sec - start->tv_sec;
    result->tv_nsec = end->tv_nsec - start->tv_nsec;
    if(result->tv_nsec < 0)
    {
        result->tv_nsec += 1000000000L;
        result->tv_sec--;
    }
}