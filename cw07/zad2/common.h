#pragma once
#include <time.h>
#include <errno.h>

struct pizza{
    int type;
};

struct oven{
    struct pizza pizzas[5];
    int last;
    int first;
    int size;
};

struct table{
    struct pizza pizzas[5];
    int last;
    int first;
    int size;
};

void random_sleep_between(float min, float max)
{
    usleep(((float)rand()/(float )RAND_MAX * (max - min) + min) * 1000000.0f);
}

void mk_semaphore(char* name, int val)
{
    sem_t* sem = sem_open(name, O_CREAT | O_EXCL|O_RDWR, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH, val);

    if(sem == SEM_FAILED)
    {
        perror("cannot create semaphore");
        exit(-1);
    }
    sem_close(sem);
}
void sem_wait_no_int(sem_t* sem)
{
    while (sem_wait(sem) && errno == EINTR);
}

char table_name[] = "/sysopy_lab07_table";
char table_name2[] = "/sysopy_lab07_table2";
char table_name3[] = "/sysopy_lab07_table3";
char oven_name[] = "/sysopy_lab07_oven";
char oven_name2[] = "/sysopy_lab07_oven2";
char oven_name3[] = "/sysopy_lab07_oven3";