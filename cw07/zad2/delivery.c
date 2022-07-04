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
struct table* table = (void*)-1;
sem_t* table_semaphores[3] = {(void*)-1,(void*)-1,(void*)-1};

int run = 1;

int get_pizza(struct pizza* pizza)
{
    sem_wait_no_int(table_semaphores[2]);
    sem_wait_no_int(table_semaphores[0]);
    memcpy(pizza,&(table->pizzas[table->last]), sizeof (struct pizza));
    table->first++;
    if(table->first >= 5)
        table->first = 0;
    table->size--;
    int ret = table->size;
    sem_post(table_semaphores[0]);
    sem_post(table_semaphores[1]);
    return ret;
}
void loop()
{
    struct pizza pizza;
    int n_of_pizzaz;
    n_of_pizzaz = get_pizza(&pizza);
    printf("(%d %ld) Pobieram pizze: %d Liczba pizz na stole: %d\n", getpid(), time(NULL), pizza.type, n_of_pizzaz);
    random_sleep_between(4.0f, 5.0f);
    printf(" (%d %ld) Dostarczam pizze: %d\n",getpid(), time(NULL), pizza.type);
    random_sleep_between(4.0f, 5.0f);
}
void at_exit()
{
    if(table != (void*)-1)
    {
        munmap(table, sizeof(struct table));
    }
    for(int i = 0;i<3;i++)
        if(table_semaphores[i] != (void*)-1)
            sem_close(table_semaphores[i]);
}

int main()
{

#ifdef DEBUG
    printf("delivery process starting %d\n", getpid());
#endif

    srand(time(NULL)  ^ getpid());

    atexit(at_exit);
    int table_shared_memory_fd = shm_open(table_name,O_RDWR,0);
    if(table_shared_memory_fd < 0)
    {
        perror("cannot open shared memory");
        exit(-1);
    }
    table = mmap(NULL, sizeof(struct table), PROT_READ| PROT_WRITE, MAP_SHARED,table_shared_memory_fd,0);
    if(table == (void*)-1)
    {
        perror("cannot map shared memory");
        exit(-1);
    }
    close(table_shared_memory_fd);
    table_semaphores[0] = sem_open(table_name, O_RDWR);
    if(table_semaphores[0] < 0)
    {
        perror("cannot open semaphores");
        exit(-1);
    }
    table_semaphores[1] = sem_open(table_name2, O_RDWR);
    if(table_semaphores[1] < 0)
    {
        perror("cannot open semaphores");
        exit(-1);
    }
    table_semaphores[2] = sem_open(table_name3, O_RDWR);
    if(table_semaphores[2] < 0)
    {
        perror("cannot open semaphores");
        exit(-1);
    }

    while(run)
    {
        loop();
    }

}