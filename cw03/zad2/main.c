#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
void calc(int n, int i, int m)
{
    float start = i/n;
    float end = (i+1)/n;
    float h = (end - start)/m;
    float out = 0;
    for(float x = start;x<end;x+=h)
        out+= 4*1/(1+ x*x)/n;
    char buff[1024];
    snprintf(buff,1024,"w%d.txt",i + 1);
    FILE* file = fopen(buff,"w");
    fprintf(file,"%f",out);
    fclose(file);
}
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        printf("this program requires exactly 2 arguments: n - nnumber of processes, m - number of rectangles per process");
        exit(-1);
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    for(int i=0;i<n;i++)
    {
        pid_t child = fork();
        if(child == 0)
        {
            calc(n,i,m);
            exit(0);
        }
    }
    for(int i=0;i<n;i++)
        wait(NULL);
    float sum = 0;
    for(int i=0;i<n;i++)
    {
        float val;
        char buff[1024];
        snprintf(buff,1024,"w%d.txt",i + 1);
        FILE* file = fopen(buff,"r");
        fscanf(file,"%f",&val);
        fclose(file);
        sum += val;
    }
    printf("result = %f\n", sum);
    return 0;
}