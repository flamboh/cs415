#include "config.h"
#include "park_setup.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static long park_time(Park *park)
{
    return (long)(time(NULL) - park->start_time);
}

static void log_event(Park *park, const char *fmt, ...)
{
    va_list args;

    printf("[Time: %ld] ", park_time(park));
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
}

static void broadcast_all(Park *park)
{
    pthread_cond_broadcast(&park->ticket_cv);
    pthread_cond_broadcast(&park->ride_cv);
    pthread_cond_broadcast(&park->load_cv);
    pthread_cond_broadcast(&park->unload_cv);
    for (int i = 0; i < park->n; i++) {
        pthread_cond_broadcast(&park->passengers[i].cond);
    }
}

static int queue_front_is(Queue *queue, int id)
{
    int front = -1;
    return queue_peek(queue, &front) == 0 && front == id;
}

static void make_deadline(struct timespec *deadline, int seconds)
{
    clock_gettime(CLOCK_REALTIME, deadline);
    deadline->tv_sec += seconds;
}

static int sleep_while_open(Park *park, int seconds)
{
    for (int i = 0; i < seconds; i++) {
        sleep(1);
        pthread_mutex_lock(&park->lock);
        int open = park->park_open;
        pthread_mutex_unlock(&park->lock);
        if (!open) {
            return 0;
        }
    }
    return 1;
}

static void passenger_leave_queues(Park *park, int passenger_id)
{
    queue_remove_value(&park->ticket_queue, passenger_id);
    queue_remove_value(&park->ride_queue, passenger_id);
    pthread_cond_broadcast(&park->ticket_cv);
    pthread_cond_broadcast(&park->ride_cv);
    pthread_cond_broadcast(&park->load_cv);
}

static int get_ticket_and_enter_ride_queue(Park *park, Passenger *passenger)
{
    pthread_mutex_lock(&park->lock);
    if (!park->park_open) {
        pthread_mutex_unlock(&park->lock);
        return 0;
    }

    passenger->state = PASSENGER_TICKET_QUEUE;
    queue_push(&park->ticket_queue, passenger->id);
    log_event(park, "Passenger %d entering the ticket queue", passenger->id);
    pthread_cond_broadcast(&park->ticket_cv);

    int reported_full = 0;
    while (park->park_open) {
        if (queue_front_is(&park->ticket_queue, passenger->id) &&
            !queue_is_full(&park->ride_queue)) {
            int ignored = -1;
            queue_pop(&park->ticket_queue, &ignored);
            log_event(park, "Passenger %d acquired a ticket", passenger->id);

            passenger->state = PASSENGER_RIDE_QUEUE;
            queue_push(&park->ride_queue, passenger->id);
            log_event(park, "Passenger %d has entered the ride queue", passenger->id);
            pthread_cond_broadcast(&park->ticket_cv);
            pthread_cond_broadcast(&park->ride_cv);
            pthread_cond_broadcast(&park->load_cv);
            pthread_mutex_unlock(&park->lock);
            return 1;
        }

        if (queue_front_is(&park->ticket_queue, passenger->id) &&
            queue_is_full(&park->ride_queue) && !reported_full) {
            log_event(park, "Ride queue full; Passenger %d waits for ticket", passenger->id);
            reported_full = 1;
        }
        pthread_cond_wait(&park->ticket_cv, &park->lock);
    }

    passenger_leave_queues(park, passenger->id);
    pthread_mutex_unlock(&park->lock);
    return 0;
}

static int wait_to_board(Park *park, Passenger *passenger)
{
    pthread_mutex_lock(&park->lock);
    while (!passenger->can_board) {
        if (!park->park_open) {
            passenger_leave_queues(park, passenger->id);
            pthread_mutex_unlock(&park->lock);
            return 0;
        }
        pthread_cond_wait(&passenger->cond, &park->lock);
    }

    passenger->can_board = 0;
    passenger->state = PASSENGER_RIDING;
    pthread_mutex_unlock(&park->lock);
    return 1;
}

static void wait_to_unboard(Park *park, Passenger *passenger)
{
    pthread_mutex_lock(&park->lock);
    while (!passenger->can_unboard) {
        pthread_cond_wait(&passenger->cond, &park->lock);
    }

    int car_id = passenger->assigned_car;
    passenger->can_unboard = 0;
    passenger->assigned_car = -1;
    passenger->completed_rides++;
    passenger->state = PASSENGER_DONE;
    park->cars[car_id].unboarded_count++;
    log_event(park, "Passenger %d unboarded from Car %d", passenger->id, car_id);
    pthread_cond_broadcast(&park->unload_cv);
    pthread_mutex_unlock(&park->lock);
}

static void *passenger_thread(void *arg)
{
    ThreadArg *thread_arg = arg;
    Park *park = thread_arg->park;
    int id = thread_arg->id;
    Passenger *passenger = &park->passengers[id];
    unsigned int seed = (unsigned int)(time(NULL) ^ (id * 1103515245u));
    free(thread_arg);

    pthread_mutex_lock(&park->lock);
    log_event(park, "Passenger %d entered the park", id);
    pthread_mutex_unlock(&park->lock);

    while (1) {
        pthread_mutex_lock(&park->lock);
        if (!park->park_open) {
            pthread_mutex_unlock(&park->lock);
            break;
        }
        passenger->state = PASSENGER_EXPLORING;
        log_event(park, "Passenger %d is exploring the park", id);
        pthread_mutex_unlock(&park->lock);

        int explore_time = (int)(rand_r(&seed) % 10u) + 1;
        if (!sleep_while_open(park, explore_time)) {
            break;
        }

        pthread_mutex_lock(&park->lock);
        log_event(park, "Passenger %d finished exploring, entering the ticket booth", id);
        pthread_mutex_unlock(&park->lock);

        if (!get_ticket_and_enter_ride_queue(park, passenger)) {
            break;
        }
        if (!wait_to_board(park, passenger)) {
            break;
        }
        wait_to_unboard(park, passenger);
    }

    pthread_mutex_lock(&park->lock);
    passenger->state = PASSENGER_DONE;
    log_event(park, "Passenger %d exited the park after %d ride(s)", id,
              passenger->completed_rides);
    pthread_mutex_unlock(&park->lock);
    return NULL;
}

static int board_available_passengers(Park *park, Car *car)
{
    int boarded = 0;

    while (car->onboard_count < park->p && !queue_is_empty(&park->ride_queue)) {
        int passenger_id = -1;
        Passenger *passenger = NULL;

        queue_pop(&park->ride_queue, &passenger_id);
        passenger = &park->passengers[passenger_id];
        passenger->assigned_car = car->id;
        passenger->can_board = 1;
        passenger->state = PASSENGER_BOARDING;

        car->passengers[car->onboard_count] = passenger_id;
        car->onboard_count++;
        boarded++;

        log_event(park, "Passenger %d is boarding Car %d", passenger_id, car->id);
        pthread_cond_signal(&passenger->cond);
        pthread_cond_broadcast(&park->ticket_cv);
    }

    return boarded;
}

static int car_load(Park *park, Car *car)
{
    int departed = 0;
    int expired = 0;
    struct timespec deadline;

    pthread_mutex_lock(&park->lock);
    if (!park->park_open) {
        pthread_mutex_unlock(&park->lock);
        return 0;
    }

    car->state = CAR_WAITING_TO_LOAD;
    queue_push(&park->load_queue, car->id);
    pthread_cond_broadcast(&park->load_cv);

    while (park->park_open && !queue_front_is(&park->load_queue, car->id)) {
        pthread_cond_wait(&park->load_cv, &park->lock);
    }

    if (!park->park_open) {
        queue_remove_value(&park->load_queue, car->id);
        pthread_cond_broadcast(&park->load_cv);
        pthread_mutex_unlock(&park->lock);
        return 0;
    }

    car->state = CAR_LOADING;
    car->onboard_count = 0;
    car->unboarded_count = 0;
    log_event(park, "Car %d invoked load()", car->id);

    while (park->park_open || car->onboard_count > 0) {
        if (board_available_passengers(park, car) > 0) {
            make_deadline(&deadline, park->w);
            expired = 0;
        }

        if (car->onboard_count == park->p) {
            log_event(park, "Car %d is full with %d passengers", car->id,
                      car->onboard_count);
            departed = 1;
            break;
        }

        if (car->onboard_count == 0) {
            if (!park->park_open) {
                break;
            }
            pthread_cond_wait(&park->ride_cv, &park->lock);
            continue;
        }

        if (!park->park_open) {
            departed = 1;
            break;
        }

        int rc = pthread_cond_timedwait(&park->ride_cv, &park->lock, &deadline);
        if (rc == ETIMEDOUT && !expired) {
            log_event(park, "Car %d waiting period expired", car->id);
            departed = 1;
            expired = 1;
            break;
        }
    }

    queue_remove_value(&park->load_queue, car->id);
    pthread_cond_broadcast(&park->load_cv);

    if (departed) {
        car->state = CAR_RUNNING;
        queue_push(&park->unload_queue, car->id);
        log_event(park, "Car %d has departed to ride with %d passenger(s)", car->id,
                  car->onboard_count);
    }

    pthread_mutex_unlock(&park->lock);
    return departed;
}

static void car_run(Park *park, Car *car)
{
    sleep((unsigned int)park->r);

    pthread_mutex_lock(&park->lock);
    log_event(park, "Car %d has returned from the ride", car->id);
    car->state = CAR_WAITING_TO_UNLOAD;
    pthread_mutex_unlock(&park->lock);
}

static void car_unload(Park *park, Car *car)
{
    pthread_mutex_lock(&park->lock);
    while (!queue_front_is(&park->unload_queue, car->id)) {
        pthread_cond_wait(&park->unload_cv, &park->lock);
    }

    car->state = CAR_UNLOADING;
    log_event(park, "Car %d has invoked unload()", car->id);
    for (int i = 0; i < car->onboard_count; i++) {
        int passenger_id = car->passengers[i];
        Passenger *passenger = &park->passengers[passenger_id];
        passenger->can_unboard = 1;
        pthread_cond_signal(&passenger->cond);
    }

    while (car->unboarded_count < car->onboard_count) {
        pthread_cond_wait(&park->unload_cv, &park->lock);
    }

    queue_remove_value(&park->unload_queue, car->id);
    log_event(park, "Car %d finished unloading", car->id);
    car->onboard_count = 0;
    car->unboarded_count = 0;
    car->state = CAR_WAITING_TO_LOAD;
    pthread_cond_broadcast(&park->unload_cv);
    pthread_mutex_unlock(&park->lock);
}

static void *car_thread(void *arg)
{
    ThreadArg *thread_arg = arg;
    Park *park = thread_arg->park;
    int id = thread_arg->id;
    Car *car = &park->cars[id];
    free(thread_arg);

    while (car_load(park, car)) {
        car_run(park, car);
        car_unload(park, car);
    }

    pthread_mutex_lock(&park->lock);
    car->state = CAR_WAITING_TO_LOAD;
    log_event(park, "Car %d stopped", id);
    pthread_mutex_unlock(&park->lock);
    return NULL;
}

int main(int argc, char **argv)
{
    Park park;
    pthread_t *passenger_threads = NULL;
    pthread_t *car_threads = NULL;

    memset(&park, 0, sizeof(park));
    parse_args(argc, argv, &park);
    if (park_init(&park) != 0) {
        fprintf(stderr, "Failed to initialize park\n");
        park_destroy(&park);
        return EXIT_FAILURE;
    }

    print_config(&park);
    passenger_threads = calloc((size_t)park.n, sizeof(pthread_t));
    car_threads = calloc((size_t)park.c, sizeof(pthread_t));
    if (passenger_threads == NULL || car_threads == NULL) {
        fprintf(stderr, "Failed to allocate thread handles\n");
        park_destroy(&park);
        free(passenger_threads);
        free(car_threads);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < park.n; i++) {
        if (pthread_create(&passenger_threads[i], NULL, passenger_thread,
                           make_arg(&park, i)) != 0) {
            fprintf(stderr, "Failed to create passenger thread %d\n", i);
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < park.c; i++) {
        if (pthread_create(&car_threads[i], NULL, car_thread, make_arg(&park, i)) != 0) {
            fprintf(stderr, "Failed to create car thread %d\n", i);
            return EXIT_FAILURE;
        }
    }

    sleep((unsigned int)park.t);
    pthread_mutex_lock(&park.lock);
    park.park_open = 0;
    log_event(&park, "Park closed");
    broadcast_all(&park);
    pthread_mutex_unlock(&park.lock);

    for (int i = 0; i < park.n; i++) {
        pthread_join(passenger_threads[i], NULL);
    }
    pthread_mutex_lock(&park.lock);
    broadcast_all(&park);
    pthread_mutex_unlock(&park.lock);
    for (int i = 0; i < park.c; i++) {
        pthread_join(car_threads[i], NULL);
    }

    free(passenger_threads);
    free(car_threads);
    park_destroy(&park);
    return EXIT_SUCCESS;
}
