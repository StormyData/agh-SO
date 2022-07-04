#pragma once
#include <sys/msg.h>
#define MAX_TEXT_MSG_LEN 512
#define MAX_NOF_CLIENTS 20
#define M_NAME_MAX 256

enum CLIENT_TO_SERVER_MSG_TYPE{
    INIT = 1,
    LIST = 2,
    TOALL = 3,
    TOONE = 4,
    STOP = 5
};
#define HIGHEST_CLIENT_TO_SERVER_MSG_TYPE 6

enum SERVER_TO_CLIENT_MSG_TYPE
{
    INIT_RESPONSE = 1,
    USERLIST = 2,
    METAMSG = 3,
    BROADCAST_MSG = 4,
    MSG = 5,
    QUIT = 6
};
char server_key[M_NAME_MAX] = "/systemy_operacyjne_cw06_zad2_server";

struct text_msg{
    char text[MAX_TEXT_MSG_LEN];
    int client;
};
union client_to_server_msg_payload{
    struct text_msg msg;
    char key[M_NAME_MAX];
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