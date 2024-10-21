#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "input_parser.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <scheduling_algorithm>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    int algorithm_choice = atoi(argv[2]);

    SchedulingAlgorithm algorithm;
    switch (algorithm_choice) {
        case 1: algorithm = PREEMPTIVE_SJF; break;
        case 2: algorithm = ROUND_ROBIN; break;
        case 3: algorithm = MULTI_LEVEL_FEEDBACK; break;
        default:
            fprintf(stderr, "Invalid scheduling algorithm choice\n");
            return 1;
    }

    int num_processes;
    Process** processes = parse_input(input_file, &num_processes);
    if (processes == NULL) {
        fprintf(stderr, "Failed to parse input file\n");
        return 1;
    }

    Scheduler* scheduler = create_scheduler(algorithm, num_processes);
    os_srand(1);  // Seed the random number generator

    int time_slice_remaining = TIME_SLICE;

    // Main simulation loop
    while (scheduler->completed_processes < num_processes) {
        // Add new arriving processes
        int new_processes_added = 0;
        for (int i = 0; i < num_processes; i++) {
            if (processes[i]->arrival_time == scheduler->current_time) {
                add_new_process(scheduler, processes[i]);
                new_processes_added++;
            }
        }

        // Update I/O time for processes in I/O queue
        node_t* current_node = scheduler->io_queue->front;
        while (current_node != NULL) {
            Process* p = (Process*)current_node->data;
            p->io_time++;
            current_node = current_node->next;
        }     

        // Handle I/O completions
        int completed_io_count = handle_io_completion(scheduler);

        // Preempt the current process situations
        if (new_processes_added + completed_io_count > 0 && scheduler->current_process != NULL) { 
            // If at least one new process has added to the ready queue, preempt the current process in PREEMPTIVE_SJF
            if (scheduler->algorithm == PREEMPTIVE_SJF) {
                enqueue(scheduler->ready_queue, scheduler->current_process);
                scheduler->current_process = NULL;
            }
            // If at least one process has completed I/O, preempt the current process in MLFQ 
            // if there is one process in a higher priority queue
            if (scheduler->algorithm == MULTI_LEVEL_FEEDBACK) {
                int higher_priority_queue_size = 0;
                for (int i = scheduler->current_process->priority_level - 1; i >= 0; i--) {
                    higher_priority_queue_size += scheduler->priority_queues[i]->size;
                }
                if (higher_priority_queue_size > 0) {
                    enqueue(scheduler->ready_queue, scheduler->current_process);
                    scheduler->current_process = NULL;
                }
            }
        }

        // Schedule next process
        if (scheduler->current_process == NULL) {
            schedule_process(scheduler);
            time_slice_remaining = TIME_SLICE;
        }

        // Update ready time for processes in ready queue
        current_node = scheduler->ready_queue->front;
        while (current_node != NULL) {
            Process* p = (Process*)current_node->data;
            p->ready_time++;
            current_node = current_node->next;
        }

        // Run current process
        if (scheduler->current_process != NULL) {
            Process* current = scheduler->current_process;

            // Simulate the passage of time
            scheduler->current_time++;            
            current->remaining_time--;
            current->running_time++;
            time_slice_remaining--;
            current->allotment_time_used++;

            // Check if the process has completed
            if (current->remaining_time == 0) {
                handle_process_completion(scheduler);

            // Check if the process has an I/O request
            } else if (IO_request()) {
                enqueue(scheduler->io_queue, current);
                scheduler->current_process = NULL;

            // Check if the time slice has expired
            } else if (time_slice_remaining == 0 && scheduler->algorithm != PREEMPTIVE_SJF) {
                if (scheduler->algorithm == ROUND_ROBIN) {
                    enqueue(scheduler->ready_queue, current);
                } else if (scheduler->algorithm == MULTI_LEVEL_FEEDBACK) {
                    int next_priority = (current->priority_level + 1 < NUM_PRIORITY_LEVELS) ? current->priority_level + 1 : NUM_PRIORITY_LEVELS - 1;
                    current->priority_level = next_priority;
                    enqueue(scheduler->priority_queues[next_priority], current);
                }
                scheduler->current_process = NULL;
            
            // Rule 4: Check if the process has used its allotment time in MLFQ, even it is already in the lowest priority queue
            } else if (scheduler->algorithm == MULTI_LEVEL_FEEDBACK && current->allotment_time_used >= TIME_SLICE) {
                int next_priority = (current->priority_level + 1 < NUM_PRIORITY_LEVELS) ? current->priority_level + 1 : NUM_PRIORITY_LEVELS - 1;
                current->priority_level = next_priority;
                enqueue(scheduler->priority_queues[next_priority], current);
                current->allotment_time_used = 0;
                scheduler->current_process = NULL;
            } 
        } else {
            // If no process is currently running, simulate the passage of time
            scheduler->current_time++;
        }

        if (scheduler->algorithm == MULTI_LEVEL_FEEDBACK) {
            scheduler->boost_timer++;
            if (scheduler->boost_timer >= MLFQ_BOOST_TIME) {
                scheduler->current_process = NULL;
            }
        }
    }

    // Print final statistics
    print_statistics(scheduler);

    // Clean up
    destroy_scheduler(scheduler);
    for (int i = 0; i < num_processes; i++) {
        destroy_process(processes[i]);
    }
    free(processes);

    return 0;
}
