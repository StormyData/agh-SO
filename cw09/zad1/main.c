#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;

volatile int n_of_waiting_elfs = 0;
volatile int n_od_waiting_raindeers = 0;

const int max_elfs = 3;
const int max_raindeers = 9;
const int max_presents = 3;
const int n_of_elfs = 10;



pthread_t* waiting_ids;

void wait_random(float from, float to)
{
    float randv = (float)rand()/(float)RAND_MAX;
    float wait_time = (to - from) * randv + from;
    int wait_seconds = floorf(wait_time);
    wait_time -= wait_seconds;
    struct timespec timespec;
    timespec.tv_nsec = 1000000000LL * wait_time;
    timespec.tv_sec = wait_seconds;
    while(nanosleep(&timespec, &timespec));
}

void* elf_thread(void* _)
{
    while(1)
    {
        wait_random(2,5);
        pthread_mutex_lock(&mutex);
        if(n_of_waiting_elfs == max_elfs)
        {
            printf("Elf: czeka na powrót elfów, %lu\n", pthread_self());
        }
        while(n_of_waiting_elfs == max_elfs)
            pthread_cond_wait(&cond, &mutex);
        waiting_ids[n_of_waiting_elfs] = pthread_self();
        n_of_waiting_elfs++;
        printf("Elf; czeka %d elfów na mikołaja, %lu\n", n_of_waiting_elfs, pthread_self());
        if(n_of_waiting_elfs == max_elfs)
        {
            printf("Elf: wybudzam Mikołaja, %lu\n", pthread_self());
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            printf("Elf: Mikołaj rozwiązuje problem, %lu\n", pthread_self());
            wait_random(1,2);
        }
        else
            pthread_mutex_unlock(&mutex);


    }
    return NULL;
}
void* raindeer_thread(void* _)
{
    while(1)
    {
        wait_random(5, 10);
        pthread_mutex_lock(&mutex);
        while(n_od_waiting_raindeers == max_raindeers)
            pthread_cond_wait(&cond, &mutex);
        n_od_waiting_raindeers++;
        printf("Renifer: czeka %d reniferów na Mikołaja, %lu\n", n_od_waiting_raindeers, pthread_self());
        if(n_od_waiting_raindeers == max_raindeers)
        {
            printf("Renifer: wybudzam Mikołaja, %lu\n", pthread_self());
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
            wait_random(2, 4);
        }
        else
            pthread_mutex_unlock(&mutex);


    }
    return NULL;
}
void* santa_thread(void* _)
{
    int n = 0;
    while(n < max_presents)
    {
        pthread_mutex_lock(&mutex);
        while(n_of_waiting_elfs < max_elfs  && n_od_waiting_raindeers < max_raindeers)
            pthread_cond_wait(&cond, &mutex);

        printf("Mikołaj: budzę się\n");
        n_of_waiting_elfs;
        n_od_waiting_raindeers;
        //printf("%d, %d\n", n_of_waiting_elfs, n_od_waiting_raindeers);
        if(n_od_waiting_raindeers == max_raindeers)
        {
            n_od_waiting_raindeers = 0;
            printf("Mikołaj: dostarczam zabawki\n");
            wait_random(2,4);
            n++;
        }
        else if(n_of_waiting_elfs == max_elfs)
        {
            n_of_waiting_elfs = 0;
            printf("Mikołaj: rozwiązuje problemy elfów");
            for(int i=0;i< max_elfs;i++)
                printf(" %lu", waiting_ids[i]);
            printf("\n");
            wait_random(1,2);
        }
        printf("Mikołaj: zasypiam\n");
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

    }
    printf("Mikołaj: dostarczyłem %d razy prezenty, znikam\n", max_presents);
    exit(0);
    return NULL;
}


int main()
{
    waiting_ids = malloc(sizeof(pthread_t) * max_elfs);
    pthread_t santa;
    pthread_create(&santa, NULL, santa_thread, NULL);
    pthread_t elfs[n_of_elfs];
    for(int i=0;i< n_of_elfs;i++)
        pthread_create(elfs + i, NULL, elf_thread, NULL);
    pthread_t raindeers[max_raindeers];
    for(int i=0;i< max_raindeers;i++)
        pthread_create(raindeers + i, NULL, raindeer_thread, NULL);
    pthread_join(santa, NULL);
    for(int i=0;i< n_of_elfs;i++)
        pthread_join(elfs[i], NULL);
    for(int i=0;i< max_raindeers;i++)
        pthread_join(raindeers[i], NULL);
    free(waiting_ids);
    return 0;
}