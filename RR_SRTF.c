#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

struct Process {
    int pid;
    int burst_time;
    int arrival_time;
    int waiting_time;
};

void initialize_processes(struct Process processes[], int n) {
    for (int i = 0; i < n; i++) {
        printf("\nEnter arrival time for Process P%d: ", i);
        scanf("%d", &processes[i].arrival_time);

        printf("Enter burst time for Process P%d: ", i);
        scanf("%d", &processes[i].burst_time);

        pid_t pid = fork();
        if (pid == 0) {
            processes[i].pid = getpid();
            printf("Process P%d created with PID: %d\n", i, processes[i].pid);
            exit(0);
        } else {
            wait(NULL);
            processes[i].pid = pid;
        }
    }
}

void round_robin(struct Process processes[], int n, int quantum) {
    int remaining_burst[n];
    int time = 0, done;

    for (int i = 0; i < n; i++) {
        remaining_burst[i] = processes[i].burst_time;
        processes[i].waiting_time = 0;
    }

    while (1) {
        done = 1;

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= time && remaining_burst[i] > 0) {
                done = 0;

                if (remaining_burst[i] > quantum) {
                    time += quantum;
                    remaining_burst[i] -= quantum;
                } else {
                    time += remaining_burst[i];
                    processes[i].waiting_time = time - processes[i].burst_time - processes[i].arrival_time;
                    remaining_burst[i] = 0;
                }
            }
        }

        if (done == 1)
            break;

        int no_ready_process = 1;
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= time && remaining_burst[i] > 0) {
                no_ready_process = 0;
                break;
            }
        }

        if (no_ready_process)
            time++;
    }
}

void srtf_scheduling(struct Process processes[], int n) {
    int remaining_time[n];
    int time = 0, completed = 0, shortest = -1, min_remaining = INT_MAX;
    int is_completed[n];

    for (int i = 0; i < n; i++) {
        remaining_time[i] = processes[i].burst_time;
        processes[i].waiting_time = 0;
        is_completed[i] = 0;
    }

    while (completed != n) {
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= time && !is_completed[i] && remaining_time[i] < min_remaining) {
                min_remaining = remaining_time[i];
                shortest = i;
            }
        }

        if (shortest == -1) {
            time++;
            continue;
        }

        remaining_time[shortest]--;
        if (remaining_time[shortest] == 0) {
            completed++;
            is_completed[shortest] = 1;
            min_remaining = INT_MAX;

            processes[shortest].waiting_time = time + 1 - processes[shortest].burst_time - processes[shortest].arrival_time;
        }

        time++;
    }
}

float calculate_average_waiting_time(struct Process processes[], int n) {
    float total_waiting_time = 0;
    for (int i = 0; i < n; i++) {
        total_waiting_time += processes[i].waiting_time;
    }
    return total_waiting_time / n;
}

int main() {
    int n, quantum;

    printf("Enter the number of processes: ");
    scanf("%d", &n);

    struct Process processes[n];

    initialize_processes(processes, n);

    printf("\nEnter quantum time for Round-Robin scheduling: ");
    scanf("%d", &quantum);

    round_robin(processes, n, quantum);
    float avg_waiting_rr = calculate_average_waiting_time(processes, n);
    printf("\nRound-Robin Average Waiting Time: %.2f\n", avg_waiting_rr);

    srtf_scheduling(processes, n);
    float avg_waiting_srtf = calculate_average_waiting_time(processes, n);
    printf("\nSRTF Scheduling Average Waiting Time: %.2f\n", avg_waiting_srtf);

    return 0;
}

