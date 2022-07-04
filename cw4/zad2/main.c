#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
static void handler_resethand(int signo)
{
    printf("received signal %d\n",signo);
}
static void handler_SIGCHLD(int signo, siginfo_t* info, void* context)
{
    printf("received signal: SIGCHLD from pid=%d owned by UID=%d ",info->si_pid,info->si_uid);
    if(info->si_code == CLD_EXITED)
    {
        printf("exit_code=%d ", info->si_status);
        float stime = info->si_stime / sysconf(_SC_CLK_TCK);
        float utime = info->si_utime / sysconf(_SC_CLK_TCK);
        printf("time in usermode %f, time in systemmode %f ",utime,stime);
    }
    char* code_str = "";
    switch (info->si_code) {
        case CLD_EXITED:
            code_str = "CLD_EXITED";
            break;
        case CLD_KILLED:
            code_str = "CLD_KILLED";
            break;
        case CLD_DUMPED:
            code_str = "CLD_DUMPED";
            break;
        case CLD_TRAPPED:
            code_str = "CLD_TRAPPED";
            break;
        case CLD_STOPPED:
            code_str = "CLD_STOPPED";
            break;
        case CLD_CONTINUED:
            code_str = "CLD_CONTINUED";
            break;

    }
    printf("signal code %s\n",code_str);
}
void test_SA_SIGINFO()
{

    printf("test SIGINFO\n");
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler_SIGCHLD;
    sigaction(SIGCHLD,&sa,NULL);
    sigset_t  ss;
    sigemptyset(&ss);
    pid_t cpid;
    if((cpid = fork()) == 0)
    {
        sleep(3);
        exit(2);
    }
    sigval_t sv;
    sv.sival_ptr = NULL;
    printf("stopping child\n");
    sigqueue(cpid,SIGSTOP,sv);
    sleep(1);
    printf("continuing child\n");
    sigqueue(cpid,SIGCONT,sv);
    alarm(5);
    while(1)
    {
        sigsuspend(&ss);
    }
}
void test_SA_NOCLDSTOP()
{
    printf("test NOCLDSTOP\n");
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler_SIGCHLD;
    sigaction(SIGCHLD,&sa,NULL);
    sigset_t  ss;
    sigemptyset(&ss);
    pid_t cpid;
    if((cpid = fork()) == 0)
    {
        sleep(3);
        exit(2);
    }
    sigval_t sv;
    sv.sival_ptr = NULL;
    printf("stopping child\n");
    sigqueue(cpid,SIGSTOP,sv);
    sleep(1);
    printf("continuing child\n");
    sigqueue(cpid,SIGCONT,sv);
    alarm(5);
    while(1)
    {
        sigsuspend(&ss);
    }
}
void test_SA_RESETHAND()
{
    int signal = SIGUSR1;
    printf("test RESETHAND\n");
    struct sigaction sa;
    sa.sa_flags = SA_RESETHAND;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler_resethand;
    sigaction(signal,&sa,NULL);
    sigset_t  ss;
    sigemptyset(&ss);

    sigval_t sv;
    sv.sival_ptr = NULL;
    printf("sending signal once\n");
    sigqueue(getpid(),signal,sv);
    sleep(1);
    printf("sending signal second time\n");
    sigqueue(getpid(),signal,sv);


    alarm(5);
    while(1)
    {
        sigsuspend(&ss);
    }


}
static  void handler_SIGALRM(int signo, siginfo_t* info, void* context)
{
    exit(0);
}


int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("this program requires exactly one argument: test type\n");
        exit(-1);
    }
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler_SIGALRM;
    sigaction(SIGALRM,&sa,NULL);

    if(!strcmp(argv[1],"SIGINFO"))
        test_SA_SIGINFO();
    else if(!strcmp(argv[1],"NOCLDSTOP"))
        test_SA_NOCLDSTOP();
    else if(!strcmp(argv[1],"RESETHAND"))
        test_SA_RESETHAND();
    else
    {
        printf("unknown test type\n");
        exit(-1);
    }
}
