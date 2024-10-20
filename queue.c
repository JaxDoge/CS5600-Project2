#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

queue_t* create_queue() {
    queue_t *queue = malloc(sizeof(queue_t));
    queue->front = NULL; 
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}

void enqueue(queue_t *queue, void *element) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->data = element;
    new_node->next = NULL;

    if (queue->rear == NULL) {
        queue->front = queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
    
    queue->size++;
}

void* dequeue(queue_t *queue) {
    if (queue->front == NULL)
        return NULL;

    node_t *temp = queue->front;
    void *data = temp->data;

    queue->front = queue->front->next;

    if (queue->front == NULL)
        queue->rear = NULL;

    free(temp);
    queue->size--;
    return data;
}

void* peek(queue_t *queue) {
    if (queue->front == NULL)
        return NULL;
    return queue->front->data;
}

bool is_empty(queue_t *queue) {
    return queue->size == 0;
}

int queue_size(queue_t *queue) {
    return queue->size;
}

void destroy_queue(queue_t *queue) {
    while (!is_empty(queue)) {
        dequeue(queue);
    }
    free(queue);
}

void print_queue(queue_t *queue, void (*print_function)(void*)) {
    node_t *current = queue->front;
    while (current != NULL) {
        print_function(current->data);
        current = current->next;
    }
    printf("\n");
}
