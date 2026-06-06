#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue {
    int *items;
    int capacity;
    int front;
    int rear;
    int size;
} Queue;

int queue_init(Queue *queue, int capacity);
void queue_destroy(Queue *queue);
void queue_clear(Queue *queue);

int queue_is_empty(const Queue *queue);
int queue_is_full(const Queue *queue);
int queue_size(const Queue *queue);
int queue_capacity(const Queue *queue);

int queue_push(Queue *queue, int item);
int queue_pop(Queue *queue, int *item);
int queue_peek(const Queue *queue, int *item);
int queue_at(const Queue *queue, int index, int *item);
int queue_contains(const Queue *queue, int item);
int queue_remove_value(Queue *queue, int item);

#endif
