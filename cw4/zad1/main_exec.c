#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf(" this program requires exactly 1 argument\n");
        fflush(stdout);
        exit(-1);
    }
    if(!strcmp(argv[1],"ignore"))
    {
        signal(SIGUSR1,SIG_IGN);
        raise(SIGUSR1);
        execl(argv[0],argv[0],"raise", NULL);
    }
    else if(!strcmp(argv[1],"raise"))
    {
        raise(SIGUSR1);
    }
    else if(!strcmp(argv[1],"mask"))
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set,SIGUSR1);
        sigprocmask(SIG_BLOCK,&set,NULL);
        raise(SIGUSR1);
        execl(argv[0],argv[0],"test_mask", NULL);

    }
    else if(!strcmp(argv[1],"test_mask"))
    {
        raise(SIGUSR1);
        sigset_t set;
        sigset_t oset;
        sigemptyset(&oset);
        sigemptyset(&set);
        sigprocmask(SIG_BLOCK,&set,&oset);
        if(sigismember(&oset,SIGUSR1))
        {
            printf("execl signal is masked\n");
            fflush(stdout);
        }
        else
        {
            printf("execl signal is not masked\n");
            fflush(stdout);
        }
    }
    else if(!strcmp(argv[1],"pending"))
    {
        sigset_t sset;
        sigemptyset(&sset);
        sigaddset(&sset,SIGUSR1);
        sigprocmask(SIG_BLOCK,&sset,NULL);

        raise(SIGUSR1);

        sigset_t set;
        sigemptyset(&set);
        sigpending(&set);
        if(sigismember(&set,SIGUSR1)) {
            printf("signal is pending\n");
            fflush(stdout);
        }
        else
        {
            printf("signal is not pending\n");
            fflush(stdout);
        }
        execl(argv[0],argv[0] ,"test_pending",NULL);
    }
    else if(!strcmp(argv[1],"test_pending"))
    {
        sigset_t set;
        sigemptyset(&set);
        sigpending(&set);
        if(sigismember(&set,SIGUSR1)) {
            printf("execl signal is pending\n");
            fflush(stdout);
        }
        else
        {
            printf("execl signal is not pending\n");
            fflush(stdout);
        }

    }
    exit(0);
}