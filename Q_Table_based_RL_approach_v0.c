#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUM_PROCESSES 5
#define NUM_STATES 6         // Add one more state to track processes in the CPU
#define NUM_ACTIONS NUM_PROCESSES // Action corresponds to scheduling each process
#define ALPHA 0.1             // Learning rate
#define GAMMA 0.9             // Discount factor
#define EPSILON 0.2           // Exploration rate for epsilon-greedy policy

// Process structure
typedef struct {
    int pid;
    float burst_time_estimate;
    int waiting_time;
    int is_io_bound;
    int is_system_process;
    float priority;
    int in_cpu;  // Flag to track if the process is in the CPU
} Process;

// Q-table to store Q-values for each (state, action) pair
float q_table[NUM_STATES][NUM_ACTIONS];

// State names for the Q-table
const char* state_names[] = {
    "All processes low priority",
    "Prioritize high burst time",
    "Prioritize I/O bound processes",
    "System processes prioritized",
    "Mixed strategy (default)",
    "Process in CPU"
};

// Initialize the processes with random data for testing
void initialize_processes(Process processes[]) {
    srand(time(0));
    for (int i = 0; i < NUM_PROCESSES; i++) {
        processes[i].pid = i;
        processes[i].burst_time_estimate = (float)(rand() % 10 + 1); // Random burst time estimate
        processes[i].waiting_time = rand() % 10;
        processes[i].is_io_bound = rand() % 2;
        processes[i].is_system_process = rand() % 2;
        processes[i].priority = 0.0; // Initial priority
        processes[i].in_cpu = 0; // Initially, processes are not in the CPU
    }
}

// Function to choose an action using epsilon-greedy policy
int choose_action(int state) {
    if ((float)rand() / RAND_MAX < EPSILON) {
        return rand() % NUM_ACTIONS; // Random action (exploration)
    }
    
    // Exploitation: choose action with max Q-value for the current state
    int best_action = 0;
    for (int a = 1; a < NUM_ACTIONS; a++) {
        if (q_table[state][a] > q_table[state][best_action]) {
            best_action = a;
        }
    }
    return best_action;
}

// Calculate reward based on process attributes
float calculate_reward(Process *process) {
    // Reward function based on shorter waiting time and burst time
    return (10.0 - process->waiting_time) + (10.0 - process->burst_time_estimate);
}

// Q-learning update rule
void update_q_table(int state, int action, int next_state, float reward) {
    float max_q_next = q_table[next_state][0];
    for (int a = 1; a < NUM_ACTIONS; a++) {
        if (q_table[next_state][a] > max_q_next) {
            max_q_next = q_table[next_state][a];
        }
    }

    q_table[state][action] = q_table[state][action] + ALPHA * (reward + GAMMA * max_q_next - q_table[state][action]);
}

// Function to print the ready queue and process priorities
void print_ready_queue(Process ready_queue[], int size) {
    printf("\nReady Queue:\n");
    printf("PID\tBurst Time\tWaiting Time\tPriority\n");
    for (int i = 0; i < size; i++) {
        printf("%d\t%.2f\t\t%d\t\t%.2f\n", ready_queue[i].pid, ready_queue[i].burst_time_estimate, ready_queue[i].waiting_time, ready_queue[i].priority);
    }
}

// Display the Q-table with meaningful state names and process Q-values
void display_q_table() {
    printf("\nQ-Table:\n");
    printf("State\\Action\t");
    for (int a = 0; a < NUM_ACTIONS; a++) {
        printf("P%d\t", a);
    }
    printf("\n");
    for (int s = 0; s < NUM_STATES; s++) {
        // Print state name with padding to align the table
        printf("%-30s", state_names[s]);
        for (int a = 0; a < NUM_ACTIONS; a++) {
            printf("%.2f\t", q_table[s][a]);
        }
        printf("\n");
    }
}

// Function to simulate scheduling with Q-learning
void q_learning_scheduler(Process processes[]) {
    int current_state = 0; // Initial state (example)
    Process ready_queue[NUM_PROCESSES];
    int ready_queue_size = NUM_PROCESSES; // Initially all processes are in the ready queue

    // Initialize ready queue
    for (int i = 0; i < NUM_PROCESSES; i++) {
        ready_queue[i] = processes[i];
    }

    // Training loop with ready queue changes
    for (int episode = 0; episode < 10; episode++) {
        int action = choose_action(current_state);

        // Perform action: select and schedule the chosen process
        Process *scheduled_process = &ready_queue[action];
        printf("\nEpisode %d: Scheduling Process %d\n", episode + 1, scheduled_process->pid);

        // Mark the process as in CPU
        scheduled_process->in_cpu = 1;

        // Simulate reward for scheduling this process
        float reward = calculate_reward(scheduled_process);

        // Print ready queue before updating priorities
        printf("\nReady Queue Before Priority Update:\n");
        print_ready_queue(ready_queue, ready_queue_size);

        // Update priority for each process in ready queue
        for (int i = 0; i < ready_queue_size; i++) {
            if (!ready_queue[i].in_cpu) {  // Update only processes that are not in CPU
                ready_queue[i].priority = q_table[current_state][ready_queue[i].pid];
            }
        }

        // Print ready queue after updating priorities
        printf("\nReady Queue After Priority Update:\n");
        print_ready_queue(ready_queue, ready_queue_size);

        // Display the Q-value used to select the process
        printf("\nQ value for Process %d in State %d: %.2f\n", scheduled_process->pid, current_state, q_table[current_state][scheduled_process->pid]);

        // Transition to a new state (for simplicity, we assume new state = action % NUM_STATES)
        int next_state = action % NUM_STATES;

        // Update Q-table based on observed reward and new state
        update_q_table(current_state, action, next_state, reward);

        // Print updated Q-table
        display_q_table();

        // After scheduling, process finishes and is placed back in ready queue
        scheduled_process->in_cpu = 0;

        // Update the Q-value when the process returns to the ready queue
        q_table[next_state][scheduled_process->pid] += 0.1; // Small increment for demonstration

        // Remove the scheduled process from the ready queue
        // Shift all remaining processes to fill the gap
        for (int i = action; i < ready_queue_size - 1; i++) {
            ready_queue[i] = ready_queue[i + 1];
        }
        ready_queue_size--;  // Decrease the size of the ready queue

        // Print the Q-values for all processes
        printf("\nQ values for all processes in State %d:\n", current_state);
        for (int i = 0; i < NUM_PROCESSES; i++) {
            printf("P%d Q value: %.2f\n", i, q_table[current_state][i]);
        }

        // Move to the next state
        current_state = next_state;
    }

    // Print final Q-values of all processes
    printf("\nFinal Q values for each process:\n");
    printf("Process\t\tFinal Q value\n");
    for (int i = 0; i < NUM_PROCESSES; i++) {
        float final_q_value = q_table[current_state][i];
        printf("P%d\t\t%.2f\n", i, final_q_value);
    }
}

int main() {
    Process processes[NUM_PROCESSES];
    initialize_processes(processes);

    // Train Q-learning scheduler
    q_learning_scheduler(processes);

    return 0;
}
