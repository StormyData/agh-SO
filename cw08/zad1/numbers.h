#pragma once
#include "common.h"
#include <time.h>
#include <math.h>

struct numbers_job{
    int value_from;
    int value_to;
    struct pgm_img* img_ptr;
};

void* do_numbers_job(void* arg)
{
    struct numbers_job* numbersJob = arg;
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    int img_width = numbersJob->img_ptr->width;
    int img_height = numbersJob->img_ptr->height;
    u_int8_t* data = numbersJob->img_ptr->tab;

    for(int x = 0;x < img_height;x++)
    {
        for(int y = 0; y < img_height; y++)
        {
            u_int8_t curr = data[x + y * img_width];
            if(curr >= numbersJob->value_from && curr < numbersJob->value_to)
                data[x + y * img_width] = 255 - curr;
        }
    }

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    struct timespec* result = malloc(sizeof (struct timespec));
    timespec_diff(&start, &end, result);
    free(arg);
    return result;
}


struct numbers_job* get_numbers_job(struct pgm_img* img, int i, int m)
{
    struct numbers_job* res = malloc(sizeof (struct numbers_job));
    res->img_ptr = img;
    int per = (int)ceilf((float)256/m);
    res->value_from = i * per;
    res->value_to = (i + 1) * per;
    return res;
}