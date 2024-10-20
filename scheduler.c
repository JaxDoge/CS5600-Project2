#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "utilities.h"

#define CHANCE_OF_IO_REQUEST 10
#define CHANCE_OF_IO_COMPLETE 4

int os_rand() {
    return rand();
}

void os_srand(unsigned int seed) {
    srand(seed);
}

int IO_request() {
    return (os_rand() % CHANCE_OF_IO_REQUEST == 0);
}

int IO_complete() {
    return (os_rand() % CHANCE_OF_IO_COMPLETE == 0);
}

Scheduler* create_scheduler(SchedulingAlgorithm algorithm) {
    Scheduler* scheduler = malloc(sizeof(Scheduler));
    scheduler->algorithm = algorithm;
    scheduler->ready_queue = create_queue();
    scheduler->io_queue = create_queue();
    scheduler->current_process = NULL;
    scheduler->current_time = 0;
    scheduler->total_processes = 0;
    scheduler->completed_processes = 0;

    if (algorithm == MULTI_LEVEL_FEEDBACK) {
        for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
            scheduler->priority_queues[i] = create_queue();
        }
    }

    return scheduler;
}

void destroy_scheduler(Scheduler* scheduler) {
    destroy_queue(scheduler->ready_queue);
    destroy_queue(scheduler->io_queue);
    
    if (scheduler->algorithm == MULTI_LEVEL_FEEDBACK) {
        for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
            destroy_queue(scheduler->priority_queues[i]);
        }
    }
    
    free(scheduler);
}

void schedule_process(Scheduler* scheduler) {
    Process* next_process = NULL;

    switch (scheduler->algorithm) {
        case PREEMPTIVE_SJF:
            next_process = select_next_process_sjf(scheduler);
            break;
        case ROUND_ROBIN:
            next_process = select_next_process_rr(scheduler);
            break;
        case MULTI_LEVEL_FEEDBACK:
            next_process = select_next_process_mlfq(scheduler);
            break;
    }

    if (next_process != NULL) {
        if (scheduler->current_process != NULL && scheduler->algorithm == PREEMPTIVE_SJF) {
            // If the new process has a shorter remaining time, preempt the current process
            if (next_process->remaining_time < scheduler->current_process->remaining_time) {
                enqueue(scheduler->ready_queue, scheduler->current_process);
                scheduler->current_process = next_process;
            } else {
                enqueue(scheduler->ready_queue, next_process);
            }
        } else {
            scheduler->current_process = next_process;
        }

        if (scheduler->current_process->start_time == -1) {
            scheduler->current_process->start_time = scheduler->current_time;
        }
    }
}

void handle_process_completion(Scheduler* scheduler) {
    Process* completed_process = scheduler->current_process;
    completed_process->completion_time = scheduler->current_time;
    scheduler->completed_processes++;
    scheduler->current_process = NULL;
    // Update statistics here
}

void handle_io_completion(Scheduler* scheduler) {
    if (is_empty(scheduler->io_queue)) {
        return;
    }

    queue_t* temp_queue = create_queue();
    queue_t* completed_io = create_queue();

    // Traverse the IO queue
    while (!is_empty(scheduler->io_queue)) {
        Process* io_process = dequeue(scheduler->io_queue);
        if (IO_complete()) {
            // If IO is complete, add to completed_io queue
            enqueue(completed_io, io_process);
        } else {
            // If IO is not complete, add back to temp queue
            enqueue(temp_queue, io_process);
        }
    }

    // Sort completed IO processes by PID
    int completed_count = queue_size(completed_io);
    Process** completed_array = malloc(completed_count * sizeof(Process*));
    for (int i = 0; i < completed_count; i++) {
        completed_array[i] = dequeue(completed_io);
    }

    // Sort the completed processes by PID
    quicksort(completed_array, 0, completed_count - 1);

    // Enqueue sorted completed processes to ready queue
    for (int i = 0; i < completed_count; i++) {
        if (scheduler->algorithm == MULTI_LEVEL_FEEDBACK) {
            enqueue(scheduler->priority_queues[completed_array[i]->priority_level], completed_array[i]);
        } else {
            enqueue(scheduler->ready_queue, completed_array[i]);
        }
    }

    free(completed_array);

    // Move remaining processes back to IO queue
    while (!is_empty(temp_queue)) {
        enqueue(scheduler->io_queue, dequeue(temp_queue));
    }

    destroy_queue(temp_queue);
    destroy_queue(completed_io);
}

void add_new_process(Scheduler* scheduler, Process* process) {
    if (scheduler->algorithm == MULTI_LEVEL_FEEDBACK) {
        // New processes always start in the highest priority queue
        enqueue(scheduler->priority_queues[0], process);
    } else {
        enqueue(scheduler->ready_queue, process);
    }
}

// Implement the scheduling algorithm specific functions here
Process* select_next_process_sjf(Scheduler* scheduler) {
    if (is_empty(scheduler->ready_queue)) {
        return NULL;
    }

    node_t* current = scheduler->ready_queue->front;
    node_t* prev = NULL;
    node_t* shortest_node = current;
    node_t* shortest_prev = NULL;

    while (current != NULL) {
        Process* process = (Process*)current->data;
        Process* shortest_process = (Process*)shortest_node->data;

        if (process->remaining_time < shortest_process->remaining_time ||
            (process->remaining_time == shortest_process->remaining_time && process->pid < shortest_process->pid)) {
            shortest_node = current;
            shortest_prev = prev;
        }

        prev = current;
        current = current->next;
    }

    // Remove the shortest job from the queue
    Process* shortest_job = (Process*)shortest_node->data;

    if (shortest_prev == NULL) {
        scheduler->ready_queue->front = shortest_node->next;
    } else {
        shortest_prev->next = shortest_node->next;
    }

    if (shortest_node == scheduler->ready_queue->rear) {
        scheduler->ready_queue->rear = shortest_prev;
    }

    free(shortest_node);
    scheduler->ready_queue->size--;

    return shortest_job;
}

Process* select_next_process_rr(Scheduler* scheduler) {
    if (is_empty(scheduler->ready_queue)) {
        return NULL;
    }

    Process* next_process = dequeue(scheduler->ready_queue);
    
    // If the process has remaining time, it will be added back to the queue after its time slice
    if (next_process->remaining_time > TIME_SLICE) {
        next_process->remaining_time -= TIME_SLICE;
        enqueue(scheduler->ready_queue, next_process);
    }

    return next_process;
}

Process* select_next_process_mlfq(Scheduler* scheduler) {
    // Rule 5: Check if it's time to boost all processes
    if (scheduler->boost_timer >= MLFQ_BOOST_TIME) {
        for (int i = 1; i < NUM_PRIORITY_LEVELS; i++) {
            while (!is_empty(scheduler->priority_queues[i])) {
                Process* process = dequeue(scheduler->priority_queues[i]);
                enqueue(scheduler->priority_queues[0], process);
            }
        }
        scheduler->boost_timer = 0;
    }

    // Rule 1 and 2: Select the highest priority non-empty queue
    for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
        if (!is_empty(scheduler->priority_queues[i])) {
            Process* next_process = dequeue(scheduler->priority_queues[i]);
            return next_process;
        }
    }
    return NULL;
}

void print_statistics(Scheduler* scheduler) {
    // Implementation to print statistics as per the required format
    // ...
}

