#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
    if(argc != 4)
    {
        printf("this program requires exactly 3 arguments N, number of producers, number of consumers");
        exit(-1);
    }
    int np = atoi(argv[2]);
    int nc = atoi(argv[3]);
    char* path_to_fifo = "./fifo";
    mkfifo(path_to_fifo,S_IRUSR | S_IWUSR);
    char* writefile = "out.txt";
    pid_t children[nc + np];
    for(int i =0; i< nc; i++)
    {
        pid_t child;
        if((child = fork()) == 0)
        {
            execl("./consumer","./consumer", path_to_fifo, writefile, argv[1], NULL);
        }
        children[i] = child;
    }
    for(int i =0; i< np; i++)
    {
        pid_t child;
        if((child = fork()) == 0)
        {
            char buff[512];
            sprintf(buff,"%d",i);
            char open_file_buff[4096];
            sprintf(open_file_buff,"in%d.txt",i + 1);
            execl("./producer","./producer", path_to_fifo, buff, open_file_buff, argv[1], NULL);
        }
        children[nc + i] = child;
    }
    for(int i =0; i< nc+np;)
    {
        int wstatus;
        waitpid(children[i],&wstatus,0);
        if(!WIFSTOPPED(wstatus))
            i++;
    }
}