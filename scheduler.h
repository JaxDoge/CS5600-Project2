#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "queue.h"

#define TIME_SLICE 4  // For Round Robin
#define NUM_PRIORITY_LEVELS 3  // For Multi-level Feedback Queue
#define MLFQ_BOOST_TIME 100  // Time period S for Rule 5

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
    int current_time;
    int total_processes;
    int completed_processes;
    
    // For Multi-level Feedback Queue
    queue_t* priority_queues[NUM_PRIORITY_LEVELS];
    int boost_timer;  // Counter for MLFQ boost
} Scheduler;

Scheduler* create_scheduler(SchedulingAlgorithm algorithm);
void destroy_scheduler(Scheduler* scheduler);
void schedule_process(Scheduler* scheduler);
void handle_process_completion(Scheduler* scheduler);
void handle_io_completion(Scheduler* scheduler);
void add_new_process(Scheduler* scheduler, Process* process);
void print_statistics(Scheduler* scheduler);

// Scheduling algorithm specific functions
Process* select_next_process_sjf(Scheduler* scheduler);
Process* select_next_process_rr(Scheduler* scheduler);
Process* select_next_process_mlfq(Scheduler* scheduler);

// Function to seed the random number generator
void os_srand(unsigned int seed);

#endif
