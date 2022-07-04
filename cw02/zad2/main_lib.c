#include <stdio.h>
#include <stdlib.h>
#define BUFFSIZE 1024
void count_occurences(FILE* src_f,char c, int* out_lines,int* out_total)
{
    char buffer[BUFFSIZE];
    int bytes_read;
    int good_line = 0;
    do
    {
        bytes_read = fread(buffer,1,BUFFSIZE,src_f);
        if(ferror(src_f))
        {
            fprintf(stderr,"cannot read from src file\n");
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
    FILE* src_f = fopen(src,"rb");

    if(!src_f)
    {
        fprintf(stderr,"error cannot open source file %s\n",src);
        exit(1);
    }
    int count_total=0;
    int count_lines=0;
    count_occurences(src_f,c,&count_lines,&count_total);
    fclose(src_f);
    printf("total count: %d\nline count: %d\n",count_total,count_lines);

}
