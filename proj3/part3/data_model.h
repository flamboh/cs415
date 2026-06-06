#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <pthread.h>
#include <time.h>

#include "queue.h"

typedef enum {
    PASSENGER_EXPLORING,
    PASSENGER_TICKET_QUEUE,
    PASSENGER_WAITING_TO_BOARD,
    PASSENGER_CAN_BOARD,
    PASSENGER_RIDING,
    PASSENGER_CAN_UNBOARD,
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
    int completed_rides;
    time_t ticket_queue_entered_at;
    time_t ride_queue_entered_at;
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
    int fast_mode;
    int current_time;
    time_t start_time;

    pthread_mutex_t lock;
    pthread_cond_t tick_cv;
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

    long total_ticket_queue_seconds;
    long total_ride_queue_seconds;
    int ticket_queue_samples;
    int ride_queue_samples;
    int total_passengers_served;
    int total_car_runs;
    int total_car_passengers;
} Park;

typedef struct ThreadArg {
    Park *park;
    int id;
} ThreadArg;

#endif
