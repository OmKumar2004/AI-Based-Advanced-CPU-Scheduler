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
void save_to_file(int iteration, int process_id,int last);

int main() {
    srand(time(NULL));
    
    initialize_processes();
    initialize_q_table();

    printf("Initial Processes:\n");
    display_processes();

    int iteration = 0;

    while (1) {
        // Checking if all processes are completed
        bool all_completed = true;
        for (int i = 0; i < num_processes; i++) {
            if (!processes[i].completed) {
                all_completed = false;
                break;
            }
        }
        if (all_completed) break;
        
        // Calculating reward and updating Q-table
        update_q_table();

        // Selecting the process to schedule based on Q-value
        int selected_process = select_action();
        Process *p = &processes[selected_process];

        // Saving the current state and Q-table into a  txt file
        save_to_file(iteration++, selected_process,0);

        // Executing the process for the time quantum
        printf("\nExecuting Process ID: %d\n", p->id);
        p->burst_time -= TIME_QUANTUM;
        if (p->burst_time <= 0) {
            p->burst_time = 0;
            p->completed = true;
        }

        // Updating waiting times of other processes
        for (int i = 0; i < num_processes; i++) {
            if (i != selected_process && !processes[i].completed) {
                processes[i].waiting_time += TIME_QUANTUM;
            }
        }

        // Updating state variables (random values for dynamic simulation)
        update_state(selected_process);

        all_completed = true;
        for (int i = 0; i < num_processes; i++) {
            if (!processes[i].completed) {
                all_completed = false;
                break;
            }
        }
        if (all_completed){
            // Saving the last final iteration result
            save_to_file(iteration++, selected_process,1);
        }

        // Removing completed process from Q-table
        if (p->completed) {
            remove_completed_process(selected_process);
        }

        // Displaying updated process table
        display_processes();
    }

    printf("\nAll processes completed!\n");
    return 0;
}

void initialize_processes() {
    printf("Enter the number of processes (max %d): ", MAX_PROCESSES);
    scanf("%d", &num_processes);
    for (int i = 0; i < num_processes; i++) {
        processes[i].id = i;
        processes[i].burst_time = rand() % 20 + 1; 
        processes[i].waiting_time = 0;
        processes[i].system_priority = rand() % 10 + 1;
        processes[i].cpu_utilization = (float)(rand() % 100) / 100.0;
        processes[i].memory_usage = (float)(rand() % 100) / 100.0;
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
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].completed) continue;

        // Calculating reward for the current process
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
    float w1 = 0.35, w2 = 0.2, w3 = 0.2, w4 = 0.15, w5 = 0.05;

    float reward = (w1 * (float)p.system_priority) + 
                   (w2 * -(float)p.burst_time) + 
                   (w3 * (float)p.waiting_time) + 
                   (w4 * -(float)p.cpu_utilization) + 
                   (w5 * (float)p.memory_usage);

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


void save_to_file(int iteration, int process_id,int last) {
    char filename[50];
    sprintf(filename, "output_iteration_%d.txt", iteration);
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Current Process States:\n");
    fprintf(file, "ID\tBurst\tWait\tPriority\tCPU%%\tMemory%%\tCompleted\n");
    for (int i = 0; i < num_processes; i++) {
        fprintf(file, "%d\t%d\t\t%d\t\t%d\t\t\t%.2f\t%.2f\t%s\n",
                processes[i].id, processes[i].burst_time, processes[i].waiting_time,
                processes[i].system_priority, processes[i].cpu_utilization,
                processes[i].memory_usage, processes[i].completed ? "Yes" : "No");
    }

    // Q-table headers
    fprintf(file, "\nQ-Table:\n");
    fprintf(file, "Process ID\tPriority\tCPU Utilization\tMemory Usage\tWaiting Time\tBurst Time\n");

    for (int i = 0; i < num_processes; i++) {
        fprintf(file, "Process %d: ", i);
        // Writing the values of the Q-table with respect to the actual states
        fprintf(file, " %.2f\t\t%.2f\t\t\t%.2f\t\t\t%.2f\t\t\t%.2f\t\n",
                Q_table[i][0], Q_table[i][1], Q_table[i][2], Q_table[i][3], Q_table[i][4]);
    }
    if(last==0){
        fprintf(file, "\nWill now execute Process ID: %d\n", process_id);
    }
    else {
        fprintf(file, "\nAll processes completed!\n");
    }

    fclose(file);
    printf("Saved output to %s\n", filename);
}
