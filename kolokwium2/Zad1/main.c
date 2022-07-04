#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Funkcja 'sysv_mem' powinna utworzy i dolaczyc do procesu
 * blok pamieci wspolnej systemu V.
 *
 * Klucz do tworzonego bloku pamieci przekazywany jest w
 * argumencie 'key' a jego rozmiar w argumencie 'size'. Funkcja
 * powinna zapisac adres utworzonego bloku pamieci w zmiennej
 * wskazywanej przez wskaznik 'mem'. Prawa dostepu do bloku
 * pamieci powinny byc ograniczone do odczytu i zapisu dla
 * wlasciciela.
 *
 * Jako wynik dzialania funkcja 'sysv_mem' powinna zwrocic
 * identyfikator utworzonego bloku pamieci.
 */
int sysv_mem(int key, int size, void **mem) {
    int sh_id = shmget(key, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    *mem = shmat(sh_id, NULL, 0);
    // Uzupelnij cialo funkcji sysv_mem zgodnie z
    // komentarzem powyzej.
    return sh_id;

}

int main(int argc, char *argv[]) {
    int *mem, id, k, r;
    id = sysv_mem(IPC_PRIVATE, sizeof(int)*10, (void **) &mem);
    pid_t pid=fork();
    if(pid) {
        waitpid(pid, NULL, 0);
        for(r=0,k=0;k<10;k++) r^=mem[k];
        shmdt(mem);
        shmctl(id, IPC_RMID, NULL);
        printf("r: %d\n", r);
    } else {
        for(k=0;k<10;k++) mem[k]=rand()%65536;
        shmdt(mem);
    }

    return 0;
}
