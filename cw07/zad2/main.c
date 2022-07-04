#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/stat.h>
#include "common.h"
pid_t spawn_cook()
{
    pid_t child;
    if((child = fork()) == 0)
    {
        execl("./cook","./cook", NULL);
        exit(0);
    }
    return child;
}
pid_t spawn_delivery()
{
    pid_t child;
    if((child = fork()) == 0)
    {
        execl("./delivery","./delivery", NULL);
        exit(0);
    }
    return child;

}
void on_term(int signo)
{
    exit(0);
}
void at_exit()
{
    sem_unlink(table_name);
    sem_unlink(table_name2);
    sem_unlink(table_name3);
    sem_unlink(oven_name);
    sem_unlink(oven_name2);
    sem_unlink(oven_name3);
    shm_unlink(table_name);
    shm_unlink(oven_name);
}

int main(int argc, char** argv)
{
    srand(time(NULL)  ^ getpid());
#ifdef DEBUG
    printf("main process starting\n");
#endif
    if(argc != 3)
    {
        printf("not enough arguments\n");
        exit(-1);
    }
    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = on_term;

    sigaction(SIGTERM,&sa, NULL);
    sigaction(SIGINT,&sa, NULL);

    atexit(at_exit);

    int oven_shared_memory = shm_open(oven_name,O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(oven_shared_memory < 0)
    {
        perror("cannot create shared memory");
        exit(-1);
    }
    if(ftruncate(oven_shared_memory,sizeof(struct oven)) == -1)
    {
        perror("cannot resize shared memory");
        exit(-1);
    }
    struct oven* oven = mmap(NULL, sizeof(struct oven), PROT_READ| PROT_WRITE, MAP_SHARED,oven_shared_memory,0);
    if(oven == (void*)-1)
    {
        perror("cannot open shared memory");
        exit(-1);
    }
    close(oven_shared_memory);
    oven->last = 0;
    oven->first = 0;
    oven->size = 0;
    munmap(oven, sizeof(struct oven));

    mk_semaphore(oven_name,1);
    mk_semaphore(oven_name2,5);
    mk_semaphore(oven_name3,0);

    int table_shared_memory = shm_open(table_name,O_CREAT| O_EXCL |O_RDWR, S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(table_shared_memory < 0)
    {
        perror("cannot create shared memory");
        exit(-1);
    }
    if(ftruncate(table_shared_memory,sizeof(struct table)) == -1)
    {
        perror("cannot resize shared memory");
        exit(-1);
    }

    struct table* table = mmap(NULL, sizeof(struct table), PROT_READ| PROT_WRITE, MAP_SHARED,table_shared_memory,0);
    if(table == (void*)-1)
    {
        perror("cannot open shared memory");
        exit(-1);
    }
    close(table_shared_memory);

    table->last = 0;
    table->first = 0;
    table->size = 0;
    munmap(table, sizeof(struct table));

    mk_semaphore(table_name,1);
    mk_semaphore(table_name2,5);
    mk_semaphore(table_name3,0);


    pid_t children[N + M];
    for(int i=0;i<N;i++)
        children[i] = spawn_cook();
    for(int i=0;i<M;i++)
        children[i + N] = spawn_delivery();
    for(int i=0;i< N + M; i++)
    {
        int status;
        waitpid(children[i],&status,0);
        if(WIFEXITED(status) || WIFSIGNALED(status))
            i++;
    }

}
