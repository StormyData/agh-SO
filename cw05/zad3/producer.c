#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv)
{
    if(argc != 5)
    {
        printf("this program requires 4 arguments, path to fifo, row number, read file, N\n");
        exit(-1);
    }
    int ln = atoi(argv[2]);
    int n = atoi(argv[4]);
    FILE* named_fifo = fopen(argv[1], "w");
    FILE* file = fopen(argv[3],"r");
    char* buffer = malloc(sizeof(int) + n);
    *((int*)buffer) = ln;
    size_t bytes_read;
    while((bytes_read = fread(buffer + sizeof(int),1,n,file)) > 0)
    {
        memset(buffer + sizeof(int) + bytes_read, 0, n - bytes_read);
        fwrite(buffer,1,n + sizeof(int),named_fifo);
        fflush(named_fifo);
        sleep(rand()%3);
    }
    fclose(named_fifo);
    fclose(file);
    free(buffer);
    return 0;
}
