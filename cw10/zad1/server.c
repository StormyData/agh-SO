#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "vector.h"
#include "common_tcp.h"
#include "game_server.h"

int connection_id_counter = 0;
int master_tcp_network_socket = -1;
int master_tcp_unix_socket = -1;
int epoll_fd = -1;
struct server server;

struct MyVector connections;


int get_connection_index_from_fd(int fd) {
    for(int i =0;i < connections.len; i++)
    {
        struct connection* connection = connections.data[i];
        if(connection->fd == fd)
            return i;
    }
    return -1;
}
int get_connection_index_from_id(int id) {
    for(int i =0;i < connections.len; i++)
    {
        struct connection* connection = connections.data[i];
        if(id == connection->id)
            return i;
    }
    return -1;
}



void accept_and_add_connection(int master_fd)
{
    int accepted_fd = accept(master_fd, NULL, NULL);
    int flags = fcntl(accepted_fd,F_GETFL);
    fcntl(accepted_fd, F_SETFL, flags | O_NONBLOCK);
    struct epoll_event event;
    event.data.fd = accepted_fd;
    event.events = EPOLLIN | epoll_edge;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accepted_fd, &event);
    struct connection* connection = malloc(sizeof(struct connection));
    InitConnection(connection, accepted_fd);
    connection->id = connection_id_counter;
    connection_id_counter++;
    MyVectorAppend(&connections, connection);
    //printf("accepted new connection with fd: %d\n", accepted_fd);
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

void setup_network_master_socket(int port)
{
    master_tcp_network_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in network_address;
    network_address.sin_family = AF_INET;
    network_address.sin_port = htons(port);
    network_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(master_tcp_network_socket, (struct sockaddr *) &network_address, sizeof(network_address)))
    {
        perror("unable to bind network socket\n");
        exit(-1);
    }
    int flags = fcntl(master_tcp_network_socket,F_GETFL);
    fcntl(master_tcp_network_socket, F_SETFL, flags | O_NONBLOCK);

    listen(master_tcp_network_socket, 100);


    struct epoll_event ev;
    ev.events = EPOLLIN| epoll_edge;
    ev.data.fd = master_tcp_network_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_tcp_network_socket, &ev) == -1) {
        perror("cannot add network socket to epoll\n");
        exit(-1);
    }

}
void setup_unix_master_socket(char* socket_path)
{

    master_tcp_unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un unix_address;
    unix_address.sun_family = AF_UNIX;
    strcpy(unix_address.sun_path, socket_path);
    unlink(socket_path);
    if(bind(master_tcp_unix_socket, (struct sockaddr *) &unix_address, sizeof(unix_address)))
    {
        perror("unable to bind unix socket\n");
        exit(-1);
    }
    int flags = fcntl(master_tcp_unix_socket,F_GETFL);
    fcntl(master_tcp_unix_socket, F_SETFL, flags | O_NONBLOCK);
    listen(master_tcp_unix_socket, 100);

    struct epoll_event ev;
    ev.events = EPOLLIN | epoll_edge;
    ev.data.fd = master_tcp_unix_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_tcp_unix_socket, &ev) == -1) {
        perror("cannot add network socket to epoll\n");
        exit(-1);
    }
}
void at_exit()
{
    if(master_tcp_network_socket != -1)
        close(master_tcp_network_socket);
    if(master_tcp_unix_socket != -1)
        close(master_tcp_unix_socket);
    if(epoll_fd != -1)
        close(epoll_fd);
    destroy_server(&server);
}


void remove_connection(int index) {

    struct connection *connection = connections.data[index];
    struct message* msg = malloc(sizeof(struct message));
    msg->type = CTS_DISCONNECTED;
    msg->player_id = connection->id;
    server_recv_message(&server, msg);
    printf("closing connection for fd: %d\n", connection->fd);
    shutdown(connection->fd, SHUT_RDWR);
    close(connection->fd);
    DestroyConnection(connection);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connection->fd, NULL);
    free(connection);
    MyVectorRemoveAndReplaceWithLast(&connections, index);
}

void process_message(void* context, char* message, unsigned int size)
{
    int index = *(int*)context;
    struct connection* connection = connections.data[index];
    struct message* msg = parse_message(message, size);
    msg->player_id = connection->id;
//    printf("from: %d received message: ", msg->player_id);
//    print_message(msg);
//    printf("\n");
    server_recv_message(&server,msg);
}
void send_game_message(struct message* message, void* context)
{
    int index = get_connection_index_from_id(message->player_id);
    if(index < 0)
    {
        free_message(message);
        return;
    }
    char* data;
    uint32_t size;
//    printf("to: %d sending message: ", message->player_id);
//    print_message(message);
//    printf("\n");
    serialize_message(message,&data, &size);
    if(send_message(connections.data[index],data,size,epoll_fd))
    {
        remove_connection(index);
    }
    free_message(message);
}
int main(int argc, char** argv)
{
    init_server(&server,send_game_message,NULL);
    int port = 2223;
    char* socket_path = "./tcp_socket";
    if(argc == 3 || argc == 2)
    {
        port = atoi(argv[1]);
    }
    if(argc == 3)
    {
        socket_path = argv[2];
    }
    atexit(at_exit);
    MyVectorInit(&connections, 10);
    const int MAX_EVENTS = 20;
    setup_epoll();
    setup_network_master_socket(port);
    setup_unix_master_socket(socket_path);

    struct epoll_event events[MAX_EVENTS];

    int nfds;
    while (1)
    {
        nfds = epoll_wait(epoll_fd, events,MAX_EVENTS,-1);
        if(nfds == -1)
        {
            if(errno != EINTR)
            {
                perror("epoll wait error\n");
                exit(-1);
            }
        }
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == master_tcp_network_socket || events[i].data.fd == master_tcp_unix_socket)
            {
                accept_and_add_connection(events[i].data.fd);
            }
            else
            {
                if(events[i].events & EPOLLIN)
                {
                    //printf("received data from %d\n", events[i].data.fd);
                    int index = get_connection_index_from_fd(events[i].data.fd);
                    if(index == -1)
                        continue;
                    if(try_recv_message(connections.data[index], process_message, &index))
                        remove_connection(index);

                }
                if(events[i].events & EPOLLOUT)
                {
                    //printf("sending data to %d\n", events[i].data.fd);
                    int index = get_connection_index_from_fd(events[i].data.fd);
                    if(index == -1)
                        continue;
                    if(try_send_message(connections.data[index],epoll_fd))
                        remove_connection(index);
                }
            }
        }
    }
}



