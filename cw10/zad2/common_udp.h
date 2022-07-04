#include <netinet/in.h>
struct my_socket_addr
{
    struct sockaddr_storage addr;
    socklen_t addr_len;
};
struct connection
{
    struct timespec last_active_on;
    int id;
    struct my_socket_addr addr;
    int fd;
};
void sock_addr_printer(struct sockaddr* addr)
{
    if(addr->sa_family == AF_INET)
    {
        struct sockaddr_in* in_addr = (struct sockaddr_in*)addr;
        printf("%d.",(in_addr->sin_addr.s_addr>>0)&255);
        printf("%d.",(in_addr->sin_addr.s_addr>>8)&255);
        printf("%d.",(in_addr->sin_addr.s_addr>>16)&255);
        printf("%d ",(in_addr->sin_addr.s_addr>>24)&255);
        printf("%d\n", in_addr->sin_port);
    }
    else if(addr->sa_family == AF_UNIX)
    {
        struct sockaddr_un* un_addr = (struct sockaddr_un*)addr;
        for(int i=0;i<108;i++)
        {
            if(un_addr->sun_path[i] == '\0')
                break;
            printf("%c",un_addr->sun_path[i]);
        }
        printf("\n");
    }
}
void print_sock_name(int fd)
{
    struct sockaddr addr;
    socklen_t len = sizeof(addr);
    getsockname(fd,&addr, &len);
    sock_addr_printer(&addr);
}

void ping(int fd)
{
    struct sockaddr addr;
    socklen_t addr_size = sizeof(addr);
    char buffer[4096];
    ssize_t retval = recvfrom(fd,buffer,4096,MSG_DONTWAIT,&addr, &addr_size);
    if(retval == -1)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        perror("recv error\n");
    }
    printf("from: ");

    for(int j=0;j<retval;j++)
    {
        printf("%02x ", buffer[j]);
    }
    printf("\n");
    sendto(fd,buffer,retval,0,&addr,addr_size);
}