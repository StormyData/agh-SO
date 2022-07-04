#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
int nof_regular_files = 0;
int nof_directories = 0;
int nof_block_devices = 0;
int nof_character_devices = 0;
int nof_fifo = 0;
int nof_sockets = 0;
int nof_links = 0;

void print_stats(const struct stat* sb)
{
    if(sb == NULL)
    {
        printf("stats: unknown\n");
        return;
    }
    char* type;
    switch (sb->st_mode & S_IFMT) {
        case S_IFSOCK:
            type = "socket";
            nof_sockets++;
            break;
        case S_IFLNK:
            type = "link";
            nof_links++;
            break;
        case S_IFREG:
            type = "file";
            nof_regular_files++;
            break;
        case S_IFBLK:
            type = "block device";
            nof_block_devices++;
            break;
        case S_IFCHR:
            type = "character device";
            nof_character_devices++;
            break;
        case S_IFIFO:
            type = "fifo";
            nof_fifo++;
            break;
        case S_IFDIR:
            type = "directory";
            nof_directories++;
    }

    printf("type: %s\t",type);

    char buffer[200];
    printf("stats: ");
    printf("nof hard links: %lu\t",sb->st_nlink);
    printf("size: %ld\t",sb->st_size);
    struct tm* time_ptr = localtime(&sb->st_atime);
    strftime(buffer,200,"%Y-%m-%d %H:%M:%S", time_ptr);

    printf("last access time: %s\t",buffer);

    time_ptr = localtime(&sb->st_mtime);
    strftime(buffer,200,"%Y-%m-%d %H:%M:%S", time_ptr);

    printf("last modification time: %s\n\n",buffer);
}

struct stack{
    struct stack* next;
    char* data;
};
void push(struct stack** top,const char* ref)
{
    struct stack* tmp = malloc(sizeof (struct stack));
    tmp->next = *top;
    tmp->data = malloc(strlen(ref) + 1);
    strcpy(tmp->data,ref);
    (*top) = tmp;
}
char* pop(struct stack** top)
{
    if((*top)->next == NULL)
    {
        return NULL;
    }
    char* data = (*top)->data;
    struct stack* tmp = (*top)->next;
    free((*top));
    (*top) = tmp;
    return data;
}

//printf("%s,%d\n",__func__ ,__LINE__);

void crawl(const char* begin)
{
    struct stack* top = malloc(sizeof (struct stack));
    top->data = NULL;
    push(&top,begin);
    while(top->next != NULL)
    {
        char* curr_dir_path = pop(&top);
        DIR* curr_dir = opendir(curr_dir_path);
        if(curr_dir == NULL)
        {
            fprintf(stderr,"cannot open directory %s, %s\n\n", curr_dir_path,strerror(errno));
            free(curr_dir_path);
            continue;
        }
        errno = 0;
        struct dirent* entry;
        while((entry = readdir(curr_dir)) != NULL)
        {
            if((!strcmp(entry->d_name,"."))||(!strcmp(entry->d_name,"..")))
                continue;
            char* next_path = malloc(strlen(curr_dir_path) + strlen(entry->d_name) + 2);
            strcpy(next_path,curr_dir_path);
            strcat(next_path,"/");
            strcat(next_path,entry->d_name);
            struct stat sb;
            if(lstat(next_path,&sb))
            {
                printf("path: %s\t",next_path);
                printf("error %s\t", strerror(errno));
                print_stats(NULL);
            }
            else
            {
                printf("path: %s\t",next_path);
                print_stats(&sb);
                if((sb.st_mode & S_IFMT) == S_IFDIR)
                    push(&top,next_path);
            }
        }
        if(errno)
        {
            fprintf(stderr,"unexpected error occurred while reading directory %s, %s\n\n",curr_dir_path,strerror(errno));
            free(curr_dir_path);
            continue;
        }

        free(curr_dir_path);
    }
}

int main(int argc,char** argv)
{
    if(argc != 2)
    {
        perror("this program requires exactly 1 argument: path to start of the directory tree");
        exit(1);
    }
    char* rpath = realpath(argv[1],NULL);
    crawl(rpath);
    free(rpath);
    printf("%d regular files found\n",nof_regular_files);
    printf("%d directories found\n",nof_directories);
    printf("%d block devices found\n",nof_block_devices);
    printf("%d character devices found\n",nof_character_devices);
    printf("%d fifo found\n",nof_fifo);
    printf("%d links found\n",nof_links);
    printf("%d sockets found\n",nof_sockets);
}
