#include "park_setup.h"

#include <stdlib.h>
#include <time.h>

int park_init(Park *park)
{
    park->park_open = 1;
    park->start_time = time(NULL);

    if (pthread_mutex_init(&park->lock, NULL) != 0 ||
        pthread_cond_init(&park->ticket_cv, NULL) != 0 ||
        pthread_cond_init(&park->ride_cv, NULL) != 0 ||
        pthread_cond_init(&park->load_cv, NULL) != 0 ||
        pthread_cond_init(&park->unload_cv, NULL) != 0) {
        return -1;
    }

    park->passengers = calloc((size_t)park->n, sizeof(Passenger));
    park->cars = calloc((size_t)park->c, sizeof(Car));
    if (park->passengers == NULL || park->cars == NULL) {
        return -1;
    }

    if (queue_init(&park->ticket_queue, park->n) != 0 ||
        queue_init(&park->ride_queue, park->j) != 0 ||
        queue_init(&park->load_queue, park->c) != 0 ||
        queue_init(&park->unload_queue, park->c) != 0) {
        return -1;
    }

    for (int i = 0; i < park->n; i++) {
        park->passengers[i].id = i;
        park->passengers[i].assigned_car = -1;
        park->passengers[i].state = PASSENGER_DONE;
        pthread_cond_init(&park->passengers[i].cond, NULL);
    }

    for (int i = 0; i < park->c; i++) {
        park->cars[i].id = i;
        park->cars[i].state = CAR_WAITING_TO_LOAD;
        park->cars[i].passengers = calloc((size_t)park->p, sizeof(int));
        if (park->cars[i].passengers == NULL) {
            return -1;
        }
    }

    return 0;
}

void park_destroy(Park *park)
{
    for (int i = 0; i < park->n; i++) {
        pthread_cond_destroy(&park->passengers[i].cond);
    }
    for (int i = 0; i < park->c; i++) {
        free(park->cars[i].passengers);
    }

    queue_destroy(&park->ticket_queue);
    queue_destroy(&park->ride_queue);
    queue_destroy(&park->load_queue);
    queue_destroy(&park->unload_queue);

    pthread_cond_destroy(&park->ticket_cv);
    pthread_cond_destroy(&park->ride_cv);
    pthread_cond_destroy(&park->load_cv);
    pthread_cond_destroy(&park->unload_cv);
    pthread_mutex_destroy(&park->lock);

    free(park->passengers);
    free(park->cars);
}

ThreadArg *make_arg(Park *park, int id)
{
    ThreadArg *arg = malloc(sizeof(ThreadArg));
    if (arg == NULL) {
        return NULL;
    }
    arg->park = park;
    arg->id = id;
    return arg;
}
