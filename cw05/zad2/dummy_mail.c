#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if(argc <= 2)
        exit(-1);
    if(!strcmp(argv[1],"in"))
    {
        char buff[512];
        int bytes_read = 0;
        FILE* file = fopen(argv[2],"w");
        while((bytes_read = fread(buff,1,512,stdin)) > 0)
            fwrite(buff,1,bytes_read,file);
        fclose(file);
    }
    else if(!strcmp(argv[1],"out"))
    {
        char buff[512];
        int bytes_read;
        for(int i = 2;i< argc;i++) {
            FILE *file = fopen(argv[i], "r");
            while ((bytes_read = fread(buff, 1, 512, file)) > 0)
                fwrite(buff, 1, bytes_read, stdout);
            fclose(file);
        }
    }
}