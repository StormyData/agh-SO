#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
//#define DEBUG

struct strlistelem {
    char *val;
    struct strlistelem *next;
};
struct strlistelem* shallow_copy(struct strlistelem* base)
{
    struct strlistelem root;
    struct strlistelem* curr = &root;
    root.next = NULL;
    while(base != NULL)
    {
        curr->next = malloc(sizeof (struct strlistelem));
        curr = curr->next;
        curr->val = base->val;
        curr->next = NULL;
        base = base->next;
    }
    return root.next;
}
size_t list_len(struct strlistelem* base)
{
    size_t len = 0;
    while (base != NULL)
    {
        len++;
        base = base->next;
    }
    return len;
}
char** to_string_array(struct strlistelem* base)
{
    size_t len = list_len(base);
    char** arr = malloc((len + 1) * sizeof(char*));
    int i = 0;
    while (base != NULL)
    {
        arr[i] = base->val;
        base = base->next;
        i++;
    }
    arr[len] = NULL;
    return arr;
}
void shallow_free_list(struct strlistelem* base)
{
    while (base != NULL)
    {
        struct strlistelem* next = base->next;
        free(base);
        base = next;
    }
}

struct strlistelem* tokenize_to_list_delim(char *str, char* delim) {
    struct strlistelem root;
    root.next = NULL;
    root.val = NULL;
    struct strlistelem *curr = &root;
    char *tok_buff;
    for (char *token = strtok_r(str, delim, &tok_buff); token; token = strtok_r(NULL, delim, &tok_buff)) {
        curr->next = malloc(sizeof(struct strlistelem));
        curr->next->val = token;
        curr = curr->next;
        curr->next = NULL;
    }
    return root.next;
}

pid_t exec_with_params_and_pipes(char* path, struct strlistelem* args, int in_pipe_pair[2], int out_pipe_pair[2])
{

    pid_t child;
    char** arg_arr = to_string_array(args);
    if((child = fork()) == 0)
    {
        if(in_pipe_pair[0] >= 0)
            dup2(in_pipe_pair[0],STDIN_FILENO);
        if(in_pipe_pair[1] >= 0)
            close(in_pipe_pair[1]);
        if(out_pipe_pair[0] >= 0)
            close(out_pipe_pair[0]);
        if(out_pipe_pair[1] >= 0)
            dup2(out_pipe_pair[1],STDOUT_FILENO);
        execvp(path, arg_arr);
        exit(0);
    }
    free(arg_arr);
    return child;
}
void print_list(struct strlistelem* base, const char* delim)
{
    while(base != NULL)
    {
        printf("%s",base->val);
        if(base->next != NULL)
            printf("%s", delim);
        base = base->next;
    }
}

void exec_pipeline(struct strlistelem* base)
{
    if(base == NULL)
        return;
    int last[2];
    int curr[2];
    last[0] = -1;
    last[1] = -1;
    curr[0] = -1;
    curr[1] = -1;
    if(base->next != NULL)
        pipe(curr);
    size_t len = list_len(base);
    pid_t* arr = malloc(len * sizeof(pid_t));
    int i = 0;
    while(base != NULL)
    {
        char* command_copy = malloc(strlen(base->val) + 1);
        strcpy(command_copy,base->val);
        struct strlistelem* root_args = tokenize_to_list_delim(command_copy," ");
        char* path = root_args->val;
#ifdef DEBUG
        printf("creating process %s with args [", path);
        print_list(root_args,", ");
#endif
        arr[i] = exec_with_params_and_pipes(path, root_args, last, curr);
#ifdef DEBUG
        printf("], PID = %d\n",arr[i]);
#endif
        i++;
        shallow_free_list(root_args);
        if(last[0] >= 0)
            close(last[0]);
        if(last[1] >= 0)
            close(last[1]);
        last[0] = curr[0];
        last[1] = curr[1];
        if(base->next != NULL)
            pipe(curr);
        base = base->next;
        free(command_copy);
    }
    close(last[1]);
    char buff[1024];
    ssize_t bytes_read;
    while((bytes_read = read(last[0],buff,1024)) > 0)
    {
        fwrite(buff,1,bytes_read,stdout);
    }
    fflush(stdout);
    close(curr[0]);
    for(i=0;i<len;)
    {
        int wstatus;
        waitpid(arr[i],&wstatus,0);
        if(!WIFSTOPPED(wstatus))
            i++;
    }
    free(arr);
}

struct vector_dict
{
    char** keys;
    struct strlistelem** values;
    size_t size;
    size_t capacity;
};

void vector_dict_init(struct vector_dict* vectorDict)
{
    size_t initial_size = 10;
    vectorDict->keys = malloc(initial_size * sizeof(char*));
    vectorDict->values = malloc(initial_size * sizeof (struct strlistelem*));
    vectorDict->size = 0;
    vectorDict->capacity = initial_size;
}
void vector_dict_free(struct vector_dict* vectorDict)
{
    for(int i=0;i<vectorDict->size;i++)
        free(vectorDict->keys[i]);
    free(vectorDict->keys);
    free(vectorDict->values);
}
void vector_dict_set(struct vector_dict* vectorDict, char* key, struct strlistelem* value)
{
    for(int i = 0; i< vectorDict->size; i++)
    {
        if(!strcmp(vectorDict->keys[i], key))
        {
            vectorDict->values[i] = value;
            return;
        }
    }
    if(vectorDict->size == vectorDict->capacity)
    {
        size_t new_capacity = 2 * vectorDict->capacity;
        vectorDict->keys = realloc(vectorDict->keys, new_capacity * sizeof(char*));
        vectorDict->values = realloc(vectorDict->values, new_capacity * sizeof(struct strlistelem*));
        vectorDict->capacity = new_capacity;
    }
    vectorDict->keys[vectorDict->size] = malloc(strlen(key) + 1);
    strcpy(vectorDict->keys[vectorDict->size],key);
    vectorDict->values[vectorDict->size] = value;
    vectorDict->size++;

}
struct strlistelem* vector_dict_get(struct vector_dict* vectorDict, char* key)
{
    for(int i = 0; i< vectorDict->size; i++)
    {
        if(!strcmp(vectorDict->keys[i], key))
            return vectorDict->values[i];
    }
    return NULL;
}
char* trim_whitespaces(char* str)
{
    while (*str == ' ') str++;
    char* end = str + strlen(str) - 1;
    while (end > str && *end == ' ') end--;
    if(str == end)
        return NULL;
    *(end + 1) = '\0';
    return str;
}
void exec_line(struct vector_dict* aliases, char* line)
{
    struct strlistelem* linelist = tokenize_to_list_delim(line, "|");
    struct strlistelem* list = NULL;
    struct strlistelem* head = NULL;
    while (linelist != NULL)
    {
        char* trimmed = trim_whitespaces(linelist->val);
#ifdef DEBUG
        printf("testing for presence of '%s' in the alias database: ", trimmed);
#endif
        struct strlistelem* data = vector_dict_get(aliases,trimmed);
        if(data == NULL)
        {
            #ifdef DEBUG
            printf("not found adding as command\n");
#endif
            struct strlistelem* new_elem = malloc(sizeof(struct strlistelem));
            new_elem->val = trimmed;
            new_elem->next = NULL;
            if(list == NULL)
            {
                list = new_elem;
                head = new_elem;
            } else{
                head->next = new_elem;
                head = new_elem;
            }

        }
        else
        {
            struct strlistelem* new_elem = shallow_copy(data);

#ifdef DEBUG
            printf("found, appending [");
            print_list(new_elem, " | ");
            printf("]\n");
#endif
            if(list == NULL)
            {
                list = new_elem;
                head = new_elem;
            }
            else
            {
                head->next = new_elem;
            }
            while (head->next != NULL)
                head = head->next;
        }
        linelist= linelist->next;
    }
#ifdef DEBUG
    printf("executing command pipeline [");
    print_list(list, " | ");
    printf("]\n");
#endif
    exec_pipeline(list);
}
void analyze_line(struct vector_dict* aliases, char* line_str)
{
#ifdef DEBUG
    printf("begining line analysis [%s]\n", line_str);
#endif
    struct strlistelem* line_list = tokenize_to_list_delim(line_str,"=");
    size_t ll = list_len(line_list);
    if(ll == 0)
        return;
    if(ll == 1)
    {
        exec_line(aliases, line_list->val);
        return;
    }
    struct strlistelem* tmp = line_list;
    while (tmp->next != NULL)
        tmp = tmp->next;
    struct strlistelem* data = tokenize_to_list_delim(tmp->val,"|");
    tmp = line_list;
    while (tmp->next != NULL)
    {
        char* trimmed = trim_whitespaces(tmp->val);
        #ifdef DEBUG
        printf("making alias for '%s' = [", trimmed);
        print_list(data, " | ");
        printf("]\n");
#endif
        vector_dict_set(aliases, trimmed, data);
        tmp = tmp->next;
    }
}

int main(int argc, char** argv) {
    if(argc != 2)
    {
        printf("this program requires exactly one argument - path to file\n");
        exit(-1);
    }
    FILE *file = fopen(argv[1], "r");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *str = malloc(length + 1);
    fread(str, 1, length, file);
    str[length] = '\0';
    struct vector_dict aliases;
    vector_dict_init(&aliases);
    struct strlistelem* lines = tokenize_to_list_delim(str, "\n");
    while (lines != NULL)
    {
        analyze_line(&aliases, lines->val);
        lines = lines->next;
    }
    vector_dict_free(&aliases);
    shallow_free_list(lines);
    free(str);
}
