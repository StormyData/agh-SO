#pragma once
#include <string.h>
#include <stdio.h>
#include "vector.h"
#include "game_common.h"

char* c_str_copy(char * str)
{
    if(str == NULL)
        return NULL;
    char* tmp = malloc(strlen(str) + 1);
    strcpy(tmp, str);
    return tmp;
}

//void* c_string_key_copier(void* key)
//{
//    void* tmp = malloc(strlen(key) + 1);
//    strcpy(tmp, key);
//    return tmp;
//}
//int c_string_key_comparator(void* key1, void* key2)
//{
//    return !strcmp(key1, key2);
//}
//int int_key_comparator(void* key1, void* key2)
//{
//    return *(int*)key1 == *(int*)key2;
//}
//void* int_key_copier(void* key)
//{
//    void* tmp = malloc(sizeof(int));
//    *(int*)tmp = *(int*)key;
//    return tmp;
//}

struct player{
    int player_id;
    char* name;
    int room_no;
    int side;
};

struct room{
    int room_id;
    enum GAME_STATE state;
    int x_player_id;
    int o_player_id;
    struct board board;
};

struct server{
    struct MyVector players;
    struct MyVector rooms;
    struct player* waiting_player;
    int last_room_id;
    void (*send_message)(struct message* message, void* context); // should take ownership of the message
    void* send_context;
};

int do_move(struct player *player, struct room *room, int move);
char check_solution(struct board *board);

void init_player(struct player* player, char* name, int player_id)
{
    player->player_id = player_id;
    player->room_no = -1;
    player->name = c_str_copy(name);
    player->side = 0;
}
void init_room(struct room* room, struct player* player_1, struct player* player_2);

void destroy_player(void* player)
{
    free(((struct player*)player)->name);
    free(player);
}
void init_server(struct server* server, void (*send_message)(struct message*, void*), void* send_context)
{
    MyVectorInit(&server->rooms, 10);
    //MyLinkedDictInit(&server->rooms, int_key_comparator, int_key_copier, NULL, free);
    MyVectorInit(&server->players, 10);
    server->waiting_player = NULL;
    server->last_room_id = 0;
    server->send_message = send_message;
    server->send_context = send_context;
}
void server_get_player_and_room(struct server* server, int player_id, struct player** player, struct room** room);

void server_send_board_update_for_room(struct server* server, struct room* room);
void server_send_paired_message_for_room(struct server* server, struct room* room);
void server_send_rejected_move_message_to_player(struct server* server, int player_id, char* reason);
void server_send_other_side_disconnected_message(struct server* server, int player_id);
void server_pair_or_wait_player(struct server* server, struct player* player);

struct player* server_get_player(struct server* server, int player_id)
{
    for(int i= 0;i< server->players.len;i++)
    {
        struct player* p = server->players.data[i];
        if(p->player_id == player_id)
        {
            return p;
        }
    }
    return NULL;
}

void server_send_board_state_to_player(struct server* server, int player_id)
{
    struct player* player;
    struct room* room;
    server_get_player_and_room(server,player_id,&player,&room);
    if(room == NULL)
        return;
    struct message* board_state_message = malloc(sizeof(struct message));
    board_state_message->type = STC_BOARD_UPDATE;
    board_state_message->player_id = player_id;
    board_state_message->payload = malloc(sizeof(struct board));
    memcpy(board_state_message->payload, &room->board, sizeof(struct board));
    server->send_message(board_state_message, server->send_context);
}
void destroy_server(struct server* server)
{
    for(int i=0;i<server->players.len;i++)
    {
        struct message* message = malloc(sizeof(struct message));
        message->player_id = ((struct player*)server->players.data[i])->player_id;
        message->payload = NULL;
        message->type = STC_SHUTDOWN;
        server->send_message(message, server->send_context);
        destroy_player((struct player*)server->players.data[i]);

    }
    for(int i = 0;i< server->rooms.len;i++)
    {
        struct room* room = server->rooms.data[i];
        free(room);
    }
    MyVectorDestroy(&server->players);
    MyVectorDestroy(&server->rooms);
    //MyLinkedDictDestroy(&server->rooms);
}

void server_recv_message(struct server* server, struct message* message); // takes ownership of the message






int do_move(struct player *player, struct room *room, int move)
{
    if(player == NULL)
        return -1;
    if(player->side == 0 || room == NULL)
        return -2;
    if((player->side == 1 && room->state != XES_TURN) || (player->side == -1 && room->state != OS_TURN))
        return -4;
    char* field = &room->board.colors[move/3][move%3];
    if(*field != 0)
        return -3;
    *field = (char)player->side;
    char ret = check_solution(&room->board);
    if(ret == 1)
    {
        room->state = XES_WIN;
        return 1;
    }
    if(ret == -1)
    {
        room->state = OS_WIN;
        return 2;
    }
    if(ret == -2)
    {
        room->state = DRAW;
        return 3;
    }

    if(player->side == 1)
        room->state = OS_TURN;
    else
        room->state = XES_TURN;

    return 0;
}

void server_send_board_update_for_room(struct server* server, struct room* room)
{
    struct message* board_state_message = malloc(sizeof(struct message));
    board_state_message->type = STC_BOARD_UPDATE;
    board_state_message->player_id = room->x_player_id;
    board_state_message->payload = malloc(sizeof(struct board));
    memcpy(board_state_message->payload, &room->board, sizeof(struct board));
    server->send_message(board_state_message, server->send_context);

    board_state_message = malloc(sizeof(struct message));
    board_state_message->type = STC_BOARD_UPDATE;
    board_state_message->player_id = room->o_player_id;
    board_state_message->payload = malloc(sizeof(struct board));
    memcpy(board_state_message->payload, &room->board, sizeof(struct board));
    server->send_message(board_state_message, server->send_context);
}

void init_room(struct room* room, struct player* player_1, struct player* player_2)
{
    if(rand()%2)
    {
        struct player* tmp = player_1;
        player_1 = player_2;
        player_2 = tmp;
    }
    room->x_player_id = player_1->player_id;
    player_1->side = 1;
    room->o_player_id = player_2->player_id;
    player_2->side = -1;
    room->state = XES_TURN;
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<3;j++)
            room->board.colors[i][j] = 0;
    }
}
char* get_player_name_reference(struct server* server, int player_id)
{
    for(int i= 0;i< server->players.len;i++)
    {
        struct player* player = server->players.data[i];
        if(player->player_id == player_id)
        {
            return player->name;
        }
    }
    return NULL;
}

void server_send_paired_message_for_room(struct server* server, struct room* room)
{
    struct message* paired_message = malloc(sizeof(struct message));
    paired_message->type = STC_PAIRED;
    paired_message->player_id = room->x_player_id;
    paired_message->payload = malloc(sizeof(struct stc_paired_message_payload));
    struct stc_paired_message_payload* payload = paired_message->payload;
    payload->other_players_name = c_str_copy(get_player_name_reference(server,room->o_player_id));
    payload->side = 1;
    server->send_message(paired_message, server->send_context);

    paired_message = malloc(sizeof(struct message));
    paired_message->type = STC_PAIRED;
    paired_message->player_id = room->o_player_id;
    paired_message->payload = malloc(sizeof(struct stc_paired_message_payload));
    payload = paired_message->payload;
    payload->other_players_name = c_str_copy(get_player_name_reference(server,room->x_player_id));
    payload->side = -1;
    server->send_message(paired_message, server->send_context);
}
void server_send_rejected_move_message_to_player(struct server* server, int player_id, char* reason)
{
    struct message* message = malloc(sizeof(struct message));
    message->player_id = player_id;
    message->payload = c_str_copy(reason);
    message->type = STC_MOVE_REJECTED;
    server->send_message(message, server->send_context);
}


void server_get_player_and_room(struct server* server, int player_id, struct player** player, struct room** room)
{
    *room = NULL;
    *player = NULL;
    for(int i= 0;i< server->players.len;i++)
    {
        struct player* p = server->players.data[i];
        if(p->player_id == player_id)
        {
            *player = p;
            break;
        }
    }
    if(*player == NULL || (*player)->room_no == -1)
        return;
    for(int i= 0;i< server->rooms.len;i++)
    {
        struct room* r = server->rooms.data[i];
        if(r->room_id == (*player)->room_no)
        {
            *room = r;
            break;
        }
    }

//    *room = MyLinkedDictGet(&server->rooms, &(*player)->room_no);
}

void server_send_other_side_disconnected_message(struct server* server, int player_id)
{
    struct message* message = malloc(sizeof (struct message));
    message->player_id = player_id;
    message->payload = NULL;
    message->type = STC_DISCONNECTED;
    server->send_message(message, server->send_context);
}

void server_pair_or_wait_player(struct server* server, struct player* player)
{
    if (server->waiting_player == NULL) {
        struct message *wait_message = malloc(sizeof(struct message));
        wait_message->type = STC_WAITING;
        wait_message->player_id = player->player_id;
        wait_message->payload = NULL;
        server->send_message(wait_message, server->send_context);
        server->waiting_player = player;
    } else {
        struct room *new_room = malloc(sizeof(struct room));
        init_room(new_room, player, server->waiting_player);
        player->room_no = server->last_room_id;
        server->waiting_player->room_no = server->last_room_id;
        new_room->room_id = server->last_room_id;
        MyVectorAppend(&server->rooms,new_room);
//        if (MyLinkedDictInsert(&server->rooms, &server->last_room_id, new_room, 0)) {
//            fprintf(stderr, "cannot insert newly created room to the room dict");
//            exit(-1);
//        }
        server->last_room_id++;
        server_send_paired_message_for_room(server, new_room);
        server_send_board_update_for_room(server, new_room);
        server->waiting_player = NULL;
    }
}
void server_remove_player(struct server* server, int player_id);
void server_add_player(struct server* server, int player_id, char* name)
{
    for(int i=0;i < server->players.len;i++)
    {
        struct player* p = server->players.data[i];
        if(p->player_id == player_id)
        {
            server_remove_player(server, player_id);
            break;
        }
    }
    int ret = 1;
    for(int i=0;i < server->players.len;i++)
    {
        struct player* p = server->players.data[i];
        if(!strcmp(p->name, name))
        {
            ret = 0;
            break;
        }
    }
    if (!ret) {
        struct message *rejection_message = malloc(sizeof(struct message));
        rejection_message->type = STC_NAME_REJECTED;
        rejection_message->player_id = player_id;
        rejection_message->payload = NULL;
        server->send_message(rejection_message, server->send_context);
    } else {
        printf("player with name %s joined from id: %d\n", name, player_id);
        struct player *new_player = malloc(sizeof(struct player));
        init_player(new_player, name, player_id);
        MyVectorAppend(&server->players, new_player);
        server_pair_or_wait_player(server, new_player);
    }
}
void server_send_game_ended_message_to_room(struct server* server, struct room* room, int who_won)
{
    struct message* game_ended_message = malloc(sizeof(struct message));
    game_ended_message->type = STC_GAME_ENDED;
    game_ended_message->player_id = room->x_player_id;
    game_ended_message->payload = malloc(sizeof(int));
    *(int*)game_ended_message->payload = who_won;
    server->send_message(game_ended_message, server->send_context);

    game_ended_message = malloc(sizeof(struct message));
    game_ended_message->type = STC_GAME_ENDED;
    game_ended_message->player_id = room->o_player_id;
    game_ended_message->payload = malloc(sizeof(int));
    *(int*)game_ended_message->payload = who_won;
    server->send_message(game_ended_message, server->send_context);
}
void server_recv_message(struct server* server, struct message* message) // takes ownership of the message
{
    switch (message->type) {
        case CTS_INIT: {
            server_add_player(server, message->player_id, (char*)message->payload);
            break;
        }
        case STC_PAIRED:
        case STC_BOARD_UPDATE:
        case STC_NAME_REJECTED:
        case STC_WAITING:
        case STC_DISCONNECTED:
        case STC_SHUTDOWN:
        case STC_GAME_ENDED:
        case STC_MOVE_REJECTED: {
            fprintf(stderr, "server received message type intended for the client, ignoring\n");
            break;
        }
        case CTS_PERFORM_MOVE: {
            struct player* player;
            struct room* room;
            server_get_player_and_room(server, message->player_id, &player, &room);
            int ret = do_move(player, room, *(int*)message->payload);
            if(ret == -1) {
                fprintf(stderr, "tried to perform action by a non-existent player\n");
            }
            else if(ret == -2) {
                server_send_rejected_move_message_to_player(server, message->player_id, "Not in room");
            }
            else if(ret == -3) {
                server_send_rejected_move_message_to_player(server, message->player_id, "field already taken");
            }
            else if(ret == -4) {
                server_send_rejected_move_message_to_player(server, message->player_id, "Not your turn");
            }
            else if(ret == 1)
            {
                server_send_board_update_for_room(server, room);
                server_send_game_ended_message_to_room(server, room, 1);
            }
            else if(ret == 2)
            {
                server_send_board_update_for_room(server, room);
                server_send_game_ended_message_to_room(server, room, -1);
            }
            else if(ret == 3)
            {
                server_send_board_update_for_room(server, room);
                server_send_game_ended_message_to_room(server, room, 0);
            }
            else {
                server_send_board_update_for_room(server, room);
            }
            break;
        }
        case CTS_DISCONNECTED:
        {
            server_remove_player(server, message->player_id);
            break;
        }
    }
    free_message(message);
}
void server_remove_player(struct server* server, int player_id)
{
    struct player* player;
    struct room* room;
    server_get_player_and_room(server, player_id, &player, &room);
    if(player == NULL)
        return;
    if(player == server->waiting_player)
        server->waiting_player = NULL;
    if(room != NULL)
    {
        struct player* other = NULL;
        if(player->side == 1) {
            server_send_other_side_disconnected_message(server, room->o_player_id);
            other = server_get_player(server,room->o_player_id);
        }
        else if(player->side == -1) {
            server_send_other_side_disconnected_message(server, room->x_player_id);
            other = server_get_player(server,room->x_player_id);
        }
        if(other != NULL)
        {
            other->side = 0;
            other->room_no = -1;
            int room_index = -1;
            for(int i=0;i < server->rooms.len;i++)
            {
                if(server->rooms.data[i] == room)
                {
                    room_index = i;
                    break;
                }
            }
            free(room);
            MyVectorRemoveAndReplaceWithLast(&server->rooms, room_index);
            //MyLinkedDictDelete(&server->rooms, &player->room_no);
            server_pair_or_wait_player(server, other);
        }
    }
    for(int i=0;i<server->players.len;i++)
    {
        struct player* p = server->players.data[i];
        if(p == player)
        {
            MyVectorRemoveAndReplaceWithLast(&server->players,i);
            break;
        }
    }
}


