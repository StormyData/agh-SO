#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void dummy_read(char* arg)
{
    char* begin = "./dummy_mail out ";
    char* command = malloc(strlen(begin) + strlen(arg) + 1);
    strcpy(command, begin);
    strcat(command, arg);
    FILE* stream = popen(command,"r");
    char buffer[512];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, 512, stream)) > 0)
        fwrite(buffer, 1, bytes_read, stdout);
    pclose(stream);
}
void dummy_write(char* arg1, char* arg2, char* arg3)
{
    char* begin = "./dummy_mail in ";
    char* command = malloc(strlen(begin) + strlen(arg1) + 1);
    strcpy(command, begin);
    strcat(command, arg1);
    FILE* stream = popen(command,"w");
    fprintf(stream, "%s, %s\n", arg2, arg3);
    pclose(stream);

}

int main(int argc, char** argv)
{
    if(argc != 2 && argc != 4)
    {
        printf("this program requires one or three arguments\n");
        exit(-1);
    }
    if(argc == 2)
        dummy_read(argv[1]);
    else if(argc == 4)
        dummy_write(argv[1], argv[2], argv[3]);
}
