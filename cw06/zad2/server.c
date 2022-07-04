#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#include "common.h"

int LOGGING_LEVEL = 1;

mqd_t client_queue_id[MAX_NOF_CLIENTS];
int n_clients = 0;
mqd_t queue_id = -1;
char quiting = 0;
FILE* logfile = NULL;

void logger(char * msg, int level)
{
    if(level < LOGGING_LEVEL)
        return;
    char level_strings[4][6] = {"DEBUG","INFO ","WARN ","ERROR"};
    char time_buff[100];
    struct tm structtm;
    time_t timer = time(NULL);
    localtime_r(&timer,&structtm);
    strftime(time_buff, 100,"%c",&structtm);
    char* level_str = "INLEV";
    if(level >= 0 && level < 4)
    {
        level_str = level_strings[level];
    }
    if(logfile != NULL)
        fprintf(logfile,"%s :: %s :: %s\n", level_str, time_buff, msg );
    fprintf(stdout,"%s :: %s :: %s\n", level_str, time_buff, msg);

}


int add_client(char* client_path)
{
    int i;
    for(i=0;i<MAX_NOF_CLIENTS;i++)
    {
        if(client_queue_id[i]<0)
            break;
    }
    if(i == MAX_NOF_CLIENTS)
    {
        logger("cannot add any more clients\n", 3);
        return -1;
    }
    client_queue_id[i] = mq_open(client_path,O_WRONLY);
    if(client_queue_id < 0)
        return -1;
    n_clients++;
    return i;
}
void remove_client(mqd_t client)
{
    if(client<0 || client >= MAX_NOF_CLIENTS || client_queue_id[client] < 0)
    {
        return;
    }
    mq_close(client_queue_id[client]);
    client_queue_id[client] = -1;
    n_clients--;
}
void send_init_response_to(int client, time_t time)
{
    if(client < 0 || client >= MAX_NOF_CLIENTS)
        return;
    mqd_t send_queue_id = client_queue_id[client];
    struct server_to_client_msg buffer;
    buffer.mtype = INIT_RESPONSE;
    buffer.data.when = time;
    buffer.data.payload.identifier = client;
    mq_send(send_queue_id,(char*) &buffer,sizeof (struct server_to_client_msg),INIT_RESPONSE);
}
void send_listing_to(int to, time_t time)
{
    if(to<0 || to >= MAX_NOF_CLIENTS)
    {
        return;
    }
    mqd_t send_queue_id = client_queue_id[to];
    struct server_to_client_msg buffer;
    buffer.mtype = USERLIST;
    buffer.data.when = time;
    buffer.data.payload.clients.n = 0;
    for(int i=0;i<MAX_NOF_CLIENTS;i++)
    {
        if(client_queue_id[i] >= 0)
        {
            buffer.data.payload.clients.list[buffer.data.payload.clients.n] = i;
            buffer.data.payload.clients.n++;
        }
    }
    mq_send(send_queue_id,(char*)&buffer, sizeof(struct server_to_client_msg),USERLIST);
}

void send_msg_to(int from, int to, int type,time_t  time, char msg[MAX_NOF_CLIENTS])
{
    if(to<0 || to >= MAX_NOF_CLIENTS)
    {
        return;
    }
    mqd_t send_queue_id = client_queue_id[to];
    struct server_to_client_msg buffer;
    buffer.mtype = type;
    buffer.data.payload.msg.client = from;
    buffer.data.when = time;
    strcpy(buffer.data.payload.msg.text, msg);
    mq_send(send_queue_id,(char*)&buffer,sizeof(struct server_to_client_msg), buffer.mtype);
}
void send_msg_to_all(int from, int type,time_t time, char msg[MAX_NOF_CLIENTS])
{
    for(int i=0;i<MAX_NOF_CLIENTS;i++)
        if(client_queue_id[i]>=0)
            send_msg_to(from, i, type, time, msg);
}

void cleanup(void)
{
    if(queue_id>=0)
    {
        mq_unlink(server_key);
        mq_close(queue_id);
    }
    if(logfile != NULL)
        fclose(logfile);
}
void send_stop_msg_to(int client, time_t time)
{
    if(client<0 || client >= MAX_NOF_CLIENTS)
        return;
    mqd_t send_queue_id = client_queue_id[client];
    struct server_to_client_msg buffer;
    buffer.mtype = QUIT;
    buffer.data.when = time;
    mq_send(send_queue_id,(char*)&buffer, sizeof(struct server_to_client_msg),QUIT);
}
void on_sigterm(int sig)
{
    if(!quiting)
    {
        quiting = 1;
        time_t now = time(NULL);
        for(int i=0;i<MAX_NOF_CLIENTS;i++)
            if(client_queue_id[i]>=0)
                send_stop_msg_to(i, now);
        logger("stopping server", 1);
    }

}
int main(int argc, char** argv)
{
    logfile = fopen("log.txt","w");
    if(logfile == NULL)
        logger("error opening logfile", 3);

    char buffer[MAX_TEXT_MSG_LEN + 100];
    logger("server starting",1);
    for(int i=0;i<MAX_NOF_CLIENTS;i++)
        client_queue_id[i] = -1;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct client_to_server_msg);
    queue_id = mq_open(server_key,O_CREAT|O_EXCL|O_RDONLY,
                       S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH, &attr);

    snprintf(buffer, MAX_TEXT_MSG_LEN + 100,"master queue id is %d", queue_id);
    logger(buffer, 0);
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = on_sigterm;
    sigaction(SIGINT,&sa,NULL);
    atexit(cleanup);
    if(queue_id < 0)
    {
        snprintf(buffer, MAX_TEXT_MSG_LEN + 100,"cannot create a master queue: %s",strerror(errno));

        logger(buffer,3);
        exit(errno);

    }
    logger("queue created",0);
    struct client_to_server_msg recv_msg;
    while (n_clients > 0 || !quiting)
    {
        if(mq_receive(queue_id,(char*)&recv_msg, sizeof(struct client_to_server_msg),
                  NULL) < 0)
        {
            if(errno != EINTR)
            {
                snprintf(buffer, MAX_TEXT_MSG_LEN + 100,"error reading message: %s",strerror(errno));
                logger(buffer,3);
                continue;
            }
        }
        if(quiting && recv_msg.mtype != STOP)
            continue;
        int from = recv_msg.data.from;
        int client_id;
        switch (recv_msg.mtype) {
            case STOP:
                remove_client(from);
                sprintf(buffer, "user %d left the server", from);
                send_msg_to_all(0,METAMSG, time(NULL), buffer);
                logger(buffer,1);
                break;
            case INIT:
                client_id = add_client(recv_msg.data.payload.key);
                sprintf(buffer, "user %d joined the server", client_id);
                send_init_response_to(client_id, time(NULL));
                send_msg_to_all(-1,METAMSG,time(NULL), buffer);
                logger(buffer,1);
                sprintf(buffer, "user %d queue id %d", client_id, client_queue_id[client_id]);
                logger(buffer, 0);
                break;
            case TOALL:
                if(n_clients < 2)
                {
                    snprintf(buffer,MAX_TEXT_MSG_LEN + 100, "you are alone on the server");
                    send_msg_to(-1, from,METAMSG , time(NULL),
                                buffer);
                    snprintf(buffer,MAX_TEXT_MSG_LEN + 100, "user %d tried to say [%s] to all when they were alone on the server", from, recv_msg.data.payload.msg.text);
                    logger(buffer, 1);

                }
                else
                {
                    snprintf(buffer,MAX_TEXT_MSG_LEN + 100, "user %d said [%s] to all", from, recv_msg.data.payload.msg.text);
                    send_msg_to_all(from,BROADCAST_MSG, time(NULL), recv_msg.data.payload.msg.text);
                    logger(buffer,1);
                }
                break;
            case TOONE:
            {
                int to = recv_msg.data.payload.msg.client;
                printf("to %d\n", to);
                if(to < 0 || to >= MAX_NOF_CLIENTS || client_queue_id[to]<0)
                {
                    snprintf(buffer,MAX_TEXT_MSG_LEN + 100, "user %d does not exist", to);
                    send_msg_to(-1, from,METAMSG , time(NULL),
                                buffer);
                    snprintf(buffer,MAX_TEXT_MSG_LEN + 100, "user %d tried to say [%s] to a non-existent user %d", from, recv_msg.data.payload.msg.text, to);
                    logger(buffer, 1);

                }
                else
                {
                    send_msg_to(from, recv_msg.data.payload.msg.client,MSG , time(NULL),
                                recv_msg.data.payload.msg.text);
                    if(recv_msg.data.payload.msg.client == from)
                        snprintf(buffer,MAX_TEXT_MSG_LEN + 100, "user %d said [%s] to themself", from, recv_msg.data.payload.msg.text);
                    else
                        snprintf(buffer,MAX_TEXT_MSG_LEN + 100, "user %d said [%s] to %d", from, recv_msg.data.payload.msg.text, recv_msg.data.payload.msg.client);
                    logger(buffer,1);
                }
                break;
            }


            case LIST:
                snprintf(buffer, MAX_TEXT_MSG_LEN + 100,"user %d requested userlist", from);
                send_listing_to(from,time(NULL));
                logger(buffer,1);
                break;
        }
    }
    logger("server stopped", 1);
}