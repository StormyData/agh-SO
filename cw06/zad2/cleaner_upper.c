#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"



int main(int argc, char** argv)
{
    if(argc != 2)
    {
        mq_unlink(server_key);
        exit(0);
    }
    int proj_id = atoi(argv[1]);
    char our_key[M_NAME_MAX];
    sprintf(our_key,"/systemy_operacyjne_cw06_zad2_client%d",proj_id);
    mq_unlink(our_key);

}