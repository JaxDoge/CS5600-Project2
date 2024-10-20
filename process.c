#include <stdlib.h>
#include "process.h"

Process* create_process(int pid, int arrival_time, int service_time, int priority) {
    Process* p = (Process*)malloc(sizeof(Process));
    p->pid = pid;
    p->arrival_time = arrival_time;
    p->service_time = service_time;
    p->priority = priority;
    p->remaining_time = service_time;
    p->start_time = -1;
    p->completion_time = -1;
    p->turnaround_time = 0;
    p->waiting_time = 0;
    p->response_time = 0;    
    p->ready_time = 0;
    p->running_time = 0;
    p->io_time = 0;
    p->priority_level = 0;
    p->allotment_time_used = 0;
    return p;
}

void destroy_process(Process* p) {
    free(p);
}

void update_process_stats(Process* p, int current_time) {
    if (p->start_time == -1) {
        p->start_time = current_time;
        p->response_time = p->start_time - p->arrival_time;
    }
    
    if (p->remaining_time == 0 && p->completion_time == -1) {
        p->completion_time = current_time;
        p->turnaround_time = p->completion_time - p->arrival_time;
        p->waiting_time = p->turnaround_time - p->service_time;
    }
}
