#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include "common.h"
#include <memory.h>
#include <errno.h>

#include <sys/stat.h>
struct oven* oven = NULL;
struct table* table = NULL;
int oven_semaphores = -1;
int table_semaphores = -1;

int run = 1;

int put_pizza(struct pizza* pizza)
{
    struct sembuf sembuf[2];
    sembuf[0].sem_op = -1;
    sembuf[0].sem_flg = 0;
    sembuf[0].sem_num = 0;

    sembuf[1].sem_op = -1;
    sembuf[1].sem_flg = 0;
    sembuf[1].sem_num = 1;

    while(semop(oven_semaphores,sembuf,2))
    {
        if(errno == EIDRM)
        {
            exit(-1);
        }
    }
    memcpy(&(oven->pizzas[oven->last]), pizza, sizeof (struct pizza));
    oven->last++;
    if(oven->last >= 5)
        oven->last = 0;
    oven->size++;

    int ret = oven->size;
    sembuf[0].sem_op = 1;

    sembuf[1].sem_num = 2;
    sembuf[1].sem_op = 1;
    while(semop(oven_semaphores, sembuf, 2))
    {
        if(errno == EIDRM)
        {
            exit(-1);
        }
    }

    return ret;
}
int get_pizza(struct pizza* pizza)
{
    struct sembuf sembuf[2];
    sembuf[0].sem_op = -1;
    sembuf[0].sem_flg = 0;
    sembuf[0].sem_num = 0;

    sembuf[1].sem_op = -1;
    sembuf[1].sem_flg = 0;
    sembuf[1].sem_num = 2;


    while(semop(oven_semaphores,sembuf,2))
    {
        if(errno == EIDRM)
        {
            exit(-1);
        }
    }

    memcpy(pizza,&(oven->pizzas[oven->last]), sizeof (struct pizza));
    oven->first++;
    if(oven->first >= 5)
        oven->first = 0;
    oven->size--;
    int ret = oven->size;

    sembuf[0].sem_op = 1;

    sembuf[1].sem_op =  1;
    sembuf[1].sem_flg = 0;
    sembuf[1].sem_num = 1;
    while(semop(oven_semaphores, sembuf, 2))
    {
        if(errno == EIDRM)
        {
            exit(-1);
        }
    }

    return ret;
}
int finish_pizza(struct pizza* pizza)
{
    struct sembuf sembuf[2];
    sembuf[0].sem_op = -1;
    sembuf[0].sem_flg = 0;
    sembuf[0].sem_num = 0;

    sembuf[1].sem_op = -1;
    sembuf[1].sem_flg = 0;
    sembuf[1].sem_num = 1;
    while(semop(table_semaphores,sembuf,2))
    {
        if(errno == EIDRM)
        {
            exit(-1);
        }
    }
    memcpy(&(table->pizzas[table->last]), pizza, sizeof (struct pizza));
    table->last++;
    if(table->last >= 5)
        table->last = 0;
    table->size++;
    int ret = table->size;

    sembuf[0].sem_op = 1;

    sembuf[1].sem_op = 1;
    sembuf[1].sem_num = 2;
    while(semop(table_semaphores, sembuf, 2))
    {
        if(errno == EIDRM)
        {
            exit(-1);
        }
    }
    return ret;
}


void rm_oven_shared_memory()
{
    if(oven != NULL)
        shmdt(oven);
}

void rm_table_shared_memory()
{
    if(table != NULL)
        shmdt(table);
}
void loop()
{
    struct pizza pizza;
    pizza.type = rand()%10;
    printf("(%d %ld) przygotowuje pizze: %d\n", getpid(), time(NULL), pizza.type);
    random_sleep_between(1.0f, 2.0f);
    int n_of_pizzaz;
    n_of_pizzaz = put_pizza(&pizza);
    printf("(%d %ld) dodalem pizze: %d, liczba pizz w piecu: %d\n", getpid(), time(NULL), pizza.type, n_of_pizzaz);
    random_sleep_between(4.0f, 5.0f);
    n_of_pizzaz = get_pizza(&pizza);
    printf("%d\n", getpid());
    int n_of_pizzaz2 = finish_pizza(&pizza);
    printf("(%d %ld) Wyjmuję pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n", getpid(), time(NULL), pizza.type, n_of_pizzaz , n_of_pizzaz2);
}
int main()
{

#ifdef DEBUG
    printf("cook process starting: %d\n", getpid());
#endif

    srand(time(NULL) ^ getpid());
    key_t master_key_oven = ftok(getenv("HOME"),0);
    key_t master_key_table = ftok(getenv("HOME"), 1);
    atexit(rm_oven_shared_memory);
    int oven_shared_memory = shmget(master_key_oven,sizeof(struct oven),S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(oven_shared_memory < 0)
    {
        perror("cannot open shared memory\n");
        exit(-1);
    }
    oven = shmat(oven_shared_memory, NULL, 0);
    oven_semaphores = semget(master_key_oven,2,S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(oven_semaphores < 0)
    {
        perror("cannot open semaphores\n");
        exit(-1);
    }

    atexit(rm_table_shared_memory);
    int table_shared_memory = shmget(master_key_table,sizeof (struct table), S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(table_shared_memory < 0)
    {
        perror("cannot open shared memory\n");
        exit(-1);
    }
    table = shmat(table_shared_memory,NULL,0);
    table_semaphores = semget(master_key_table, 2, S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(table_semaphores < 0)
    {
        perror("cannot open semaphores\n");
        exit(-1);
    }

    while(run)
    {
        loop();
    }

}