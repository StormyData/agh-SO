#pragma once
#include <string.h>
#include <stdlib.h>
#include <errno.h>
int epoll_edge = EPOLLET;

void recv_peeker(int fd)
{
    printf("peeker : ");
    char buffer[4096];
    int retval = recv(fd,buffer,4096,MSG_PEEK);
    if(retval == 0)
    {
        printf("closed\n");
        return;
    }
    if(retval == -1)
    {
        printf("error %d, %s\n", errno, strerror(errno));
        return;
    }
    for(int i=0;i< retval;i++)
    {
        printf("%02x ",*(buffer + i));
    }
    printf("\n");
}


struct send_q_element{
    char* data;
    unsigned int size;
    unsigned int sent_already;
    struct send_q_element* next;
};


struct connection{
    int fd;
    int id;
    char* buffer;
    unsigned int expected_length;
    unsigned int read_already;
    struct send_q_element* head;
    struct send_q_element* tail;
};


void InitConnection(struct connection* connection, int fd)
{
    connection->fd = fd;
    connection->expected_length = sizeof(uint32_t);
    connection->read_already = 0;
    connection->buffer = malloc(sizeof(uint32_t));
    connection->head = NULL;
    connection->tail = NULL;
}
void DestroyConnection(struct connection* connection)
{
    if(connection->buffer != NULL)
        free(connection->buffer);
    struct send_q_element* head = connection->head;
    while(head != NULL)
    {
        free(head->data);
        struct send_q_element* tmp = head;
        head = head->next;
        free(tmp);
    }
}


int try_recv_message(struct connection* connection, void(*on_completion)(void* context, char* message, uint32_t size), void* context) {
    if(connection->expected_length < sizeof(uint32_t))
    {
        fprintf(stderr,"error expected length < %lu\n", sizeof(uint32_t));
        exit(-1);
    }
    //int read_this_cycle = 0;
    while(1)
    {
        //recv_peeker(connection->fd);
        ssize_t retval = recv(connection->fd, connection->buffer + connection->read_already, connection->expected_length - connection->read_already, 0);
        if(retval == 0)
        {
            //printf("%d\n",read_this_cycle);
            return 1;
        }
        if(retval == -1)
        {
            //printf("errno: %d\n" ,errno);
            //printf("%s\n", strerror(errno));
            if(errno == EWOULDBLOCK || errno == EAGAIN) {
              //  printf("%d\n",read_this_cycle);
                return 0;
            }
            perror("recv error\n");
        }
        else
        {
            //printf("received data from %d: ", connection->fd);
            //for(int i=0;i< retval;i++)
            //    printf("%02x ",*(connection->buffer + connection->read_already + i));
            //printf("\n");
            connection->read_already += retval;
            //read_this_cycle += retval;
        }
        if(connection->read_already == connection->expected_length)
        {
            if(connection->expected_length > 4)
            {
                on_completion(context,connection->buffer + sizeof(uint32_t), connection->expected_length - sizeof(uint32_t));
                connection->buffer = realloc(connection->buffer, sizeof(uint32_t));
                connection->expected_length = sizeof(uint32_t);
                connection->read_already = 0;
            }
            else
            {
                connection->expected_length = sizeof(uint32_t) + ntohl(*(uint32_t *)connection->buffer);
                connection->buffer = realloc(connection->buffer,connection->expected_length);
                connection->read_already = sizeof(uint32_t);
            }
        }
    }
}


int try_send_message(struct connection* connection, int epoll_fd)
{
    while(1)
    {
        if(connection->head == NULL)
        {
            struct epoll_event event;
            event.data.fd = connection->fd;
            event.events = EPOLLIN | epoll_edge;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, connection->fd, &event);
            return 0;
        }
        struct send_q_element* head = connection->head;

        if(head->sent_already < head->size)
        {
            ssize_t retval = send(connection->fd, head->data + head->sent_already, head->size - head->sent_already, 0);
            if(retval == -1)
            {
                if(errno == EWOULDBLOCK || errno == EAGAIN) {
                    return 0;
                }
                if(errno == ECONNRESET) {
                    return 1;
                }
                perror("send error\n");
            }
            else {
//                printf("sent data to %d: ", connection->fd);
//                for(int i=0;i< retval;i++)
//                    printf("%02x ",*(head->data + head->sent_already + i));
//                printf("\n");
                head->sent_already += retval;
            }
        }
        if(head->sent_already == head->size)
        {
            free(head->data);
            connection->head = head->next;
            free(head);
            if(connection->head == NULL)
                connection->tail = NULL;
        }
    }
}

int send_message(struct connection* connection, char* message, unsigned int size, int epoll_fd)
{
    struct send_q_element* curr = malloc(sizeof(struct send_q_element));
    char* cpy = malloc(size + sizeof(uint32_t));
    uint32_t* cpy_size = (uint32_t *)cpy;
    *cpy_size = htonl(size);
    memcpy(cpy + sizeof(uint32_t), message, size);

    curr->sent_already = 0;
    curr->size = size + sizeof(uint32_t);
    curr->data = cpy;
    curr->next = NULL;
    if(connection->tail == NULL)
    {
        connection->head = curr;
        connection->tail = curr;
    }
    else
        connection->tail->next = curr;
    struct epoll_event event;
    event.data.fd = connection->fd;
    event.events = EPOLLIN | EPOLLOUT | epoll_edge;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, connection->fd, &event);
    return try_send_message(connection, epoll_fd);
}

