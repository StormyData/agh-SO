#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
volatile int num_of_received_signals = 0;
volatile int num_of_signals_to_send;
volatile int n;
static void handler(int signo, siginfo_t* info, void* context)
{
    if(signo == SIGUSR1 || signo == SIGRTMIN)
    {
        if(num_of_signals_to_send>0)
        {
            kill(info->si_pid,SIGUSR1);
            num_of_signals_to_send--;
        }
        else
        {
            kill(info->si_pid,SIGUSR2);
        }
        num_of_received_signals++;
    }
    else if(signo == SIGUSR2 || signo == SIGRTMIN + 1)
    {
        printf("received %d signals total out of %d sent\n",num_of_received_signals,n);
        exit(0);
    }
}

int main(int argc, char** argv)
{
    if(argc != 3) {
        printf("this program requires exactly two arguments: n, pid\n");
        exit(-1);
    }
    num_of_signals_to_send = n = atoi(argv[1]);
    int pid = atoi(argv[2]);
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
    sigdelset(&ss,SIGALRM);
    sigprocmask(SIG_SETMASK,&ss,NULL);
    kill(pid,SIGUSR1);
    num_of_signals_to_send--;
    alarm(10);
    while(1)
    {
        sigsuspend(&ss);
    }

}