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
