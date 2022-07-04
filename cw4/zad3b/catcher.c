#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
volatile int nof_signals = 0;
static void handler(int signo, siginfo_t* info, void* context)
{
    if(signo == SIGUSR1 || signo == SIGRTMIN)
    {
        nof_signals++;
        kill(info->si_pid,SIGUSR1);
    }
    else if(signo == SIGUSR2 || signo == SIGRTMIN + 1)
    {
        kill(info->si_pid,SIGUSR2);
        printf("receied %d signals total\n",nof_signals);
        exit(0);
    }
}

int main()
{
    printf("pid = %d\n",getpid());
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask,SIGUSR1);
    sigaddset(&sa.sa_mask,SIGUSR2);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGUSR1,&sa,NULL);
    sigaction(SIGUSR2,&sa,NULL);

    sigset_t ss;
    sigfillset(&ss);
    sigdelset(&ss,SIGUSR1);
    sigdelset(&ss,SIGUSR2);
    sigprocmask(SIG_SETMASK,&ss,NULL);

    while(1)
    {
        sigsuspend(&ss);
    }

}