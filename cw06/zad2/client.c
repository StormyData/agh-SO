#include <mqueue.h>
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
mqd_t send_queue_id;
mqd_t recv_queue_id = -1;
char our_key[M_NAME_MAX];
char quitting = 0;
pid_t child;

void on_sigusr1_child(int signo, siginfo_t *info, void *context)
{
    our_id = info->si_value.sival_int;
}
void process_msg(struct server_to_client_msg* msg)
{
    char time_buff[100];
    struct tm structtm;
    localtime_r(&msg->data.when,&structtm);
    strftime(time_buff, 100,"%c",&structtm);;
    switch (msg->mtype) {
        case INIT_RESPONSE:
            our_id = msg->data.payload.identifier;
            union sigval sv;
            sv.sival_int = our_id;
            sigqueue(child,SIGUSR1,sv);
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
void on_sigterm_parent(int sig)
{
    quitting = 1;
    kill(child,sig);
}

void send_init()
{
    struct client_to_server_msg msg;
    msg.mtype = INIT;
    strncpy(msg.data.payload.key,our_key,M_NAME_MAX);
    int stat = mq_send(send_queue_id,(char*)&msg,sizeof(struct client_to_server_msg),INIT);
    if(stat)
    {
        printf("send error %s\n", strerror(errno));
    }
}
void poll_queue()
{
    struct server_to_client_msg recv_mesg;
    ssize_t stat = mq_receive(recv_queue_id,(char*)&recv_mesg,
                              sizeof (struct server_to_client_msg), NULL);
    if(stat < 0 && errno != EAGAIN && errno != EINTR)
    {
        fprintf(stderr,"receive error %s\n", strerror(errno));
    }
    if(stat >= 0)
    {
        process_msg(&recv_mesg);
    }
}
void poll_stdin()
{
    char* line = NULL;
    size_t n = 0;

    struct pollfd pollfd;
    pollfd.fd = STDIN_FILENO;
    pollfd.events = POLL_IN;

    pollfd.revents = 0;
    int stat = poll(&pollfd,1,0);
    if(stat < 0)
    {
        fprintf(stderr,"poll error %s\n", strerror(errno));
    }
    if(stat > 0)
    {
        getline(&line, &n,stdin);
        size_t len = strlen(line);
        if(len < 2)
            return;
        if(line[len - 1] == '\n')
            line[len - 1] = '\0';
        char* tok_buff = NULL;
        char* line_end = line + strlen(line);
        char* cmd = strtok_r(line," ",&tok_buff);
        char* cmd_end = cmd + strlen(cmd);
        if(cmd == NULL)
            return;
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
                mq_send(send_queue_id,(char*)&msg,sizeof (struct client_to_server_msg), TOALL);
            }

        }
        else if(!strcmp(cmd, "2ONE"))
        {
            struct client_to_server_msg msg;
            if(cmd_end == line_end)
            {
                printf("no id to send to\n");
                return;
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
                return;
            }
            char* rest = to_str_end + 1;

            if(strlen(rest) >= MAX_TEXT_MSG_LEN)
            {
                printf("message too long, cannot send\n");
            }
            else
            {
                strncpy(msg.data.payload.msg.text, rest, MAX_TEXT_MSG_LEN);
                mq_send(send_queue_id,(char*)&msg,sizeof (struct client_to_server_msg),TOONE);
            }

        }
        else if(!strcmp(cmd, "LIST"))
        {
            struct client_to_server_msg msg;
            msg.mtype = LIST;
            msg.data.from = our_id;
            mq_send(send_queue_id,(char*)&msg,sizeof (struct client_to_server_msg), LIST);
        }
        else
        {
            printf("unknown command %s", cmd);
        }
    }
    if(n > 0)
        free(line);
    usleep(50000);
}

void cleanup_sender()
{
    //printf("calling cleanup\n");
    if(our_id >= 0) {
        struct client_to_server_msg buff;
        buff.mtype = STOP;
        buff.data.from = our_id;
        mq_send(send_queue_id, (char *) &buff, sizeof(struct client_to_server_msg), STOP);
        mq_close(send_queue_id);
    }
}
void cleanup_receiver()
{
    if(recv_queue_id>=0)
    {
        mq_unlink(our_key);
        mq_close(recv_queue_id);
    }
}
int main(int argc, char ** argv)
{
    srand(time(NULL));
    int project_id = rand() + 1;
    if(argc == 2)
        project_id = atoi(argv[1]);
    sprintf(our_key,"/systemy_operacyjne_cw06_zad2_client%d",project_id);


    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = on_sigterm;
    sigaction(SIGINT,&sa,NULL);

    atexit(cleanup_sender);

    send_queue_id = mq_open(server_key, O_WRONLY);
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

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct server_to_client_msg);

    atexit(cleanup_receiver);

    recv_queue_id = mq_open(our_key, O_CREAT|O_EXCL|O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH,&attr);
    //printf("our queue id %d\n", recv_queue_id);
    if(recv_queue_id < 0)
    {
        printf("cannot open recv queue %s\n", strerror(errno));
        exit(errno);
    }


    send_init();

    //printf("init sent\n");

    if((child = fork()) == 0)
    {
        //child
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = on_sigusr1_child;
        sigaction(SIGUSR1,&sa,NULL);

        //sender
        mq_close(recv_queue_id);
        while (!quitting)
            poll_stdin();
    }
    else
    {
        //parent
        sa.sa_handler = on_sigterm_parent;
        sigaction(SIGINT,&sa,NULL);

        //receiver
        mq_close(send_queue_id);
        while (!quitting)
            poll_queue();
        kill(child, SIGINT);
    }
}