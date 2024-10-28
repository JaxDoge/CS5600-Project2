#ifndef PROCESS_H
#define PROCESS_H

typedef struct {
    int pid;
    int arrival_time;
    int service_time;
    int priority;
    int remaining_time;
    int start_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int ready_time;
    int running_time;
    int io_time;
    int priority_level;      // Added priority level for MLFQ
    int allotment_time_used; // Added allotment usage for MLFQ
} Process;

// Function prototypes
Process* create_process(int pid, int arrival_time, int service_time, int priority);
void destroy_process(Process* p);
// Update process stats whenever a process is first scheduled or completed
void update_process_stats(Process* p, int current_time);

#endif
