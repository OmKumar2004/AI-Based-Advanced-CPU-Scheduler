#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_PROCESSES 10
#define MAX_STATES 5
#define TIME_QUANTUM 2
#define LEARNING_RATE 0.2
#define DISCOUNT_FACTOR 0.9
#define EPSILON 0.1 // Exploration factor

// Process structure
typedef struct {
    int id;
    int burst_time;
    int waiting_time;
    int system_priority;
    float cpu_utilization;
    float memory_usage;
    bool completed;
} Process;

// Global variables
Process processes[MAX_PROCESSES];
float Q_table[MAX_PROCESSES][MAX_STATES];
int num_processes;

// Function prototypes
void initialize_processes();
void initialize_q_table();
int select_action();
void update_q_table();
float calculate_reward(Process p);
void update_state(int process_id);
void remove_completed_process(int process_id);
void display_processes();
void display_q_table();

int main() {
    srand(time(NULL));
    
    initialize_processes();
    initialize_q_table();

    printf("Initial Processes:\n");
    display_processes();

    // Calculate reward and update Q-table
    update_q_table();
    // Display initial Q-table
    display_q_table();

    while (1) {
        // Check if all processes are completed
        bool all_completed = true;
        for (int i = 0; i < num_processes; i++) {
            if (!processes[i].completed) {
                all_completed = false;
                break;
            }
        }
        if (all_completed) break;

        // Select the process to schedule based on Q-value
        int selected_process = select_action();
        Process *p = &processes[selected_process];

        // Execute the process for the time quantum
        printf("\nExecuting Process ID: %d\n", p->id);
        p->burst_time -= TIME_QUANTUM;
        if (p->burst_time <= 0) {
            p->burst_time = 0;
            p->completed = true;
        }

        // Update waiting times of other processes
        for (int i = 0; i < num_processes; i++) {
            if (i != selected_process && !processes[i].completed) {
                processes[i].waiting_time += TIME_QUANTUM;
            }
        }

        // Update state variables (random values for dynamic simulation)
        update_state(selected_process);

        // Remove completed process from Q-table
        if (p->completed) {
            remove_completed_process(selected_process);
        }

        // Display updated process table and Q-table
        display_processes();
        display_q_table();
    }

    printf("\nAll processes completed!\n");
    return 0;
}

void initialize_processes() {
    printf("Enter the number of processes (max %d): ", MAX_PROCESSES);
    scanf("%d", &num_processes);
    for (int i = 0; i < num_processes; i++) {
        processes[i].id = i;
        processes[i].burst_time = rand() % 20 + 1; // Random burst time between 1 and 20
        processes[i].waiting_time = 0;
        processes[i].system_priority = rand() % 10 + 1; // Random priority between 1 and 10
        processes[i].cpu_utilization = (float)(rand() % 100) / 100.0; // Random CPU utilization (0-1)
        processes[i].memory_usage = (float)(rand() % 100) / 100.0; // Random memory usage (0-1)
        processes[i].completed = false;
    }
}

void initialize_q_table() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        for (int j = 0; j < MAX_STATES; j++) {
            Q_table[i][j] = 0.0;
        }
    }
}

int select_action() {
    float max_q_value = -1e9;
    int best_process = -1;

    for (int i = 0; i < num_processes; i++) {
        if (!processes[i].completed) {
            float q_value = 0;
            for (int j = 0; j < MAX_STATES; j++) {
                q_value += Q_table[i][j];
            }
            if (q_value > max_q_value) {
                max_q_value = q_value;
                best_process = i;
            }
        }
    }

    return best_process;
}

void update_q_table() {
    for (int i = 0; i < num_processes; i++) { // Loop through all processes
        if (processes[i].completed) continue; // Skip completed processes

        // Calculate reward for the current process
        float reward = calculate_reward(processes[i]);

        for (int j = 0; j < MAX_STATES; j++) {
            float max_next_q = 0;
            for (int k = 0; k < MAX_STATES; k++) {
                if (Q_table[i][k] > max_next_q) {
                    max_next_q = Q_table[i][k];
                }
            }
            Q_table[i][j] = Q_table[i][j] + LEARNING_RATE * (reward + DISCOUNT_FACTOR * max_next_q - Q_table[i][j]);
        }
    }
}

float calculate_reward(Process p) {
    // Optimized weights
    float w1 = 0.35, w2 = 0.2, w3 = 0.2, w4 = 0.15, w5 = 0.05;

    // Calculate reward
    float reward = (w1 * (float)p.system_priority) +   // Higher priority number, higher reward
                   (w2 * -(float)p.burst_time) +       // Lower burst time, higher reward
                   (w3 * (float)p.waiting_time) +      // Higher waiting time, higher reward
                   (w4 * -(float)p.cpu_utilization) +  // Lower CPU utilization, higher reward
                   (w5 * (float)p.memory_usage);       // Higher memory usage, higher reward

    return reward;
}

void update_state(int process_id) {
    processes[process_id].system_priority = rand() % 10 + 1;
    processes[process_id].cpu_utilization = (float)(rand() % 100) / 100.0;
    processes[process_id].memory_usage = (float)(rand() % 100) / 100.0;
}

void remove_completed_process(int process_id) {
    for (int i = 0; i < MAX_STATES; i++) {
        Q_table[process_id][i] = 0.0;
    }
    printf("Process ID %d completed and removed from Q-table.\n", process_id);
}

void display_processes() {
    printf("\nCurrent Process States:\n");
    printf("ID\tBurst\tWait\tPriority\tCPU%%\tMemory%%\tCompleted\n");
    for (int i = 0; i < num_processes; i++) {
        printf("%d\t%d\t%d\t%d\t\t%.2f\t%.2f\t%s\n",
               processes[i].id, processes[i].burst_time, processes[i].waiting_time,
               processes[i].system_priority, processes[i].cpu_utilization,
               processes[i].memory_usage, processes[i].completed ? "Yes" : "No");
    }
}

void display_q_table() {
    printf("\nQ-Table:\n");
    for (int i = 0; i < num_processes; i++) {
        printf("Process %d: ", i);
        for (int j = 0; j < MAX_STATES; j++) {
            printf("%.2f ", Q_table[i][j]);
        }
        printf("\n");
    }
}
