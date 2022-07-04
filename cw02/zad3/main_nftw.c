#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ftw.h>
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


int fn(const char *fpath, const struct stat *sb,
          int typeflag, struct FTW *ftwbuf)
{
    if(typeflag == FTW_D)
        return 0;
    printf("path: %s\t",fpath);
    char* type_ptr;
    switch (typeflag) {
        case FTW_F:
        {
            switch (sb->st_mode & S_IFMT) {
                case S_IFSOCK:
                    type_ptr = "socket";
                    nof_sockets++;
                    break;
                case S_IFLNK:
                    type_ptr = "link";
                    nof_links++;
                    break;
                case S_IFREG:
                    type_ptr = "file";
                    nof_regular_files++;
                    break;
                case S_IFBLK:
                    nof_block_devices++;
                    type_ptr = "block device";
                    break;
                case S_IFCHR:
                    nof_character_devices++;
                    type_ptr = "character device";
                    break;
                case S_IFIFO:
                    nof_fifo++;
                    type_ptr = "fifo";
                    break;

            }
        }

            break;
        case FTW_DP:
            type_ptr = "directory";
            nof_directories++;
            break;
            case FTW_DNR:
                nof_directories++;
            type_ptr = "directory (unreadable)";
            break;
        case FTW_SL:
            nof_links++;
            type_ptr = "symbolic link";
            break;
        case FTW_NS:
            type_ptr = "unknown";
            break;

    }
    printf("type: %s\t",type_ptr);
    if(typeflag == FTW_NS)
    {
        printf("stats: unknown\n");
        return 0;
    }
    print_stats(sb);

    return 0;
}
int main(int argc,char** argv)
{
    if(argc != 2)
    {
        perror("this program requires exactly 1 argument: path to start of the directory tree");
        exit(1);
    }
    char* rpath = realpath(argv[1],NULL);
    nftw(rpath,fn,100,FTW_PHYS|FTW_DEPTH);
    free(rpath);
    printf("%d regular files found\n",nof_regular_files);
    printf("%d directories found\n",nof_directories);
    printf("%d block devices found\n",nof_block_devices);
    printf("%d character devices found\n",nof_character_devices);
    printf("%d fifo found\n",nof_fifo);
    printf("%d links found\n",nof_links);
    printf("%d sockets found\n",nof_sockets);
}
