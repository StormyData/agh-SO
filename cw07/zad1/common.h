#pragma once
#include <time.h>


struct pizza{
    int type;
};

struct oven{
    struct pizza pizzas[5];
    int last;
    int first;
    int size;
};

struct table{
    struct pizza pizzas[5];
    int last;
    int first;
    int size;
};

void random_sleep_between(float min, float max)
{
    usleep(((float)rand()/(float )RAND_MAX * (max - min) + min) * 1000000.0f);
}