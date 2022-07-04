#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/stat.h>
#include "common.h"
int oven_shared_memory = -1;
int oven_semaphores = -1;
int table_shared_memory = -1;
int table_semaphores = -1;
void rm_oven_shared_memory()
{
    if(oven_shared_memory<0)
        return;
    shmctl(oven_shared_memory,IPC_RMID, NULL);
}

void rm_oven_semaphores()
{
    if(oven_semaphores<0)
        return;
    for(int i=0;i<3;i++)
        semctl(oven_semaphores,i,IPC_RMID);
}

void rm_table_shared_memory()
{
    if(table_shared_memory<0)
        return;
    shmctl(table_shared_memory,IPC_RMID, NULL);
}

void rm_table_semaphores()
{
    if(table_semaphores<0)
        return;
    for(int i=0;i<3;i++)
        semctl(table_semaphores,i,IPC_RMID);
}

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



    key_t master_key_oven = ftok(getenv("HOME"),0);
    key_t master_key_table = ftok(getenv("HOME"), 1);
    //printf("%d %d\n", master_key_table, master_key_oven);
    atexit(rm_oven_shared_memory);
    oven_shared_memory = shmget(master_key_oven,sizeof(struct oven),IPC_CREAT| S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(oven_shared_memory < 0)
    {
        perror("cannot create shared memory");
        exit(-1);
    }
    struct oven* oven = shmat(oven_shared_memory,NULL,0);
    oven->last = 0;
    oven->first = 0;
    oven->size = 0;
    shmdt(oven);
    atexit(rm_oven_semaphores);
    oven_semaphores = semget(master_key_oven,3,IPC_CREAT| S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(oven_semaphores < 0)
    {
        perror("cannot create semaphores");
        exit(-1);
    }
    struct sembuf oven_settings[3];
    oven_settings[0].sem_num = 0;
    oven_settings[0].sem_flg = 0;
    oven_settings[0].sem_op = 1;

    oven_settings[1].sem_num = 1;
    oven_settings[1].sem_flg = 0;
    oven_settings[1].sem_op = 5;

    oven_settings[2].sem_num = 2;
    oven_settings[2].sem_flg = 0;
    oven_settings[2].sem_op = 0;
    semop(oven_semaphores,oven_settings, 3);

    atexit(rm_table_shared_memory);
    table_shared_memory = shmget(master_key_table,sizeof (struct table), IPC_CREAT| S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(table_shared_memory < 0)
    {
        perror("cannot create shared memory");
        exit(-1);
    }
    struct table* table = shmat(table_shared_memory,NULL,0);
    table->last = 0;
    table->first = 0;
    table->size = 0;
    shmdt(table);

    atexit(rm_table_semaphores);
    table_semaphores = semget(master_key_table, 3, IPC_CREAT| S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    if(table_semaphores < 0)
    {
        perror("cannot create semaphores");
        exit(-1);
    }
    struct sembuf table_settings[3];
    table_settings[0].sem_num = 0;
    table_settings[0].sem_flg = 0;
    table_settings[0].sem_op = 1;

    table_settings[1].sem_num = 1;
    table_settings[1].sem_flg = 0;
    table_settings[1].sem_op = 5;


    table_settings[2].sem_num = 2;
    table_settings[2].sem_flg = 0;
    table_settings[2].sem_op = 0;
    semop(table_semaphores,table_settings, 3);

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
