#include "queue.h"

#include <stdlib.h>

int queue_init(Queue *queue, int capacity)
{
    if (queue == NULL || capacity <= 0) {
        return -1;
    }

    queue->items = calloc((size_t)capacity, sizeof(int));
    if (queue->items == NULL) {
        return -1;
    }

    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
    return 0;
}

void queue_destroy(Queue *queue)
{
    if (queue == NULL) {
        return;
    }

    free(queue->items);
    queue->items = NULL;
    queue->capacity = 0;
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
}

void queue_clear(Queue *queue)
{
    if (queue == NULL) {
        return;
    }

    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
}

int queue_is_empty(const Queue *queue)
{
    return queue == NULL || queue->size == 0;
}

int queue_is_full(const Queue *queue)
{
    return queue != NULL && queue->size == queue->capacity;
}

int queue_size(const Queue *queue)
{
    if (queue == NULL) {
        return 0;
    }

    return queue->size;
}

int queue_capacity(const Queue *queue)
{
    if (queue == NULL) {
        return 0;
    }

    return queue->capacity;
}

int queue_push(Queue *queue, int item)
{
    if (queue == NULL || queue->items == NULL || queue_is_full(queue)) {
        return -1;
    }

    queue->items[queue->rear] = item;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->size++;
    return 0;
}

int queue_pop(Queue *queue, int *item)
{
    if (queue == NULL || queue->items == NULL || queue_is_empty(queue)) {
        return -1;
    }

    if (item != NULL) {
        *item = queue->items[queue->front];
    }

    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return 0;
}

int queue_peek(const Queue *queue, int *item)
{
    if (queue == NULL || queue->items == NULL || queue_is_empty(queue)) {
        return -1;
    }

    if (item != NULL) {
        *item = queue->items[queue->front];
    }

    return 0;
}

int queue_at(const Queue *queue, int index, int *item)
{
    if (queue == NULL || queue->items == NULL || index < 0 || index >= queue->size) {
        return -1;
    }

    if (item != NULL) {
        int real_index = (queue->front + index) % queue->capacity;
        *item = queue->items[real_index];
    }

    return 0;
}

int queue_contains(const Queue *queue, int item)
{
    if (queue == NULL || queue->items == NULL) {
        return 0;
    }

    for (int i = 0; i < queue->size; i++) {
        int current = 0;
        if (queue_at(queue, i, &current) == 0 && current == item) {
            return 1;
        }
    }

    return 0;
}

int queue_remove_value(Queue *queue, int item)
{
    if (queue == NULL || queue->items == NULL || queue_is_empty(queue)) {
        return -1;
    }

    int original_size = queue->size;
    int removed = 0;

    for (int i = 0; i < original_size; i++) {
        int current = 0;
        queue_pop(queue, &current);
        if (!removed && current == item) {
            removed = 1;
            continue;
        }
        queue_push(queue, current);
    }

    return removed ? 0 : -1;
}
