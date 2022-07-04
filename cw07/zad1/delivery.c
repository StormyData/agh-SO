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
struct table* table = NULL;
int table_semaphores = -1;

int run = 1;

int get_pizza(struct pizza* pizza)
{
    struct sembuf sembuf[2];
    sembuf[0].sem_op = -1;
    sembuf[0].sem_flg = 0;
    sembuf[0].sem_num = 0;

    sembuf[1].sem_op = -1;
    sembuf[1].sem_flg = 0;
    sembuf[1].sem_num = 2;
    while(semop(table_semaphores,sembuf,2))
    {
        if(errno == EIDRM)
        {
            exit(-1);
        }
    }
    memcpy(pizza,&(table->pizzas[table->last]), sizeof (struct pizza));
    table->first++;
    if(table->first >= 5)
        table->first = 0;
    table->size--;
    int ret = table->size;
    sembuf[0].sem_op = 1;

    sembuf[1].sem_op = 1;
    sembuf[1].sem_num = 1;
    while(semop(table_semaphores, sembuf, 2))
    {
        if(errno == EIDRM)
        {
            exit(-1);
        }
    }
    return ret;
}



void rm_table_shared_memory()
{
    if(table != NULL)
        shmdt(table);
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
int main()
{

#ifdef DEBUG
    printf("delivery process starting %d\n", getpid());
#endif

    srand(time(NULL)  ^ getpid());
    key_t master_key_table = ftok(getenv("HOME"), 1);

    atexit(rm_table_shared_memory);
    int table_shared_memory = shmget(master_key_table,sizeof (struct table), S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(table_shared_memory < 0)
    {
        perror("cannot open shared memory");
        exit(-1);
    }
    table = shmat(table_shared_memory,NULL,0);
    table_semaphores = semget(master_key_table, 2, S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(table_semaphores < 0)
    {
        perror("cannot open semaphores");
        exit(-1);
    }

    while(run)
    {
        loop();
    }

}