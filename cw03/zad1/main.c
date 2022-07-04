#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        printf("this program requires exactly 1 argument: n - nonegative integer");
        exit(-1);
    }
    int n = atoi(argv[1]);
    printf("hello from parent: PID=%d\n",getpid());
    while(n--)
    {
        pid_t child = fork();
        if(child == 0)
        {
            printf("hello from child: PID = %d, n = %d\n",getpid(),n);
            exit(0);
        }
    }

}