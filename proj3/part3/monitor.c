#include "monitor.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static long monitor_time(Park *park)
{
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

static void append_queue(char *buffer, size_t size, size_t *used, Queue *queue)
{
    append_text(buffer, size, used, "[");
    for (int i = 0; i < queue_size(queue); i++) {
        int passenger_id = -1;
        queue_at(queue, i, &passenger_id);
        append_text(buffer, size, used, "%s%d", i == 0 ? "" : ", ", passenger_id);
    }
    append_text(buffer, size, used, "]");
}

void send_monitor_snapshot(Park *park, int fd)
{
    char buffer[4096];
    size_t used = 0;
    int ticket_size = 0;
    int ticket_capacity = 0;
    int ride_size = 0;
    int ride_capacity = 0;

    pthread_mutex_lock(&park->lock);
    ticket_size = queue_size(&park->ticket_queue);
    ticket_capacity = queue_capacity(&park->ticket_queue);
    ride_size = queue_size(&park->ride_queue);
    ride_capacity = queue_capacity(&park->ride_queue);

    append_text(buffer, sizeof(buffer), &used, "[Time: %ld] Monitor: ticket queue ",
                monitor_time(park));
    append_queue(buffer, sizeof(buffer), &used, &park->ticket_queue);
    append_text(buffer, sizeof(buffer), &used, " (%d/%d), ride queue ", ticket_size,
                ticket_capacity);
    append_queue(buffer, sizeof(buffer), &used, &park->ride_queue);
    append_text(buffer, sizeof(buffer), &used, " (%d/%d)%s\n", ride_size,
                ride_capacity, park->park_open ? "" : " -- park closed");
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
