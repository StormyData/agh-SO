#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
volatile int type = 0;
volatile int num_of_received_signals = 0;
volatile int n = 0;

static void send_kill(pid_t pid,int nof_signals)
{
    for(int i=0;i<nof_signals;i++)
        kill(pid, SIGUSR1);
    kill(pid,SIGUSR2);

}
static void send_sigqueue(pid_t pid,int nof_signals)
{
    union sigval val;
    for(int i=0;i<nof_signals;i++) {
        val.sival_int = i;
        sigqueue(pid, SIGUSR1, val);
    }
    val.sival_int = 0;
    sigqueue(pid, SIGUSR2, val);
}
static void send_sigrt(pid_t pid,int nof_signals)
{
    for(int i=0;i<nof_signals;i++)
        kill(pid, SIGRTMIN);
    kill(pid,SIGRTMIN + 1);
}

static void send(pid_t pid, int nof_signals, int type)
{
    switch (type) {
        case 0:
            send_kill(pid,nof_signals);
            break;
        case 1:
            send_sigqueue(pid,nof_signals);
            break;
        case 2:
            send_sigrt(pid,nof_signals);
            break;
    }
}
static void handler(int signo, siginfo_t* info, void* context)
{
    if(signo == SIGUSR1 || signo == SIGRTMIN)
    {
        if(type == 1)
            printf("received signal no = %d\n",info->si_value.sival_int);
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
    if(argc != 4) {
        printf("this program requires exactly three arguments: n, pid, type\n");
        exit(-1);
    }
    n = atoi(argv[1]);
    int pid = atoi(argv[2]);
    type = atoi(argv[3]);
    int sig = SIGUSR1;
    int sig2 = SIGUSR2;
    if(type == 2)
    {
        sig = SIGRTMIN;
        sig2 = SIGRTMIN + 1;
    }
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask,sig);
    sigaddset(&sa.sa_mask,sig2);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(sig,&sa,NULL);
    sigaction(sig2,&sa,NULL);

    sigset_t ss;
    sigfillset(&ss);
    sigdelset(&ss,sig);
    sigdelset(&ss,sig2);
    sigprocmask(SIG_SETMASK,&ss,NULL);
    send(pid, n,type);
    while(1)
    {
        sigsuspend(&ss);
    }

}