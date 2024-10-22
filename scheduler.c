#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include "scheduler.h"
#include "utilities.h"



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

Scheduler* create_scheduler(SchedulingAlgorithm algorithm, int num_processes) {
    Scheduler* scheduler = malloc(sizeof(Scheduler));
    scheduler->all_processes = malloc(num_processes * sizeof(Process*)); 
    scheduler->algorithm = algorithm;
    scheduler->ready_queue = create_queue();
    scheduler->io_queue = create_queue();
    scheduler->current_process = NULL;
    scheduler->current_time = 0;
    scheduler->total_processes = 0;
    scheduler->completed_processes = 0;

    // Initialize priority queues for MLFQ
    scheduler->boost_timer = 0;
    if (algorithm == MULTI_LEVEL_FEEDBACK) {
        for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
            scheduler->priority_queues[i] = create_queue();
        }
    }

    // Initialize statistics
    scheduler->total_turnaround_time = 0;
    scheduler->total_waiting_time = 0;
    scheduler->total_response_time = 0;
    scheduler->total_io_time = 0;
    scheduler->longest_job_time = 0;
    scheduler->shortest_job_time = INT_MAX;

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
    
    free(scheduler->all_processes);
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

        update_process_stats(scheduler->current_process, scheduler->current_time);
    }
}

void handle_process_completion(Scheduler* scheduler) {
    Process* completed_process = scheduler->current_process;
    update_process_stats(completed_process, scheduler->current_time);
    update_scheduler_stats(scheduler, completed_process);
    scheduler->completed_processes++;
    scheduler->current_process = NULL;
    // Update statistics here
}

int handle_io_completion(Scheduler* scheduler) {
    if (is_empty(scheduler->io_queue)) {
        return 0;
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

    return completed_count;
}

void add_new_process(Scheduler* scheduler, Process* process) {
    scheduler->all_processes[scheduler->total_processes] = process;
    scheduler->total_processes++;    
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

    // Find the shortest job in the ready queue
    // If the remaining time is the same, then select the process with the smaller PID
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

    return next_process;
}

Process* select_next_process_mlfq(Scheduler* scheduler) {
    // Rule 5: Check if it's time to boost all processes
    // Step 1: Move all processes to the temp process array
    // Step 2: Sort the temp process array by PID
    // Step 3: Enqueue the sorted processes back to the highest priority queue
    if (scheduler->boost_timer >= MLFQ_BOOST_TIME) {
        int temp_process_count = 0;

        for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
            temp_process_count += queue_size(scheduler->priority_queues[i]);
        }

        if (temp_process_count > 0) {
            Process** temp_processes = malloc(temp_process_count * sizeof(Process*));
            int temp_index = 0;
            for (int i = 0; i < NUM_PRIORITY_LEVELS; i++) {
                while (!is_empty(scheduler->priority_queues[i])) {
                    Process* process = dequeue(scheduler->priority_queues[i]);
                    process->priority_level = 0;
                    process->allotment_time_used = 0;
                    temp_processes[temp_index++] = process;
                }
            }

            quicksort(temp_processes, 0, temp_index - 1);
            for (int i = 0; i < temp_index; i++) {
                enqueue(scheduler->priority_queues[0], temp_processes[i]);
            }

            free(temp_processes);
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
    // Create output directory if it doesn't exist
    struct stat st = {0};
    if (stat("output", &st) == -1) {
        if (mkdir("output", 0700) == -1) {
            perror("Failed to create output directory");
            return;
        }
    }

    // Open the output file
    FILE* file = fopen("output/statistics_output.txt", "w");
    if (file == NULL) {
        perror("Failed to open output file");
        return;
    }

    // Write to file instead of printing to console
    fprintf(file, "|        | Total time      | Total time     | Total time |\n");
    fprintf(file, "|  Job#  | in ready to run | in sleeping on | in system  |\n");
    fprintf(file, "|        | state           | I/O state      |            |\n");
    fprintf(file, "|--------|-----------------|----------------|------------|\n");

    for (int i = 0; i < scheduler->total_processes; i++) {
        Process* p = scheduler->all_processes[i];
        fprintf(file, "| pid%-2d | %-15d | %-14d | %-10d |\n",
               p->pid,
               p->ready_time,
               p->io_time,
               p->turnaround_time
               );
    }

    fprintf(file, "|--------|-----------------|----------------|------------|\n");
    fprintf(file, "Total simulation run time: %d\n", scheduler->current_time);
    fprintf(file, "Total number of jobs: %d\n", scheduler->total_processes);
    fprintf(file, "Shortest job completion time: %d\n", scheduler->shortest_job_time);
    fprintf(file, "Longest job completion time: %d\n", scheduler->longest_job_time);
    fprintf(file, "Average job completion time: %.2f\n", 
           (float)scheduler->total_turnaround_time / scheduler->total_processes);
    fprintf(file, "Average job response time: %.2f\n", 
           (float)scheduler->total_response_time / scheduler->total_processes);           
    fprintf(file, "Average time in ready queue: %.2f\n", 
           (float)scheduler->total_waiting_time / scheduler->total_processes);
    fprintf(file, "Average time sleeping on I/O: %.2f\n", 
           (float)scheduler->total_io_time / scheduler->total_processes);

    // Close the file
    fclose(file);

    printf("Statistics have been written to output/statistics.txt\n");
}

void update_scheduler_stats(Scheduler* scheduler, Process* completed_process) {
    scheduler->total_turnaround_time += completed_process->turnaround_time;
    scheduler->total_waiting_time += completed_process->waiting_time;
    scheduler->total_response_time += completed_process->response_time;
    scheduler->total_io_time += completed_process->io_time;
    
    if (completed_process->turnaround_time > scheduler->longest_job_time) {
        scheduler->longest_job_time = completed_process->turnaround_time;
    }
    
    if (completed_process->turnaround_time < scheduler->shortest_job_time) {
        scheduler->shortest_job_time = completed_process->turnaround_time;
    }
}
