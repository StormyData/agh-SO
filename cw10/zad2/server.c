#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "vector.h"
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include "common_udp.h"
#include "game_server.h"

int connection_id_counter = 0;
int master_udp_network_socket = -1;
int master_udp_unix_socket = -1;
int epoll_fd = -1;
struct server server;
struct MyVector connections;
pthread_mutex_t connections_mutex;


int get_connection_index_from_id(int id);

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
    master_udp_network_socket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in network_address;
    network_address.sin_family = AF_INET;
    network_address.sin_port = htons(port);
    network_address.sin_addr.s_addr = htonl(INADDR_ANY);
    int flags = fcntl(master_udp_network_socket,F_GETFL);
    fcntl(master_udp_network_socket, F_SETFL, flags | O_NONBLOCK);

    if(bind(master_udp_network_socket, (struct sockaddr *) &network_address, sizeof(network_address)))
    {
        perror("unable to bind network socket\n");
        exit(-1);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = master_udp_network_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_udp_network_socket, &ev) == -1) {
        perror("cannot add network socket to epoll\n");
        exit(-1);
    }

}
void setup_unix_master_socket(char* socket_path)
{

    master_udp_unix_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un unix_address;
    unix_address.sun_family = AF_UNIX;
    strcpy(unix_address.sun_path, socket_path);
    int flags = fcntl(master_udp_unix_socket,F_GETFL);
    fcntl(master_udp_unix_socket, F_SETFL, flags | O_NONBLOCK);

    unlink(socket_path);
    if(bind(master_udp_unix_socket, (struct sockaddr *) &unix_address, sizeof(unix_address)))
    {
        perror("unable to bind unix socket\n");
        exit(-1);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = master_udp_unix_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_udp_unix_socket, &ev) == -1) {
        perror("cannot add network socket to epoll\n");
        exit(-1);
    }
}
void at_exit()
{
    if(master_udp_network_socket != -1)
        close(master_udp_network_socket);
    if(master_udp_unix_socket != -1)
        close(master_udp_unix_socket);
    if(epoll_fd != -1)
        close(epoll_fd);
    //destroy_server(&server);
}


void remove_connection(int index) {
    pthread_mutex_lock(&connections_mutex);
    struct connection *connection = connections.data[index];
    struct message* msg = malloc(sizeof(struct message));
    msg->type = CTS_DISCONNECTED;
    msg->player_id = connection->id;
    server_recv_message(&server, msg);
    free(connection);
    MyVectorRemoveAndReplaceWithLast(&connections, index);
    pthread_mutex_unlock(&connections_mutex);
}

void send_game_message(struct message* message, void* context)
{
    pthread_mutex_lock(&connections_mutex);
    int index = get_connection_index_from_id(message->player_id);
    if(index < 0)
    {
        free_message(message);
        return;
    }
    char* data;
    uint32_t size;
    serialize_message(message,&data, &size);
    char buffer[4096];
    if(size > 4095)
    {
        printf("err: message too long\n");
        exit(-1);
    }
    memcpy(buffer + 1, data, size);
    buffer[0] = 0;
    struct connection* conn = connections.data[index];
    sendto(conn->fd, buffer, size + 1, 0, (const struct sockaddr *) &conn->addr.addr, conn->addr.addr_len);
    free_message(message);
    pthread_mutex_unlock(&connections_mutex);
}

int get_connection_index_from_id(int id) {
    for(int i =0;i < connections.len;i++)
    {
        struct connection* conn = connections.data[i];
        if(conn->id == id)
            return i;
    }
    return -1;
}
void* ping_thread(void* _)
{
    char data[1];
    data[0] = 1;
    struct timespec timespec;
    while(1)
    {
        clock_gettime(CLOCK_MONOTONIC,&timespec);
        pthread_mutex_lock(&connections_mutex);
        // printf("ping thread working\n");
        for(int i=0;i<connections.len;i++)
        {
            struct connection* conn = connections.data[i];
            sendto(conn->fd, data, 1, 0, (const struct sockaddr *) &conn->addr.addr, conn->addr.addr_len);
            if(timespec.tv_sec - conn->last_active_on.tv_sec > 60)
            {
                remove_connection(i);
                i--;
            }
        }
        pthread_mutex_unlock(&connections_mutex);
        sleep(10);
    }
    return NULL;
}
void try_read_from_socket(int fd)
{

    char buffer[4096];
    struct my_socket_addr addr;
    while(1)
    {
        addr.addr_len = sizeof(struct sockaddr_storage);
        pthread_mutex_lock(&connections_mutex);
        ssize_t ret = recvfrom(fd, buffer, 4096, 0, (struct sockaddr *) &addr.addr, &addr.addr_len);
        if(ret == -1)
        {
            if(errno == EWOULDBLOCK || errno == EAGAIN) {
                pthread_mutex_unlock(&connections_mutex);
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
        struct connection* current_conn = NULL;
        for(int i = 0; i< connections.len;i++)
        {
            struct connection* conn = connections.data[i];
            if(conn->fd != fd)
                continue;
            if(conn->addr.addr_len != addr.addr_len)
                continue;
            if(memcmp(&addr.addr,&conn->addr.addr, addr.addr_len) != 0) {
                continue;
            }
            current_conn = conn;
            break;
        }
        if(current_conn == NULL) // new connection
        {
            struct connection* new_conn = malloc(sizeof(struct connection));
            memcpy(&new_conn->addr, &addr, sizeof(struct my_socket_addr));

            new_conn->fd = fd;
            new_conn->id = connection_id_counter;
            connection_id_counter++;
            MyVectorAppend(&connections,new_conn);
            current_conn = new_conn;
        }
        clock_gettime(CLOCK_MONOTONIC,&current_conn->last_active_on);

        if(p_type == 0) //normal
        {
                struct message* msg = parse_message(buffer + 1, ret - 1);
                msg->player_id = current_conn->id;
//            printf("received message\n");
                if(msg->type == CTS_DISCONNECTED)
                {
                    printf("client disconnecting\n");
                    for(int i=0;i<connections.len;i++)
                    {
                        if(connections.data[i] == current_conn)
                        {
                            remove_connection(i);
                            break;
                        }
                    }
                }
                else
                    server_recv_message(&server, msg);
        }
        else if(p_type == 1) //keep-alive
        {
            //printf("received ping from: ");
            //sock_addr_printer((struct sockaddr *) &addr);
            //pass
        }
        pthread_mutex_unlock(&connections_mutex);
    }
}
void int_handler(int _)
{
    destroy_server(&server);
    exit(0);
}

int main(int argc, char** argv)
{

    srand(time(NULL));
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&connections_mutex,&mutexattr);
    pthread_mutexattr_destroy(&mutexattr);

    pthread_t ping_thread_id;
    init_server(&server,send_game_message,NULL);
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = int_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT,&sa,NULL);
    int port = 2223;
    char* socket_path = "./udp_socket";
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
    pthread_create(&ping_thread_id,NULL,ping_thread,NULL);
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
            if(events[i].events & EPOLLIN)
            {
                try_read_from_socket(events[i].data.fd);
            }
        }
    }
}



