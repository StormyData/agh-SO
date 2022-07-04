#pragma once
#include "common.h"
#include <time.h>
#include <math.h>

struct block_job
{
    struct pgm_img* img_ptr;
    int from_x;
    int to_x;
};
void* do_block_job(void* arg)
{
    struct block_job* blockJob = arg;
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    int img_width = blockJob->img_ptr->width;
    int img_height = blockJob->img_ptr->height;
    u_int8_t* data = blockJob->img_ptr->tab;
    for(int x = blockJob->from_x;x< blockJob->to_x;x++)
    {
        for(int y = 0; y < img_height; y++)
        {
            data[y*  img_width + x] = 255 - data[y* img_width + x];
        }
    }

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    struct timespec* result = malloc(sizeof (struct timespec));
    timespec_diff(&start, &end, result);
    free(arg);
    return result;
}


struct block_job* get_block_job(struct pgm_img* img, int i, int m)
{
    struct block_job* res = malloc(sizeof (struct block_job));
    res->img_ptr = img;
    int per = (int)ceilf((float)img->width/m);
    res->from_x = i * per;
    res->to_x = (i + 1) * per;
    return res;
}

