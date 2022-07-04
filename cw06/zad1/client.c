#include <sys/msg.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <time.h>

#include "common.h"



int our_id = -1;
int send_queue_id;
int recv_queue_id = -1;
key_t our_key;
char quitting = 0;

void process_msg(struct server_to_client_msg* msg)
{
    char time_buff[100];
    struct tm structtm;
    localtime_r(&msg->data.when,&structtm);
    strftime(time_buff, 100,"%c",&structtm);;
    switch (msg->mtype) {
        case INIT_RESPONSE:
            our_id = msg->data.payload.identifier;
            printf("%s: [server]: your id is %d\n",time_buff ,our_id);
            break;
        case USERLIST:
            printf("%s:\nactive users\n", time_buff);
            for(int i = 0;i< msg->data.payload.clients.n;i++)
            {
                int id = msg->data.payload.clients.list[i];
                if(id == our_id)
                    printf("[%d] (you)\n", id);
                else
                    printf("[%d]\n", id);
            }
            printf("%d users in total\n", msg->data.payload.clients.n);
            break;
        case METAMSG:
            printf("%s: [server]: %s\n",time_buff, msg->data.payload.msg.text);
            break;
        case BROADCAST_MSG:
            printf("%s: [%d] to all: %s\n", time_buff,msg->data.payload.msg.client, msg->data.payload.msg.text);
            break;
        case MSG:
            if(msg->data.payload.msg.client == our_id)
                printf("%s: you to yourself: %s\n",time_buff, msg->data.payload.msg.text);
            else
                printf("%s: [%d] to you: %s\n", time_buff,msg->data.payload.msg.client, msg->data.payload.msg.text);
            break;
        case QUIT:
            printf("%s: [server]: stopping\n",time_buff);
            quitting = 1;
            break;

    }
}

void on_sigterm(int sig)
{
    quitting = 1;
}

void cleanup()
{
    //printf("calling cleanup\n");
    if(our_id >= 0)
    {
        struct client_to_server_msg buff;
        buff.mtype = STOP;
        buff.data.from = our_id;
        msgsnd(send_queue_id,&buff, sizeof (struct client_to_Server_msg_data),0);
    }

    if(recv_queue_id>=0)
    {
        msgctl(recv_queue_id,IPC_RMID,NULL);
    }

}
void send_init()
{
    struct client_to_server_msg msg;
    msg.mtype = INIT;
    msg.data.payload.key = our_key;
    int stat = msgsnd(send_queue_id,&msg,sizeof(struct client_to_Server_msg_data),0);
    if(stat)
    {
        printf("send error %s\n", strerror(errno));
    }
}
int main(int argc, char ** argv)
{
    server_key = ftok(getenv("HOME"),0);
    srand(time(NULL));
    int project_id = rand() + 1;
    if(argc == 2)
        project_id = atoi(argv[1]);
    our_key = ftok(getenv("HOME"),project_id);
    send_queue_id = msgget(server_key, S_IWUSR|S_IWGRP|S_IWOTH);
    //printf("servers queue id %d\n", send_queue_id);
    if(send_queue_id < 0)
    {
        if(errno == ENOENT)
        {
            printf("cannot open connection, server is probably stopped\n");
        }
        else
            printf("cannot open send queue %s\n", strerror(errno));
        exit(errno);
    }
    atexit(cleanup);
    recv_queue_id = msgget(our_key, IPC_CREAT|IPC_EXCL|S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH);
    //printf("our queue id %d\n", recv_queue_id);
    if(recv_queue_id < 0)
    {
        printf("cannot open recv queue %s\n", strerror(errno));
        exit(errno);
    }
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = on_sigterm;
    sigaction(SIGINT,&sa,NULL);
    send_init();
    //printf("init sent\n");
    struct pollfd pollfd;
    pollfd.fd = STDIN_FILENO;
    pollfd.events = POLL_IN;
    char* line = NULL;
    size_t n = 0;
    while (!quitting)
    {
        struct server_to_client_msg recv_mesg;
        ssize_t stat = msgrcv(recv_queue_id,&recv_mesg,
                          sizeof (struct server_to_client_msg_data), 0, IPC_NOWAIT);
        if(stat < 0 && errno != ENOMSG)
        {
            fprintf(stderr,"receive error %s\n", strerror(errno));
        }
        if(stat >=0)
        {
            process_msg(&recv_mesg);

        }
        pollfd.revents = 0;
        stat = poll(&pollfd,1,0);
        if(stat < 0)
        {
            fprintf(stderr,"poll error %s\n", strerror(errno));
        }
        if(stat > 0)
        {
            getline(&line,&n,stdin);
            size_t len = strlen(line);
            if(len < 2)
                continue;
            if(line[len - 1] == '\n')
                line[len - 1] = '\0';
            char* tok_buff = NULL;
            char* line_end = line + strlen(line);
            char* cmd = strtok_r(line," ",&tok_buff);
            char* cmd_end = cmd + strlen(cmd);
            if(cmd == NULL)
                continue;
            if(!strcmp(cmd,"2ALL"))
            {
                struct client_to_server_msg msg;
                msg.mtype = TOALL;
                msg.data.from = our_id;
                char* rest = cmd_end + 1;
                if(cmd_end == line_end)
                {
                    printf("no message to send\n");

                }
                else if(strlen(rest) >= MAX_TEXT_MSG_LEN)
                {
                    printf("message too long, cannot send\n");
                }
                else
                {
                    strncpy(msg.data.payload.msg.text, rest, MAX_TEXT_MSG_LEN);
                    msgsnd(send_queue_id,&msg,sizeof (struct client_to_Server_msg_data), 0);
                }

            }
            else if(!strcmp(cmd, "2ONE"))
            {
                struct client_to_server_msg msg;
                if(cmd_end == line_end)
                {
                    printf("no id to send to\n");
                    continue;
                }
                char* to_str  = strtok_r(NULL, " ", &tok_buff);
                char* to_str_end = to_str + strlen(to_str);
                int to_id = (int)strtol(to_str,NULL,0);
                msg.mtype = TOONE;
                msg.data.from = our_id;
                msg.data.payload.msg.client = to_id;
                if(to_str_end == line_end)
                {
                    printf("no message to send\n");
                    continue;
                }
                char* rest = to_str_end + 1;

                if(strlen(rest) >= MAX_TEXT_MSG_LEN)
                {
                    printf("message too long, cannot send\n");
                }
                else
                {
                    strncpy(msg.data.payload.msg.text, rest, MAX_TEXT_MSG_LEN);
                    msgsnd(send_queue_id,&msg,sizeof (struct client_to_Server_msg_data), 0);
                }

            }
            else if(!strcmp(cmd, "LIST"))
            {
                struct client_to_server_msg msg;
                msg.mtype = LIST;
                msg.data.from = our_id;
                msgsnd(send_queue_id,&msg,sizeof (struct client_to_Server_msg_data), 0);
            }
            else
            {
                printf("unknown command %s", cmd);
            }
        }
        usleep(50000);

    }
    if(n > 0)
        free(line);
}
