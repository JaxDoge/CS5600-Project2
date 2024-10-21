#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "queue.h"

#define TIME_SLICE 4  // For Round Robin
#define NUM_PRIORITY_LEVELS 3  // For Multi-level Feedback Queue
#define MLFQ_BOOST_TIME 100  // Time period S for Rule 5
#define CHANCE_OF_IO_REQUEST 10 // Chance of I/O request
#define CHANCE_OF_IO_COMPLETE 4 // Chance of I/O completion

typedef enum {
    PREEMPTIVE_SJF,
    ROUND_ROBIN,
    MULTI_LEVEL_FEEDBACK
} SchedulingAlgorithm;

typedef struct {
    SchedulingAlgorithm algorithm;
    queue_t* ready_queue;
    queue_t* io_queue;
    Process* current_process;
    Process** all_processes;  // Array to store all processes
    int current_time;
    int total_processes;
    int completed_processes;

    // System-wide statistics
    int total_turnaround_time;
    int total_waiting_time;
    int total_response_time;
    int total_io_time;
    int longest_job_time;
    int shortest_job_time;
    
    // For Multi-level Feedback Queue
    queue_t* priority_queues[NUM_PRIORITY_LEVELS];
    int boost_timer;  // Counter for MLFQ boost
} Scheduler;

int IO_request();

Scheduler* create_scheduler(SchedulingAlgorithm algorithm, int num_processes);
void destroy_scheduler(Scheduler* scheduler);
void schedule_process(Scheduler* scheduler);
void handle_process_completion(Scheduler* scheduler);
int handle_io_completion(Scheduler* scheduler);
void add_new_process(Scheduler* scheduler, Process* process);
void print_statistics(Scheduler* scheduler);

// Scheduling algorithm specific functions
Process* select_next_process_sjf(Scheduler* scheduler);
Process* select_next_process_rr(Scheduler* scheduler);
Process* select_next_process_mlfq(Scheduler* scheduler);

// Function to seed the random number generator
void os_srand(unsigned int seed);

void update_scheduler_stats(Scheduler* scheduler, Process* completed_process);

#endif
