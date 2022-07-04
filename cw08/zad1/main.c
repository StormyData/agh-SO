#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "common.h"
#include "block.h"
#include "numbers.h"


int main(int argc, char** argv)
{
    if(argc != 5)
    {
        printf("invalid argument count\n");
        exit(-1);
    }
    int m = atoi(argv[1]);
    int mode = 0;
    if(!strcmp(argv[2],"block"))
        mode = 0;
    else if(!strcmp(argv[2],"numbers"))
        mode = 1;
    char* input_filename = argv[3];
    char* output_filename = argv[4];
    struct pgm_img img;
    img.tab = NULL;
    load_pgm(input_filename,&img);
    pthread_t threads[m];
    struct timespec time_tab[m];

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    if(mode == 0)
    {
        for(int i = 0; i < m; i++)
        {
            struct block_job* job = get_block_job(&img,i,m);
            pthread_create(&threads[i],NULL,do_block_job,job);
        }
    }
    else if(mode == 1)
    {
        for(int i = 0; i < m; i++)
        {
            struct numbers_job* job = get_numbers_job(&img,i,m);
            pthread_create(&threads[i],NULL,do_numbers_job,job);
        }
    }
    for(int i = 0; i < m; i++)
    {
        struct timespec* tmp;
        pthread_join(threads[i],(void**)&tmp);
        time_tab[i].tv_sec = tmp->tv_sec;
        time_tab[i].tv_nsec = tmp->tv_nsec;
        free(tmp);
    }

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    for(int i = 0; i< m;i++)
    {
        printf("thread index = %d, time taken = %lds%ldns\n", i, time_tab[i].tv_sec, time_tab[i].tv_nsec);
    }
    struct timespec total;
    timespec_diff(&start, &end, &total);
    printf("total time taken = %lds%ldns\n", total.tv_sec, total.tv_nsec);

    store_pgm(output_filename, &img);
    free(img.tab);
}
