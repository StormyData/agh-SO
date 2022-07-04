#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define BUFFSIZE 1024
void count_occurences(int src_fd,char c, int* out_lines,int* out_total)
{
    char buffer[BUFFSIZE];
    int bytes_read;
    int good_line = 0;
    do
    {
        bytes_read = read(src_fd,buffer,BUFFSIZE);
        if(bytes_read == -1)
        {
            fprintf(stderr,"cannot read from src file %s\n",strerror(errno));
            exit(5);
        }
        for(int i=0;i < bytes_read; i++)
        {
            if(buffer[i] == c)
            {
                good_line = 1;
                (*out_total)++;
            }
            if(buffer[i] == '\n' && good_line)
            {
                (*out_lines)++;
                good_line = 0;
            }
        }
    } while (bytes_read>0);
    if(good_line)
        (*out_lines)++;

}

int main(int argc,char** argv)
{
    char * src;
    char c;
    if(argc != 3)
    {
        printf("program expects 2 arguments: character to search for and file to search in\n");
        exit(-1);
    }

    src = argv[2];
    c = argv[1][0];
    int src_fd = open(src,O_RDONLY);

    if(!src_fd)
    {
        fprintf(stderr,"error cannot open source file %s, %s\n",src,strerror(errno));
        exit(1);
    }
    int count_total=0;
    int count_lines=0;
    count_occurences(src_fd,c,&count_lines,&count_total);
    printf("total count: %d\nline count: %d\n",count_total,count_lines);
    close(src_fd);


}
