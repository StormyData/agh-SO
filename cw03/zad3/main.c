#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>


int is_text_file(const char* path)
{
    char* extensions[] = {".txt", ".c", ".cpp",""};
    int n_ext = sizeof(extensions)/sizeof(extensions[0]);

    int n = strlen(path);
    const char* end = path + n;
    for(int i = 0;i< n_ext;i++)
        if(!strcmp(end- strlen(extensions[i]),extensions[i]))
            return 1;
    return 0;
}

int file_find(FILE* file, const char* str)
{
    fseek(file,0,SEEK_END);
    int len = ftell(file);
    fseek(file,0,SEEK_SET);
    len -= ftell(file);
    char* buffer = malloc(len);
    fread(buffer,len,len,file);
    char* off = memmem(buffer,len,str, strlen(str));
    int pos = -1;
    if(off != NULL)
        pos = off - buffer;
    free(buffer);
    return pos;
}

void scan_file(const char* path, const char* rootdir, const char* searched_string)
{
    FILE* file = fopen(path,"r");
    int off = file_find(file,searched_string);
    const char* relpath = path + strlen(rootdir) + 1;
    if(off>=0)
    {
        printf("found in file %s , pid= %d, at %d byte\n",relpath, getpid(),off);
    }

}
void search_dir(const char* dirpath, const char* rootdir, const char* searched_string,int max_search_depth)
{
    if(max_search_depth < 0)
        return;
    DIR* curr_dir = opendir(dirpath);
    if(curr_dir == NULL)
    {
        fprintf(stderr,"cannot open directory %s, %s\n", dirpath,strerror(errno));
        exit(-2);
    }
    errno = 0;
    struct dirent* entry;
    while((entry = readdir(curr_dir)) != NULL)
    {
        if((!strcmp(entry->d_name,"."))||(!strcmp(entry->d_name,"..")))
            continue;
        char next_path[PATH_MAX];
        strcpy(next_path,dirpath);
        strcat(next_path,"/");
        strcat(next_path,entry->d_name);
        struct stat sb;
        if(lstat(next_path,&sb))
        {
            fprintf(stderr,"path: %s\t",next_path);
            fprintf(stderr,"error %s\n", strerror(errno));
            errno = 0;
        }
        else
        {
            if((sb.st_mode & S_IFMT) == S_IFDIR)
            {
                pid_t child = fork();
                if(child == 0)
                {
                    search_dir(next_path,rootdir,searched_string,max_search_depth - 1);
                    exit(0);
                }
            }
            else if((sb.st_mode & S_IFMT) == S_IFREG)
            {
                if(is_text_file(next_path))
                    scan_file(next_path, rootdir, searched_string);
            }
        }
    }
    if(errno)
    {
        fprintf(stderr,"unexpected error occurred while reading directory %s, %s\n",dirpath,strerror(errno));
    }
}

int main(int argc, char** argv)
{
    if(argc != 3 && argc != 4)
    {
        fprintf(stderr,"this program requires  2 or 3 arguments: path to directory and str to find, max search depth\n");
        exit(-1);
    }

    int max_search_depth = INT_MAX;
    if(argc == 4)
        max_search_depth =  atoi(argv[3]);
    char rpath[PATH_MAX];
    realpath(argv[1],rpath);
    search_dir(rpath,rpath,argv[2],max_search_depth);


}