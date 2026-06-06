#include "config.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static void usage(const char *program)
{
    printf("Usage: %s [OPTIONS]\n", program);
    printf("  -n passengers\n");
    printf("  -c cars\n");
    printf("  -p capacity per car\n");
    printf("  -w wait seconds for partial car\n");
    printf("  -r ride seconds\n");
    printf("  -t park open seconds\n");
    printf("  -j max ride queue size\n");
    printf("  -h help\n");
}

void print_config(Park *park)
{
    printf("- Number of passenger threads: %d\n", park->n);
    printf("- Number of cars: %d\n", park->c);
    printf("- Capacity per car: %d\n", park->p);
    printf("- Car waiting period: %d\n", park->w);
    printf("- Car ride duration: %d\n", park->r);
    printf("- Park duration: %d seconds\n", park->t);
    printf("- Max ride queue size: %d\n\n", park->j);
}

void parse_args(int argc, char **argv, Park *park)
{
    int opt = 0;

    park->n = 10;
    park->c = 2;
    park->p = 2;
    park->w = 1;
    park->r = 1;
    park->t = 30;
    park->j = 3;

    while ((opt = getopt(argc, argv, "n:c:p:w:r:t:j:h")) != -1) {
        switch (opt) {
            case 'n': park->n = atoi(optarg); break;
            case 'c': park->c = atoi(optarg); break;
            case 'p': park->p = atoi(optarg); break;
            case 'w': park->w = atoi(optarg); break;
            case 'r': park->r = atoi(optarg); break;
            case 't': park->t = atoi(optarg); break;
            case 'j': park->j = atoi(optarg); break;
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (park->n < 1) park->n = 1;
    if (park->c < 1) park->c = 1;
    if (park->p < 1) park->p = 1;
    if (park->w < 1) park->w = 1;
    if (park->r < 1) park->r = 1;
    if (park->t < 1) park->t = 1;
    if (park->j < 1) park->j = 1;
    if (park->p > park->n) park->p = park->n;
}
