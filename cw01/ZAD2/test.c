#include <sys/times.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#ifdef DYNAMIC
    #include <dlfcn.h>
#else
    #include <wc_store.h>
#endif
void print_times(clock_t start_time,clock_t stop_time,struct tms* start_struct,struct tms* stop_struct)
{
    clock_t tps = sysconf(_SC_CLK_TCK);
    printf("Real Time: %Lf, User Time %Lf, System Time %Lf\n",
           (long double)(stop_time - start_time)/tps,
           (long double)(stop_struct->tms_utime - start_struct->tms_utime)/tps,
           (long double)(stop_struct->tms_stime - start_struct->tms_stime)/tps);
}
int main(int argc, char** argv)
{
    struct allocation_table* table = create_allocation_table();
    struct tms start_struct;
    clock_t start_time;
    struct tms stop_struct;
    clock_t stop_time;

    for(int i=1;i<argc;i++)
    {
        if(!strcmp(argv[i],"wc_files"))
        {
            int n = atoi(argv[i + 1]);
            size_t size = 1;//for \0
            for(int j = 0;j < n; j++)
                size += strlen(argv[i + 2 + j]) + 1;
            char* buff = malloc(size);
                buff[0] = '\0';
            for(int j = 0;j < n; j++)
            {
                strcat(buff,argv[i + 2 + j]);
                strcat(buff," ");
            }
            printf("counting %d files\n",n);
            start_time = times(&start_struct);
            const char* tmpfile = count_words_lines_chars(buff);
            stop_time = times(&start_struct);
            print_times(start_time,stop_time,&start_struct,&stop_struct);
            printf("storing result\n");
            start_time = times(&start_struct);
            int index = store_file(table,tmpfile);
            stop_time = times(&start_struct);
            printf("stored at index %d\n",index);
            print_times(start_time,stop_time,&start_struct,&stop_struct);
            i+= n + 1;
        }
        else if(!strcmp(argv[i],"delete_block"))
        {
            int index = atoi(argv[i + 1]);
            printf("deleting block %d\n",index);
            start_time = times(&start_struct);
            free_block(table,index);
            stop_time = times(&stop_struct);
            print_times(start_time,stop_time,&start_struct,&stop_struct);

            i++;
        }
        else if(!strcmp(argv[i],"store_file_repeated"))
        {
            int n = atoi(argv[i + 1]);
            printf("storing and deleting file %s %d times\n",argv[i + 2],n);
            start_time = times(&start_struct);
            while(n--)
            {
                int index = store_file(table,argv[i + 2]);
                free_block(table,index);
            }
            stop_time = times(&stop_struct);
            print_times(start_time,stop_time,&start_struct,&stop_struct);

            i+=2;
        }
    }
    return 0;
}