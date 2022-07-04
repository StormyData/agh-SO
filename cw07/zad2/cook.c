#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include "common.h"
#include <memory.h>
#include <errno.h>

#include <sys/stat.h>
struct oven* oven = (void*)-1;
struct table* table = (void*)-1;
sem_t* oven_semaphores[3] = {(void*)-1, (void*)-1, (void*)-1};
sem_t* table_semaphores[3] = {(void*)-1, (void*)-1, (void*)-1};

int run = 1;

int put_pizza(struct pizza* pizza)
{

    sem_wait_no_int(oven_semaphores[1]);
    sem_wait_no_int(oven_semaphores[0]);
    memcpy(&(oven->pizzas[oven->last]), pizza, sizeof (struct pizza));
    oven->last++;
    if(oven->last >= 5)
        oven->last = 0;
    oven->size++;

    int ret = oven->size;
    sem_post(oven_semaphores[0]);
    sem_post(oven_semaphores[2]);
    return ret;
}
int get_pizza(struct pizza* pizza)
{

    sem_wait_no_int(oven_semaphores[2]);
    sem_wait_no_int(oven_semaphores[0]);
    memcpy(pizza,&(oven->pizzas[oven->last]), sizeof (struct pizza));
    oven->first++;
    if(oven->first >= 5)
        oven->first = 0;
    oven->size--;
    int ret = oven->size;

    sem_post(oven_semaphores[0]);
    sem_post(oven_semaphores[1]);

    return ret;
}
int finish_pizza(struct pizza* pizza)
{
    sem_wait_no_int(table_semaphores[1]);
    sem_wait_no_int(table_semaphores[0]);
    memcpy(&(table->pizzas[table->last]), pizza, sizeof (struct pizza));
    table->last++;
    if(table->last >= 5)
        table->last = 0;
    table->size++;
    int ret = table->size;
    sem_post(table_semaphores[0]);
    sem_post(table_semaphores[2]);
    return ret;
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
void at_exit()
{
    if(oven != (void *)-1)
        munmap(oven, sizeof(struct oven));
    if(table != (void *)-1)
        munmap(oven, sizeof(struct table));
    for(int i=0;i<3;i++)
        if(oven_semaphores[i] != (void *)-1)
            sem_close(oven_semaphores[i]);
    for(int i=0;i<3;i++)
        if(table_semaphores[i] != (void *)-1)
            sem_close(table_semaphores[i]);
}
int main()
{

#ifdef DEBUG
    printf("cook process starting: %d\n", getpid());
#endif

    srand(time(NULL) ^ getpid());
    atexit(at_exit);
    int oven_shared_memory_fd = shm_open(oven_name,O_RDWR,0);
    if(oven_shared_memory_fd < 0)
    {
        perror("cannot open shared memory\n");
        exit(-1);
    }
    oven = mmap(NULL, sizeof(struct oven), PROT_READ| PROT_WRITE, MAP_SHARED,oven_shared_memory_fd,0);
    if(oven == (void*)-1)
    {
        perror("cannot open shared memory");
        exit(-1);
    }
    close(oven_shared_memory_fd);

    oven_semaphores[0] = sem_open(oven_name,O_RDWR);
    if(oven_semaphores[0] < 0)
    {
        perror("cannot open semaphores\n");
        exit(-1);
    }
    oven_semaphores[1] = sem_open(oven_name2,O_RDWR);
    if(oven_semaphores[1] < 0)
    {
        perror("cannot open semaphores\n");
        exit(-1);
    }
    oven_semaphores[2] = sem_open(oven_name3,O_RDWR);
    if(oven_semaphores[2] < 0)
    {
        perror("cannot open semaphores\n");
        exit(-1);
    }

    int table_shared_memory_fd = shm_open(table_name,O_RDWR, 0);
    if(table_shared_memory_fd < 0)
    {
        perror("cannot open shared memory\n");
        exit(-1);
    }
    table = mmap(NULL, sizeof(struct table), PROT_READ| PROT_WRITE, MAP_SHARED,table_shared_memory_fd,0);
    if(table == (void*)-1)
    {
        perror("cannot open shared memory");
        exit(-1);
    }
    close(table_shared_memory_fd);
    table_semaphores[0] = sem_open(table_name, O_RDWR);
    if(table_semaphores[0] < 0)
    {
        perror("cannot open semaphores\n");
        exit(-1);
    }
    table_semaphores[1] = sem_open(table_name2, O_RDWR);
    if(table_semaphores[1] < 0)
    {
        perror("cannot open semaphores\n");
        exit(-1);
    }
    table_semaphores[2] = sem_open(table_name3, O_RDWR);
    if(table_semaphores[2] < 0)
    {
        perror("cannot open semaphores\n");
        exit(-1);
    }

    while(run)
    {
        loop();
    }

}