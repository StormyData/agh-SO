#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#define BUFFSIZE 1024
void copy_from_offsets(FILE* src_f,FILE* dst_f,long start_offset,long end_offset)
{
    if(end_offset<=start_offset)
        return;
    long curr_back = ftell(src_f);
    fseek(src_f,start_offset,SEEK_SET);
    char buffer[BUFFSIZE];
    long left_to_write = end_offset - start_offset;
    while(left_to_write>0)
    {
        long to_write = left_to_write > BUFFSIZE ? BUFFSIZE : left_to_write;
        long bytes_read = fread(buffer,1,to_write,src_f);
        if(ferror(src_f))
        {
            fprintf(stderr,"cannot read from src file %s\n", strerror(errno));
            exit(5);
        }
        int bytes_written=0;
        while (bytes_written<bytes_read)
        {
            int tmp=fwrite(buffer + bytes_written,1,bytes_read - bytes_written,dst_f);
            if(ferror(dst_f))
            {
                fprintf(stderr,"Error writing to destination file %s\n",strerror(errno));
                exit(4);
            }
            bytes_written +=tmp;
        }

        left_to_write -= bytes_read;
    }
    fseek(src_f,curr_back,SEEK_SET);
}
void copy_without_blank_lines(FILE* src_f,FILE* dst_f)
{
    char buffer[BUFFSIZE];
    int bytes_read;
    int good_line = 0;
    long last_line_start = ftell(src_f);
    if(last_line_start == -1) {
        fprintf(stderr, "source file not seekable %s\n",strerror(errno));
        exit(3);
    }
    do
    {
        long block_start_offset = ftell(src_f);
        bytes_read = fread(buffer,1,BUFFSIZE,src_f);
        if(ferror(src_f))
        {
            fprintf(stderr,"cannot read from src file %s\n",strerror(errno));
            exit(5);
        }
        for(int i=0;i < bytes_read; i++)
        {
            if(!isspace(buffer[i]))
                good_line = 1;
            if(buffer[i] == '\n')
            {
                if(good_line)
                {
                    copy_from_offsets(src_f,dst_f,last_line_start,block_start_offset + i + 1);
                }
                good_line = 0;
                last_line_start = block_start_offset + i + 1;
            }
        }
    } while (bytes_read>0);
    if(good_line)
    {
        copy_from_offsets(src_f,dst_f,last_line_start, ftell(src_f));
    }
}

int main(int argc,char** argv)
{
    char * src;
    char* dest;
    int from_args = 0;
    if(argc == 3)
    {
        src = argv[1];
        dest = argv[2];
        from_args = 1;
    }
    else
    {
        char buff[512];
        scanf("%511s",buff);
        src = malloc(strlen(buff));
        strcpy(src,buff);
        scanf("%511s",buff);
        dest = malloc(strlen(buff));
        strcpy(dest,buff);

    }
    FILE* src_f = fopen(src,"rb");

    if(!src_f)
    {
        fprintf(stderr,"error cannot open source file %s, %s\n",src,strerror(errno));
        exit(1);
    }
    FILE* dst_f = fopen(dest,"wb");
    if(!dst_f)
    {
        fprintf(stderr,"error cannot open destination file %s, %s\n",dest,strerror(errno));
        exit(2);
    }
    copy_without_blank_lines(src_f,dst_f);

    fclose(src_f);
    fclose(dst_f);

    if(!from_args)
    {
        free(src);
        free(dest);
    }
}
