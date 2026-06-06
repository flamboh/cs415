#include "monitor.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static long monitor_time(Park *park)
{
    if (park->fast_mode) {
        return park->current_time;
    }
    return (long)(time(NULL) - park->start_time);
}

static void append_text(char *buffer, size_t size, size_t *used, const char *fmt, ...)
{
    va_list args;
    int written = 0;

    if (*used >= size) {
        return;
    }

    va_start(args, fmt);
    written = vsnprintf(buffer + *used, size - *used, fmt, args);
    va_end(args);

    if (written < 0) {
        return;
    }
    if ((size_t)written >= size - *used) {
        *used = size - 1;
        return;
    }
    *used += (size_t)written;
}

static const char *car_state_name(CarState state)
{
    switch (state) {
        case CAR_WAITING_TO_LOAD:
            return "WAITING";
        case CAR_LOADING:
            return "LOADING";
        case CAR_RUNNING:
            return "RIDING";
        case CAR_WAITING_TO_UNLOAD:
            return "WAITING TO UNLOAD";
        case CAR_UNLOADING:
            return "UNLOADING";
        default:
            return "UNKNOWN";
    }
}

static void append_queue(char *buffer, size_t size, size_t *used, Queue *queue)
{
    append_text(buffer, size, used, "[");
    for (int i = 0; i < queue_size(queue); i++) {
        int passenger_id = -1;
        queue_at(queue, i, &passenger_id);
        append_text(buffer, size, used, "%sPassenger %d", i == 0 ? "" : ", ",
                    passenger_id);
    }
    append_text(buffer, size, used, "]");
}

static void count_passengers(Park *park, int *in_park, int *exploring, int *in_queues,
                             int *waiting_in_car, int *riding)
{
    *in_park = 0;
    *exploring = 0;
    *in_queues = 0;
    *waiting_in_car = 0;
    *riding = 0;

    for (int i = 0; i < park->n; i++) {
        Passenger *passenger = &park->passengers[i];

        if (passenger->state != PASSENGER_DONE || park->park_open) {
            (*in_park)++;
        }

        if (passenger->assigned_car >= 0) {
            Car *car = &park->cars[passenger->assigned_car];
            if (car->state == CAR_LOADING) {
                (*waiting_in_car)++;
            } else {
                (*riding)++;
            }
            continue;
        }

        if (passenger->state == PASSENGER_EXPLORING ||
            (park->park_open && passenger->state == PASSENGER_DONE)) {
            (*exploring)++;
        } else if (passenger->state == PASSENGER_TICKET_QUEUE ||
                   passenger->state == PASSENGER_WAITING_TO_BOARD) {
            (*in_queues)++;
        }
    }
}

static void send_state_snapshot(Park *park, int fd, int next_timestep, int park_closed)
{
    char buffer[4096];
    size_t used = 0;
    int in_park = 0;
    int exploring = 0;
    int in_queues = 0;
    int waiting_in_car = 0;
    int riding = 0;

    pthread_mutex_lock(&park->lock);
    count_passengers(park, &in_park, &exploring, &in_queues, &waiting_in_car, &riding);

    if (park_closed) {
        append_text(buffer, sizeof(buffer), &used,
                    "\n[Time: %ld] [Monitor] ===== FINAL SYSTEM STATE =====\n",
                    monitor_time(park));
    } else {
        append_text(buffer, sizeof(buffer), &used,
                    "\n[Monitor] ===== SYSTEM STATE BEFORE TIME: %d =====\n",
                    next_timestep);
    }
    append_text(buffer, sizeof(buffer), &used, "[Monitor] Ticket Queue: ");
    append_queue(buffer, sizeof(buffer), &used, &park->ticket_queue);
    append_text(buffer, sizeof(buffer), &used, "\n[Monitor] Ride Queue: ");
    append_queue(buffer, sizeof(buffer), &used, &park->ride_queue);

    for (int i = 0; i < park->c; i++) {
        append_text(buffer, sizeof(buffer), &used,
                    "\n[Monitor] Car status %d: %s (%d/%d Passengers)", i,
                    car_state_name(park->cars[i].state), park->cars[i].onboard_count,
                    park->p);
    }

    append_text(buffer, sizeof(buffer), &used,
                "\n[Monitor] Passengers in the park: %d (%d exploring, %d in queues, "
                "%d waiting in a car, %d riding)%s\n",
                in_park, exploring, in_queues, waiting_in_car, riding,
                park->park_open ? "" : "\n[Monitor] Park is closed");
    append_text(buffer, sizeof(buffer), &used, "[Monitor] ========================\n\n");
    pthread_mutex_unlock(&park->lock);

    if (write(fd, buffer, used) < 0) {
        return;
    }
}

void send_monitor_snapshot(Park *park, int fd, int next_timestep)
{
    send_state_snapshot(park, fd, next_timestep, 0);
}

void send_closed_snapshot(Park *park, int fd)
{
    send_state_snapshot(park, fd, park->t, 1);
}

void send_final_statistics(Park *park, int fd)
{
    char buffer[2048];
    size_t used = 0;
    double avg_ticket_seconds = 0.0;
    double avg_ride_seconds = 0.0;
    double avg_passengers_per_car = 0.0;
    double avg_utilization = 0.0;

    pthread_mutex_lock(&park->lock);
    if (park->ticket_queue_samples > 0) {
        avg_ticket_seconds = (double)park->total_ticket_queue_seconds /
                             (double)park->ticket_queue_samples;
    }
    if (park->ride_queue_samples > 0) {
        avg_ride_seconds = (double)park->total_ride_queue_seconds /
                           (double)park->ride_queue_samples;
    }
    if (park->total_car_runs > 0) {
        avg_passengers_per_car = (double)park->total_car_passengers /
                                 (double)park->total_car_runs;
    }
    if (park->p > 0) {
        avg_utilization = (avg_passengers_per_car / (double)park->p) * 100.0;
    }

    append_text(buffer, sizeof(buffer), &used, "\n========== PARK CLOSED ==========\n");
    append_text(buffer, sizeof(buffer), &used, "[Monitor] FINAL STATISTICS:\n");
    append_text(buffer, sizeof(buffer), &used, "Total Simulation time: [Time: %d]\n",
                park->t);
    append_text(buffer, sizeof(buffer), &used, "Total Passengers Served: %d\n",
                park->total_passengers_served);
    append_text(buffer, sizeof(buffer), &used, "Total Rides: %d\n", park->total_car_runs);
    append_text(buffer, sizeof(buffer), &used, "Average Ticket Queue Seconds: %.1f\n",
                avg_ticket_seconds);
    append_text(buffer, sizeof(buffer), &used, "Average Ride Queue Seconds: %.1f\n",
                avg_ride_seconds);
    append_text(buffer, sizeof(buffer), &used,
                "Average Car Utilization: %.1f Percent (%.1f/%d Passengers Per Ride)\n",
                avg_utilization, avg_passengers_per_car, park->p);
    pthread_mutex_unlock(&park->lock);

    if (write(fd, buffer, used) < 0) {
        return;
    }
}

void monitor_process(int read_fd)
{
    char buffer[512];
    ssize_t bytes_read = 0;

    while ((bytes_read = read(read_fd, buffer, sizeof(buffer))) > 0) {
        if (write(STDOUT_FILENO, buffer, (size_t)bytes_read) < 0) {
            break;
        }
    }

    close(read_fd);
    _exit(EXIT_SUCCESS);
}
