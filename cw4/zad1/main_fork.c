#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

static void handler(int signal)
{
    printf("Received signal %s, pid = %d\n", strsignal(signal),getpid());
    fflush(stdout);
}
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
        if(!fork())
        {
            raise(SIGUSR1);
        }
        else
        {
            wait(NULL);
        }
    }
    else if(!strcmp(argv[1],"handler"))
    {
        printf("parent pid = %d\n",getpid());
        struct sigaction sigact;
        sigact.sa_flags = 0;
        sigemptyset(&sigact.sa_mask);
        sigact.sa_handler = handler;
        sigaction(SIGUSR1, &sigact,NULL);
        raise(SIGUSR1);
        if(!fork())
        {
            raise(SIGUSR1);
        }
        else
        {
            wait(NULL);
        }
    }
    else if(!strcmp(argv[1],"mask"))
    {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set,SIGUSR1);
        sigprocmask(SIG_BLOCK,&set,NULL);
        raise(SIGUSR1);
        if(!fork())
        {
            raise(SIGUSR1);
            sigset_t set;
            sigset_t oset;
            sigemptyset(&oset);
            sigemptyset(&set);
            sigprocmask(SIG_BLOCK,&set,&oset);
            if(sigismember(&oset,SIGUSR1))
            {
                fflush(stdout);
                printf("child signal is masked\n");
            }
            else
            {
                fflush(stdout);
                printf("child signal is not masked\n");
            }
        }
        else
        {
            wait(NULL);
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
        if(sigismember(&set,SIGUSR1))
        {
            printf("signal is pending\n");
            fflush(stdout);
        }
        else
        {
            fflush(stdout);
            printf("signal is not pending\n");
        }
        if(!fork())
        {
            sigset_t set;
            sigemptyset(&set);
            sigpending(&set);
            if(sigismember(&set,SIGUSR1))
            {
                printf("child signal is pending\n");
                fflush(stdout);
            }
            else
            {
                printf("child signal is not pending\n");
                fflush(stdout);
            }
        }
        else
        {
            wait(NULL);
        }
    }
    exit(0);
}