***Team member***: Jiaxing Tan, Yulong Cao

# Project Overview

# Usage Instructions

## Compile

To compile the project, run:

```bash
make all
```

The compiled executable will be in the project root directory.

To clean up the project, run:

```bash
make clean
```

## Run
```bash
./coordinator <input-file> <scheduling-algorithm>
```

Note that the `<input-file>` param must be provided when executing the program. 

The `<scheduling-algorithm>` param is optional and can be one of the following:

- `1` = Preemptive Shortest Job First (SJF)
- `2` = Round Robin (RR)
- `3` = Multi-level Feedback Queue (MLFQ)

The input file will be following the format specified in the project description, where each line corresponds to a process with the following format:

```
<PID>:<Arrival Time>:<CPU Burst Time>:<Priority>
```

Outputs will be written to `./output/statisics_output.txt`.

# Validation

An example of the input file and the expected output of each scheduling algorithm can be found in `./demo/`.

The demo input is specified as follows:

```
100:0:100:2
200:25:125:1
300:100:225:3
400:125:200:2
500:175:175:1
600:200:150:4
```

The numbers are scaled up because the default MLFQ boost time is 100.

Because the random IO events are introduced in the simulation, the manual verification of the output is not feasible.

## Preemptive Shortest Job First (SJF)

```
|        | Total time      | Total time     | Total time |
|  Job#  | in ready to run | in sleeping on | in system  |
|        | state           | I/O state      |            |
|--------|-----------------|----------------|------------|
| pid100 | 0               | 9              | 109        |
| pid200 | 79              | 28             | 232        |
| pid300 | 648             | 48             | 921        |
| pid400 | 424             | 65             | 689        |
| pid500 | 215             | 84             | 474        |
| pid600 | 50              | 74             | 274        |
|--------|-----------------|----------------|------------|
Total simulation run time: 1021
Total number of jobs: 6
Shortest job completion time: 109
Longest job completion time: 921
Average job completion time: 449.83
Average job response time: 8.33
Average time in ready queue: 287.33
Average time sleeping on I/O: 51.33
```

The running result matched expectation because we can see the characteristics of SJF scheduling algorithm. Process `pid300` is stalled because it has the longest CPU burst time, and the other processes are executed in the order of their CPU burst time.

Preemptive SJF demonstrated the lowest average job completion time among the three scheduling algorithms, which is expected because it always selects the shortest job to run first.


## Round Robin (RR)

```
|        | Total time      | Total time     | Total time |
|  Job#  | in ready to run | in sleeping on | in system  |
|        | state           | I/O state      |            |
|--------|-----------------|----------------|------------|
| pid100 | 21              | 5              | 126        |
| pid200 | 307             | 57             | 489        |
| pid300 | 587             | 75             | 887        |
| pid400 | 512             | 43             | 755        |
| pid500 | 510             | 60             | 745        |
| pid600 | 538             | 69             | 757        |
|--------|-----------------|----------------|------------|
Total simulation run time: 987
Total number of jobs: 6
Shortest job completion time: 126
Longest job completion time: 887
Average job completion time: 626.50
Average job response time: 18.33
Average time in ready queue: 464.00
Average time sleeping on I/O: 51.50
```

The running result matched expectation because we can see the characteristics of RR scheduling algorithm. After the process arrived (after `pid300`), the processed exhibit a similar ready time and similar finish time.

## Multi-level Feedback Queue (MLFQ)

```
|        | Total time      | Total time     | Total time |
|  Job#  | in ready to run | in sleeping on | in system  |
|        | state           | I/O state      |            |
|--------|-----------------|----------------|------------|
| pid100 | 231             | 19             | 350        |
| pid200 | 312             | 33             | 470        |
| pid300 | 559             | 118            | 902        |
| pid400 | 548             | 48             | 796        |
| pid500 | 500             | 30             | 705        |
| pid600 | 473             | 66             | 689        |
|--------|-----------------|----------------|------------|
Total simulation run time: 1002
Total number of jobs: 6
Shortest job completion time: 350
Longest job completion time: 902
Average job completion time: 652.00
Average job response time: 4.33
Average time in ready queue: 489.50
Average time sleeping on I/O: 52.33
```

# Work Distribution

- Jiaxing Tan: Primary Coding
- Yulong Cao: Testing and Documentation
