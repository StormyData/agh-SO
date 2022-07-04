#pragma once

enum GAME_STATE {XES_TURN, OS_TURN, XES_WIN, OS_WIN, DRAW};
enum MESSAGE_TYPE {CTS_INIT, STC_NAME_REJECTED, STC_WAITING, STC_PAIRED, STC_BOARD_UPDATE,
    CTS_PERFORM_MOVE, STC_MOVE_REJECTED, CTS_DISCONNECTED, STC_DISCONNECTED, STC_SHUTDOWN, STC_GAME_ENDED};

struct message{
    int player_id;
    enum MESSAGE_TYPE type;
    void* payload;
};
struct stc_paired_message_payload
{
    char* other_players_name;
    int side;
};

struct board{
    char colors[3][3];
};

char check_solution(struct board *board)
        {
    for(int i = 0 ;i < 3 ; i++)
    {
        if(board->colors[i][0] == 0)
            continue;
        if(board->colors[i][0] == board->colors[i][1] && board->colors[i][2] == board->colors[i][1])
            return board->colors[i][0];
    }
    for(int i = 0 ;i < 3 ; i++)
    {
        if(board->colors[0][i] == 0)
            continue;
        if(board->colors[1][i] == board->colors[0][i] && board->colors[2][i] == board->colors[0][i])
            return board->colors[0][i];
    }
    if(board->colors[0][0] != 0)
    {
        if(board->colors[0][0] == board->colors[1][1] && board->colors[0][0] == board->colors[2][2])
            return board->colors[0][0];
    }
    if(board->colors[2][0] != 0)
    {
        if(board->colors[2][0] == board->colors[1][1] && board->colors[2][0] == board->colors[0][2])
            return board->colors[2][0];
    }
    for(int i = 0 ; i < 3 ; i++)
        for(int j = 0; j< 3; j++)
            if(board->colors[i][j] == 0)
                return 0;
    return -2;
}

void free_message(struct message* message)
{
    switch (message->type) {
        case CTS_INIT:
        case STC_BOARD_UPDATE:
        case STC_MOVE_REJECTED:
        case CTS_PERFORM_MOVE:
        case STC_GAME_ENDED:
            free(message->payload);
            break;
        case CTS_DISCONNECTED:
        case STC_WAITING:
        case STC_NAME_REJECTED:
        case STC_DISCONNECTED:
        case STC_SHUTDOWN:
            break;
        case STC_PAIRED:
        {
            struct stc_paired_message_payload* payload = message->payload;
            free(payload->other_players_name);
            free(payload);
            break;
        }
    }
}

struct message* parse_message(char* data, uint32_t size)
{
    struct message* message = malloc(sizeof(struct message));
    switch (*data) {
        case 1:
            message->type = CTS_INIT;
            message->payload = malloc(size - 1);
            memcpy(message->payload, data + 1, size - 1);
            return message;
        case 2:
            message->type = STC_NAME_REJECTED;
            return message;
        case 3:
            message->type = STC_WAITING;
            return message;
        case 4:
            message->type = STC_PAIRED;
            message->payload = malloc(sizeof(struct stc_paired_message_payload));
            ((struct stc_paired_message_payload*)message->payload)->side = (int)*(data + 1);
            ((struct stc_paired_message_payload*)message->payload)->other_players_name = malloc(size - 2);
            memcpy(((struct stc_paired_message_payload*)message->payload)->other_players_name,data + 2, size - 2);
            return message;
        case 5:
            message->type = STC_BOARD_UPDATE;
            message->payload = malloc(sizeof(struct board));
            memcpy(message->payload, data + 1, sizeof(struct board));
            return message;
        case 6:
            message->type = CTS_PERFORM_MOVE;
            message->payload = malloc(sizeof(int));
            *(int*)message->payload = (int)*(data + 1);
            return message;
        case 7:
            message->type = STC_MOVE_REJECTED;
            message->payload = malloc(size - 1);
            memcpy(message->payload, data + 1, size - 1);
            return message;
        case 8:
            message->type = CTS_DISCONNECTED;
            return message;
        case 9:
            message->type = STC_DISCONNECTED;
            return message;
        case 10:
            message->type = STC_SHUTDOWN;
            return message;
        case 11:
            message->type = STC_GAME_ENDED;
            message->payload = malloc(sizeof(int));
            *(int*)message->payload = (int)*(data + 1);
            return message;
        default:
            fprintf(stderr, "unknown message type\n");
            free(message);
            return NULL;
    }
}

void serialize_message(struct message* message, char** data, uint32_t* size)
{

    switch (message->type) {
        case CTS_INIT:
            *size = 1 + strlen(message->payload) + 1;
            *data = malloc(*size);
            **data = 1;
            strcpy((*data + 1), message->payload);
            break;
        case STC_NAME_REJECTED:
            *size = 1;
            *data = malloc(*size);
            **data = 2;
            break;
        case STC_WAITING:
            *size = 1;
            *data = malloc(*size);
            **data = 3;
            break;
        case STC_PAIRED:
            *size = 2 + strlen(((struct stc_paired_message_payload*)message->payload)->other_players_name) + 1;
            *data = malloc(*size);
            **data = 4;
            *(*data + 1) = (char)(((struct stc_paired_message_payload*)message->payload)->side);
            strcpy((*data + 2), ((struct stc_paired_message_payload*)message->payload)->other_players_name);
            break;
        case STC_BOARD_UPDATE:
            *size = 1 + sizeof(struct board);
            *data = malloc(*size);
            **data = 5;
            memcpy((*data + 1), message->payload, sizeof(struct board));
            break;
        case CTS_PERFORM_MOVE:
            *size = 2;
            *data = malloc(*size);
            **data = 6;
            *(*data + 1) = (char)(*((int*)message->payload));
            break;
        case STC_MOVE_REJECTED:
            *size = 1 + strlen(message->payload) + 1;
            *data = malloc(*size);
            **data = 7;
            strcpy((*data + 1), message->payload);
            break;
        case CTS_DISCONNECTED:
            *size = 1;
            *data = malloc(*size);
            **data = 8;
            break;
        case STC_DISCONNECTED:
            *size = 1;
            *data = malloc(*size);
            **data = 9;
            break;
        case STC_SHUTDOWN:
            *size = 1;
            *data = malloc(*size);
            **data = 10;
            break;
        case STC_GAME_ENDED:
            *size = 2;
            *data = malloc(*size);
            **data = 11;
            *(*data + 1) = (char)(*((int*)message->payload));
            break;
    }
}

void print_message(struct message* message)
{

    switch (message->type) {
        case CTS_INIT:
            printf("CTS_INIT (%s)\n", (char*)message->payload);
            break;
        case STC_NAME_REJECTED:
            printf("STC_NAME_REJECTED\n");
            break;
        case STC_WAITING:
            printf("STC_WAITING\n");
            break;
        case STC_PAIRED:
            printf("STC_PAIRED (%s, %d)\n", ((struct stc_paired_message_payload*)message->payload)->other_players_name,
                   ((struct stc_paired_message_payload*)message->payload)->side);
            break;
        case STC_BOARD_UPDATE:
            printf("STC_BOARD_UPDATE (?)\n");
            break;
        case CTS_PERFORM_MOVE:
            printf("CTS_PERFORM_MOVE (%d)\n",*((int*)message->payload));
            break;
        case STC_MOVE_REJECTED:
            printf("STC_MOVE_REJECTED (%s)\n",(char*)message->payload);
            break;
        case CTS_DISCONNECTED:
            printf("CTS_DISCONNECTED\n");
            break;
        case STC_DISCONNECTED:
            printf("STC_DISCONNECTED\n");
            break;
        case STC_SHUTDOWN:
            printf("STC_SHUTDOWN\n");
            break;
        case STC_GAME_ENDED:
            printf("STC_GAME_ENDED (%d)\n", *(int*)message->payload);
            break;
    }
}