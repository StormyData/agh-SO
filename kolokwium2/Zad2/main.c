#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

/*
 * Funkcja 'create_mutex' powinna zainicjalizować mutex w taki sposób, by w sytuacji
 * gdy wątek próbuje zablokować mutex który już ma zablokowany nie następowało zakleszczenie,
 * a zgłaszany był błąd.
 */
void create_mutex(pthread_mutex_t* mutex) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(mutex,&attr);
    pthread_mutexattr_destroy(&attr);
}

int main(void) {
    pthread_mutex_t mutex;
    create_mutex(&mutex);

    int ret = pthread_mutex_lock(&mutex);
    if (ret != 0) {
        puts("Error");
        exit(-1);
    }
    ret = pthread_mutex_lock(&mutex);
    if (ret == EDEADLK) {
        puts("Ok");
    } else {
        printf("Error, ret = %d\n", ret);
        exit(-1);
    }
    pthread_mutex_destroy(&mutex);
}
