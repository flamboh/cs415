#include <pthread.h>

typedef struct Passenger {
    int id;
    int assigned_car;
    int can_board;
    int can_unboard;
    pthread_cond_t cond;
} Passenger;

typedef struct Car {
    int id;
    int state; // WAITING, LOADING, RIDING, UNLOADING
    int onboard_count;
    int* passengers;
    int load_order;
} Car;

typedef struct Queue {
    int *items;
    int capacity;
    int front;
    int rear;
    int size;
} Queue;
