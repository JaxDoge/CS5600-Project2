#include "input_parser.h"
#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 100

Process** parse_input(const char* filename, int* num_processes) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    Process** processes = NULL;
    int capacity = 10;
    *num_processes = 0;
    processes = (Process**) malloc(capacity * sizeof(Process*));

    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        int pid, arrival_time, service_time, priority;
        if (sscanf(line, "%d:%d:%d:%d", &pid, &arrival_time, &service_time, &priority) == 4) {
            if (*num_processes >= capacity) {
                capacity *= 2;
                processes = (Process**) realloc(processes, capacity * sizeof(Process*));
            }
            processes[*num_processes] = create_process(pid, arrival_time, service_time, priority);
            (*num_processes)++;
        }
    }

    fclose(file);

    // Sort the processes by PID using quicksort. This is done to ensure that
    // the processes are in order of their PIDs. This is important for the RR
    // and MLFQ scheduling algorithms to work correctly.
    if (*num_processes > 0) {
        quicksort(processes, 0, *num_processes - 1);
    }

    return processes;
}
