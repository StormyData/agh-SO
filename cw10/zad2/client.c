#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "vector.h"
#include "common_udp.h"
#include "game_common.h"

enum CLIENT_STATE {GAME, WAITING} clientState = WAITING;
int socket_fd = -1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct timespec server_last_active_on;

int side = 0;

int epoll_fd = -1;

void try_recv_message(int fd);

void send_message_to_server(struct message* message)
{
    char* data;
    uint32_t size;
//    printf("sending message: ");
//    print_message(message);
//    printf("\n");
    serialize_message(message, &data, &size);
    char buffer[4096];
    if(size > 4095)
    {
        printf("err: message too long\n");
        exit(-1);
    }
    memcpy(buffer + 1, data, size);
    buffer[0] = 0;
    free(data);
    pthread_mutex_lock(&mutex);
    if(socket_fd != -1)
        send(socket_fd,buffer ,size + 1, 0);
    pthread_mutex_unlock(&mutex);
}
void at_exit()
{
    if(socket_fd != -1)
    {
        close(socket_fd);
    }
    if(epoll_fd != -1)
        close(epoll_fd);
}
void setup_epoll()
{
    epoll_fd = epoll_create1(0);
    if(epoll_fd < 0)
    {
        perror("cannot create epollfd\n");
        exit(-1);
    }
}
void connect_network(char* address, u_short port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address);
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(connect(socket_fd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)))
    {
        perror("connect error\n");
        exit(-1);
    }
    int flags = fcntl(socket_fd, F_GETFL);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    struct epoll_event event;
    event.data.fd = socket_fd;
    event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
}


void connect_unix(char* path)
{
    struct sockaddr_un unix_address;
    unix_address.sun_family = AF_UNIX;
    strcpy(unix_address.sun_path, path);
    socket_fd = socket(AF_UNIX, SOCK_DGRAM,0);
    struct sockaddr_un bind_addr;
    bind_addr.sun_family = AF_UNIX;
    memset(bind_addr.sun_path,0,108);
    snprintf(bind_addr.sun_path,107,"/tmp/%d",rand());
    unlink(bind_addr.sun_path);
    if(bind(socket_fd, (const struct sockaddr *) &bind_addr, sizeof (bind_addr)))
    {

        perror("bind error\n");
        exit(-1);
    }
    if(connect(socket_fd, (const struct sockaddr *) &unix_address, sizeof(struct sockaddr_un)))
    {
        perror("connect error\n");
        exit(-1);
    }

    int flags = fcntl(socket_fd, F_GETFL);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    struct epoll_event event;
    event.data.fd = socket_fd;
    event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
}
void print_board(struct board* board)
{
    for(int i=0;i<3;i++)
    {
        for(int j = 0; j < 3; j++)
        {
            if(board->colors[i][j] == 1)
                printf("X");
            else if(board->colors[i][j] == -1)
                printf("O");
            else
                printf(" ");
            if(j != 2)
                printf("|");
        }
        printf("\t");
        for(int j = 0; j < 3; j++)
        {
            if(board->colors[i][j] == 0)
                printf("%d", i * 3 + j);
            else
                printf(" ");
            if(j != 2)
                printf("|");
        }

        if(i != 2)
            printf("\n- - -\t- - -");
        printf("\n");
    }
}
void process_message(char* message, uint32_t size)
{
    struct message* msg = parse_message(message, size);
//    printf("received message: ");
//    print_message(msg);
//    printf("\n");
    switch (msg->type) {
        case CTS_INIT:
        case CTS_PERFORM_MOVE:
        case CTS_DISCONNECTED:
            break;
        case STC_BOARD_UPDATE:
            print_board(msg->payload);
            break;
        case STC_NAME_REJECTED:
            printf("name rejected by the server\n");
            exit(-2);
        case STC_PAIRED:
            printf("paired with %s\n", ((struct stc_paired_message_payload*)msg->payload)->other_players_name);
            side = ((struct stc_paired_message_payload*)msg->payload)->side;
            printf("your symbol is %c\n", side > 0 ? 'X': 'O');
            clientState = GAME;
            break;
        case STC_MOVE_REJECTED:
            printf("server says you cannot do that move now: %s\n", (char*)msg->payload);
            break;
        case STC_DISCONNECTED:
            printf("other player disconnected, waiting for new opponent\n");
            clientState = WAITING;
            break;
        case STC_SHUTDOWN:
            printf("server is shutting down, exiting\n");
            exit(0);
        case STC_WAITING:
            printf("waiting for other player to join\n");
            clientState = WAITING;
            break;
        case STC_GAME_ENDED:
        {
            int who_won = *(int*)msg->payload;
            if(who_won == 0)
                printf("Game ended, Its a draw\n");
            else if(who_won == side)
                printf("Game ended, you won\n");
            else
                printf("Game ended, you lost\n");
            break;
        }

    }
    free_message(msg);
}
void* ping_thread(void* _)
{
    char data[1];
    data[0] = 1;
    struct timespec timespec;
    while(1)
    {
        pthread_mutex_lock(&mutex);
        send(socket_fd,data,1,0);
        clock_gettime(CLOCK_MONOTONIC,&timespec);
        if(timespec.tv_sec - server_last_active_on.tv_sec > 60)
        {
            close(socket_fd);
            socket_fd = -1;
            printf("server timed_out\n");
            exit(-2);
            return NULL;
        }
        pthread_mutex_unlock(&mutex);
        sleep(10);
    }
}
void main_loop()
{

    struct message message;
    int payload;
    message.payload = &payload;
    message.type = CTS_PERFORM_MOVE;
    const int MAX_EVENTS = 20;
    struct epoll_event events[MAX_EVENTS];
    int nfds;
    while(1)
    {
        if(socket_fd == -1)
            return;
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if(nfds == -1)
        {
            perror("epoll wait error\n");
            exit(-1);
        }
        if(socket_fd == -1)
            return;
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == socket_fd)
            {
                if(events[i].events & EPOLLIN)
                {
//                    printf("receiving message\n");
                    try_recv_message(events[i].data.fd);
                }
            }
            else if(events[i].data.fd == fileno(stdin))
            {
                scanf("%d", (int*)message.payload);
                send_message_to_server(&message);
            }
        }
    }
}
void int_handler(int _)
{
    struct message message;
    message.type = CTS_DISCONNECTED;
    send_message_to_server(&message);
    exit(0);
}
int main(int argc, char** argv) {

    srand(time(NULL));
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = int_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT,&sa,NULL);
    clock_gettime(CLOCK_MONOTONIC,&server_last_active_on);
    atexit(at_exit);
    setup_epoll();
    if(argc < 4) {
        printf("this program requires 3 or 4 arguments\n");
        return -1;
    }
    if(!strcmp(argv[2], "NETWORK"))
    {
        if(argc != 5)
        {
            printf("this program requires 4 arguments for NETWORK mode: name NETWORK ip_addr port\n");
            return -1;
        }
        u_short port = atoi(argv[4]);
        connect_network(argv[3], port);
    }
    else if(!strcmp(argv[2], "UNIX"))
    {
        if(argc != 4)
        {
            printf("this program requires 4 arguments for UNIX mode: name UNIX path_to_socket\n");
            return -1;
        }
        connect_unix(argv[3]);
    }
    else
    {
        printf("invalid connection mode\n");
        return -1;
    }
    //printf("our address: ");
    //print_sock_name(socket_fd);

    struct epoll_event event;
    event.data.fd = fileno(stdin);
    event.events = EPOLLIN;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fileno(stdin),&event);

    struct message message;
    message.type = CTS_INIT;
    message.payload = malloc(strlen(argv[1]));
    strcpy(message.payload, argv[1]);
    send_message_to_server(&message);
    free(message.payload);
    pthread_t ping_thread_id;
    pthread_create(&ping_thread_id,NULL,ping_thread,NULL);
    main_loop();
    pthread_join(ping_thread_id,NULL);

}

void try_recv_message(int fd) {
    char buffer[4096];
    while(1)
    {
        ssize_t ret = recv(fd, buffer, 4096,0);
        if(ret == -1)
        {
            if(errno == EWOULDBLOCK || errno == EAGAIN) {
                return;
            }
            perror("recvfrom error");
        }
        if(ret < 1)
        {
            printf("invalid packet");
            continue;
        }
        char p_type = *(buffer);

        clock_gettime(CLOCK_MONOTONIC,&server_last_active_on);
        if(p_type == 0) //normal
        {
            process_message(buffer + 1, ret - 1);
        }
        else if(p_type == 1) //keep-alive
        {
            //printf("recevied ping from server\n");
            //pass
        }

    }
}
