#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <pthread.h>
#include <time.h>

#include "queue.h"

typedef enum {
    PASSENGER_EXPLORING,
    PASSENGER_TICKET_QUEUE,
    PASSENGER_RIDE_QUEUE,
    PASSENGER_BOARDING,
    PASSENGER_RIDING,
    PASSENGER_DONE
} PassengerState;

typedef enum {
    CAR_WAITING_TO_LOAD,
    CAR_LOADING,
    CAR_RUNNING,
    CAR_WAITING_TO_UNLOAD,
    CAR_UNLOADING
} CarState;

typedef struct Passenger {
    int id;
    int assigned_car;
    int can_board;
    int can_unboard;
    int completed_rides;
    PassengerState state;
    pthread_cond_t cond;
} Passenger;

typedef struct Car {
    int id;
    CarState state;
    int onboard_count;
    int unboarded_count;
    int *passengers;
} Car;

typedef struct Park {
    int n;
    int c;
    int p;
    int w;
    int r;
    int t;
    int j;
    int park_open;
    time_t start_time;

    pthread_mutex_t lock;
    pthread_cond_t ticket_cv;
    pthread_cond_t ride_cv;
    pthread_cond_t load_cv;
    pthread_cond_t unload_cv;

    Queue ticket_queue;
    Queue ride_queue;
    Queue load_queue;
    Queue unload_queue;

    Passenger *passengers;
    Car *cars;
} Park;

typedef struct ThreadArg {
    Park *park;
    int id;
} ThreadArg;

#endif
