import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
import random
from collections import deque, namedtuple
import matplotlib.pyplot as plt

# Constants
NUM_PROCESSES = 5
STATE_SIZE = (
    6  # System priority, Burst Time, Waiting Time, I/O bound, Memory usage, In CPU
)
ACTION_SIZE = NUM_PROCESSES
MEMORY_SIZE = 10000
BATCH_SIZE = 64
GAMMA = 0.99
EPSILON_START = 1.0
EPSILON_END = 0.01
EPSILON_DECAY = 0.995
LEARNING_RATE = 0.001


# Define Process class with all parameters
class Process:
    def __init__(self, pid):
        self.pid = pid
        self.system_priority = np.random.randint(1, 11)  # 1-10
        self.burst_time = np.random.randint(1, 21)  # 1-20
        self.waiting_time = 0
        self.io_bound = np.random.choice([0, 1])  # Binary
        self.memory_usage = np.random.randint(1, 101)  # 1-100 MB
        self.in_cpu = 0

    def get_state(self):
        return np.array(
            [
                self.system_priority / 10,  # Normalize to [0,1]
                self.burst_time / 20,
                self.waiting_time / 50,  # Assuming max waiting time of 50
                self.io_bound,
                self.memory_usage / 100,
                self.in_cpu,
            ]
        )


# Deep Q-Network architecture
class DQN(nn.Module):
    def __init__(self, state_size, action_size):
        super(DQN, self).__init__()
        self.fc1 = nn.Linear(state_size, 64)
        self.fc2 = nn.Linear(64, 64)
        self.fc3 = nn.Linear(64, action_size)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        return self.fc3(x)


# Experience Replay Memory
Experience = namedtuple(
    "Experience", ["state", "action", "reward", "next_state", "done"]
)


class ReplayMemory:
    def __init__(self, capacity):
        self.memory = deque(maxlen=capacity)

    def push(self, *args):
        self.memory.append(Experience(*args))

    def sample(self, batch_size):
        return random.sample(self.memory, batch_size)

    def __len__(self):
        return len(self.memory)


# CPU Scheduler with Deep Q-Learning
class DQNScheduler:
    def __init__(self):
        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        self.policy_net = DQN(STATE_SIZE, ACTION_SIZE).to(self.device)
        self.target_net = DQN(STATE_SIZE, ACTION_SIZE).to(self.device)
        self.target_net.load_state_dict(self.policy_net.state_dict())

        self.optimizer = optim.Adam(self.policy_net.parameters(), lr=LEARNING_RATE)
        self.memory = ReplayMemory(MEMORY_SIZE)
        self.epsilon = EPSILON_START
        self.steps_done = 0

    def select_action(self, state):
        if random.random() > self.epsilon:
            with torch.no_grad():
                state_tensor = torch.FloatTensor(state).unsqueeze(0).to(self.device)
                q_values = self.policy_net(state_tensor)
                return q_values.max(1)[1].item()
        else:
            return random.randrange(ACTION_SIZE)

    def calculate_reward(self, process):
        # Reward function based on scheduling objectives
        priority_reward = (
            10 - process.system_priority
        ) / 10  # Lower priority preferred initially
        burst_reward = (20 - process.burst_time) / 20  # Lower burst time preferred
        waiting_penalty = -process.waiting_time / 50  # Penalty for waiting time
        io_reward = process.io_bound * 0.5  # Bonus for I/O bound processes
        memory_penalty = -process.memory_usage / 100  # Penalty for high memory usage

        return (
            priority_reward
            + burst_reward
            + waiting_penalty
            + io_reward
            + memory_penalty
        )

    def optimize_model(self):
        if len(self.memory) < BATCH_SIZE:
            return

        experiences = self.memory.sample(BATCH_SIZE)
        batch = Experience(*zip(*experiences))

        state_batch = torch.FloatTensor(np.array(batch.state)).to(self.device)
        action_batch = torch.LongTensor(batch.action).to(self.device)
        reward_batch = torch.FloatTensor(batch.reward).to(self.device)
        next_state_batch = torch.FloatTensor(np.array(batch.next_state)).to(self.device)
        done_batch = torch.FloatTensor(batch.done).to(self.device)

        current_q_values = self.policy_net(state_batch).gather(
            1, action_batch.unsqueeze(1)
        )
        next_q_values = self.target_net(next_state_batch).max(1)[0].detach()
        expected_q_values = reward_batch + (GAMMA * next_q_values * (1 - done_batch))

        # Compute loss and optimize
        loss = nn.MSELoss()(current_q_values.squeeze(), expected_q_values)
        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()

        # Update epsilon
        self.epsilon = max(EPSILON_END, self.epsilon * EPSILON_DECAY)

    def update_target_network(self):
        self.target_net.load_state_dict(self.policy_net.state_dict())


# Simulation environment
class CPUEnvironment:
    def __init__(self):
        self.processes = [Process(i) for i in range(NUM_PROCESSES)]
        self.current_time = 0
        self.performance_metrics = {
            "avg_waiting_time": [],
            "avg_turnaround_time": [],
            "cpu_utilization": [],
        }

    def reset(self):
        self.processes = [Process(i) for i in range(NUM_PROCESSES)]
        self.current_time = 0
        return self.get_state()

    def get_state(self):
        return np.mean([p.get_state() for p in self.processes], axis=0)

    def step(self, action):
        process = self.processes[action]
        process.in_cpu = 1

        # Update waiting times for other processes
        for p in self.processes:
            if p.pid != process.pid:
                p.waiting_time += 1

        # Execute process
        process.burst_time = max(0, process.burst_time - 1)
        self.current_time += 1

        # Calculate reward
        reward = self.calculate_reward(process)

        # Check if episode is done
        done = all(p.burst_time == 0 for p in self.processes)

        # Update metrics
        if done:
            self.update_performance_metrics()

        return self.get_state(), reward, done

    def calculate_reward(self, process):
        return -0.1 * process.waiting_time + (1.0 if process.burst_time == 0 else 0.0)

    def update_performance_metrics(self):
        avg_waiting = np.mean([p.waiting_time for p in self.processes])
        avg_turnaround = np.mean(
            [p.waiting_time + p.burst_time for p in self.processes]
        )
        cpu_util = self.current_time / (
            self.current_time + sum(p.waiting_time for p in self.processes)
        )

        self.performance_metrics["avg_waiting_time"].append(avg_waiting)
        self.performance_metrics["avg_turnaround_time"].append(avg_turnaround)
        self.performance_metrics["cpu_utilization"].append(cpu_util)


def train_scheduler(num_episodes=10):
    env = CPUEnvironment()
    scheduler = DQNScheduler()
    rewards_history = []

    for episode in range(num_episodes):
        state = env.reset()
        total_reward = 0
        done = False

        while not done:
            action = scheduler.select_action(state)
            next_state, reward, done = env.step(action)

            scheduler.memory.push(state, action, reward, next_state, done)
            scheduler.optimize_model()

            state = next_state
            total_reward += reward

        # if episode % 10 == 0:
        scheduler.update_target_network()
        print(
            f"Episode {episode}, Total Reward: {total_reward:.2f}, Epsilon: {scheduler.epsilon:.2f}"
        )
        rewards_history.append(total_reward)

    return scheduler, env, rewards_history


# Training and visualization
def plot_metrics(rewards_history, env):
    plt.figure(figsize=(15, 5))

    plt.subplot(131)
    plt.plot(rewards_history)
    plt.title("Training Rewards")
    plt.xlabel("Episode")
    plt.ylabel("Total Reward")

    plt.subplot(132)
    plt.plot(env.performance_metrics["avg_waiting_time"])
    plt.title("Average Waiting Time")
    plt.xlabel("Episode")
    plt.ylabel("Time Units")

    plt.subplot(133)
    plt.plot(env.performance_metrics["cpu_utilization"])
    plt.title("CPU Utilization")
    plt.xlabel("Episode")
    plt.ylabel("Utilization %")

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    scheduler, env, rewards_history = train_scheduler(num_episodes=50)
    plot_metrics(rewards_history, env)
