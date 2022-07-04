#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"

int main(int argc, char** argv)
{
    srand(time(NULL));
    if(argc != 4)
    {
        printf("invalid number of arguments\n");
        exit(-1);
    }
    struct pgm_img img;
    img.width = atoi(argv[1]);
    img.height = atoi(argv[2]);
    int n = img.width * img.height;
    img.tab = malloc(n);
    for(int i=0;i<n;i++)
        img.tab[i] = rand()%256;

    store_pgm(argv[3], &img);
    free(img.tab);

}