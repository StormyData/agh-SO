#pragma once
#include <sys/msg.h>
#define MAX_TEXT_MSG_LEN 512
#define MAX_NOF_CLIENTS 20


enum CLIENT_TO_SERVER_MSG_TYPE{
    INIT = 5,
    LIST = 4,
    TOALL = 3,
    TOONE = 2,
    STOP = 1
};
#define HIGHEST_CLIENT_TO_SERVER_MSG_TYPE 6

enum SERVER_TO_CLIENT_MSG_TYPE
{
    INIT_RESPONSE = 6,
    USERLIST = 5,
    METAMSG = 4,
    BROADCAST_MSG = 3,
    MSG = 2,
    QUIT = 1
};
key_t server_key;

struct text_msg{
    char text[MAX_TEXT_MSG_LEN];
    int client;
};
union client_to_server_msg_payload{
    struct text_msg msg;
    key_t key;
};

struct client_to_Server_msg_data{
    int from;
    union client_to_server_msg_payload payload;
};

struct client_to_server_msg{
    long mtype;
    struct client_to_Server_msg_data data;
};

struct client_list_payload
{
    int list[MAX_NOF_CLIENTS];
    int n;
};
union client_to_Server_msg_payload
{
    int identifier;
    struct client_list_payload clients;
    struct text_msg msg;
};


struct server_to_client_msg_data{
    time_t when;
    union client_to_Server_msg_payload payload;
};

struct server_to_client_msg{
    long mtype;
    struct server_to_client_msg_data data;
};