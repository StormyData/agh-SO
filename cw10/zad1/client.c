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
#include "vector.h"
#include "common_tcp.h"
#include "game_common.h"

enum CLIENT_STATE {GAME, WAITING} clientState = WAITING;
struct connection* connection;
int side = 0;

int epoll_fd = -1;
void at_exit()
{
    if(connection != NULL)
    {
        close(connection->fd);
        DestroyConnection(connection);
        free(connection);
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
    int socket_fd = socket(AF_INET, SOCK_STREAM,0);
    if(connect(socket_fd, (struct sockaddr *)&addr, sizeof (addr)))
    {
        perror("cannot connect");
        exit(-1);
    }
    int flags = fcntl(socket_fd, F_GETFL);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    struct epoll_event event;
    event.data.fd = socket_fd;
    event.events = EPOLLIN | epoll_edge;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    connection = malloc(sizeof(struct connection));
    InitConnection(connection, socket_fd);
}
void connect_unix(char* path)
{
    struct sockaddr_un unix_address;
    unix_address.sun_family = AF_UNIX;
    strcpy(unix_address.sun_path, path);
    int socket_fd = socket(AF_UNIX, SOCK_STREAM,0);
    if(connect(socket_fd, (struct sockaddr *)&unix_address, sizeof (unix_address)))
    {
        perror("cannot connect");
        exit(-1);
    }
    int flags = fcntl(socket_fd, F_GETFL);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    struct epoll_event event;
    event.data.fd = socket_fd;
    event.events = EPOLLIN | epoll_edge;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    connection = malloc(sizeof(struct connection));
    InitConnection(connection, socket_fd);
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
void process_message(void* context, char* message, uint32_t size)
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
            printf("other player disconnected\n");
            clientState = WAITING;
            break;
        case STC_SHUTDOWN:
            printf("server is shutting down, we should too\n");
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
void send_message_to_server(struct message* message)
{
    char* data;
    uint32_t size;
//    printf("sending message: ");
//    print_message(message);
//    printf("\n");
    serialize_message(message, &data, &size);

    if(send_message(connection, data, size, epoll_fd))
    {
        printf("connection closed by server\n");
        exit(0);
    }
}
int main(int argc, char** argv) {
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
    struct epoll_event event;
    event.data.fd = fileno(stdin);
    event.events = EPOLLIN | epoll_edge;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fileno(stdin),&event);
    const int MAX_EVENTS = 20;
    struct epoll_event events[MAX_EVENTS];
    struct message message;
    message.type = CTS_INIT;
    message.payload = malloc(strlen(argv[1]));
    strcpy(message.payload, argv[1]);
    send_message_to_server(&message);
    free(message.payload);
    message.payload = malloc(sizeof(int));
    message.type = CTS_PERFORM_MOVE;
    int nfds;
    while(1)
    {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if(nfds == -1)
        {
            perror("epoll wait error\n");
            exit(-1);
        }
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == connection->fd)
            {
                if(events[i].events & EPOLLIN)
                {
                    //printf("receiving message\n");
                    if(try_recv_message(connection, process_message, NULL))
                    {
                        printf("connection closed by server\n");
                        exit(0);
                    }
                }
                if(events[i].events & EPOLLOUT)
                {
                    printf("trying to send message\n");
                    if(try_send_message(connection, epoll_fd))
                    {
                        printf("connection closed by server\n");
                        exit(0);
                    }
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
