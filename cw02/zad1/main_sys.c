#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#define BUFFSIZE 1024
void handle_lseek_error(off_t returned_value)
{
    if(returned_value == -1)
    {
        fprintf(stderr,"internal error %s\n",strerror(errno));
        exit(3);
    }
}
void copy_from_offsets(int src_fd,int dst_fd,off_t start_offset,off_t end_offset)
{
    if(end_offset<=start_offset)
        return;
    off_t curr_back = lseek(src_fd,0,SEEK_CUR);
    lseek(src_fd,start_offset,SEEK_SET);
    char buffer[BUFFSIZE];
    off_t left_to_write = end_offset - start_offset;
    while(left_to_write>0)
    {
        off_t to_write = left_to_write > BUFFSIZE ? BUFFSIZE : left_to_write;
        off_t bytes_read = read(src_fd,buffer,to_write);
        if(bytes_read == -1)
        {
            fprintf(stderr,"cannot read from src file %s\n",strerror(errno));
            exit(5);
        }
        int bytes_written=0;
        while (bytes_written<bytes_read)
        {
            int tmp=write(dst_fd,buffer + bytes_written,bytes_read - bytes_written);
            if(tmp==-1)
            {
                fprintf(stderr,"Error writing to destination file %s\n",strerror(errno));
                exit(4);
            }
            bytes_written +=tmp;
        }

        left_to_write -= bytes_read;
    }
    lseek(src_fd,curr_back,SEEK_SET);
}
void copy_without_blank_lines(int src_fd,int dst_fd)
{
    char buffer[BUFFSIZE];
    int bytes_read;
    int good_line = 0;
    off_t last_line_start = lseek(src_fd,0,SEEK_CUR);
    handle_lseek_error(last_line_start);
    do
    {
        off_t block_start_offset = lseek(src_fd,0,SEEK_CUR);
        bytes_read = read(src_fd,buffer,BUFFSIZE);
        if(bytes_read == -1)
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
                    copy_from_offsets(src_fd,dst_fd,last_line_start,block_start_offset + i + 1);
                }
                good_line = 0;
                last_line_start = block_start_offset + i + 1;
            }
        }
    } while (bytes_read>0);
    if(good_line)
    {
        copy_from_offsets(src_fd,dst_fd,last_line_start, lseek(src_fd,0,SEEK_CUR));
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

    int src_fd = open(src,O_RDONLY);
    if(!src_fd)
    {
        fprintf(stderr,"error cannot open source file %s, %s\n",src,strerror(errno));
        exit(1);
    }
    int dst_fd = open(dest,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
    if(!dst_fd)
    {
        fprintf(stderr,"error cannot open destination file %s, %s\n",dest,strerror(errno));
        exit(2);
    }
    copy_without_blank_lines(src_fd,dst_fd);

    close(src_fd);
    close(dst_fd);

    if(!from_args)
    {
        free(src);
        free(dest);
    }

}
