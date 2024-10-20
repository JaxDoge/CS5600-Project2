#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct node {
    void *data;
    struct node *next;
} node_t;

typedef struct {
    node_t *front;
    node_t *rear;
    int size;
} queue_t;

queue_t* create_queue();
void enqueue(queue_t *queue, void *element);
void* dequeue(queue_t *queue);
void* peek(queue_t *queue);
bool is_empty(queue_t *queue);
int queue_size(queue_t *queue);
void destroy_queue(queue_t *queue);

// Optional: Add this if you want to print the queue for debugging
void print_queue(queue_t *queue, void (*print_function)(void*));

#endif
