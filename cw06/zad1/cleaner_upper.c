#include <sys/msg.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if(argc != 2)
        exit(-1);
    int proj_id = atoi(argv[1]);
    key_t key = ftok(getenv("HOME"),proj_id);

    int msqid = msgget(key,0);
    msgctl(msqid,IPC_RMID,NULL);

}