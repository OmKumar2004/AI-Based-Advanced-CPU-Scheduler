# AI-Based-Advanced-CPU-Scheduler
This consists of simulations based on the AI Scheduler.
Our AI Scheduler is an RL-based scheduler.
- Q Table-based RL approach: This approach helps schedule the process based on multiple decision factors, by looking at it all at once and learning from the past.
                             The code will ask for the number of processes from the user and based on the input it will randomly initialize those number of processes (Assuming all the processes are in the ready queue). The q values are computed based on the weighted reward function and the Bellman equation.

- Deep Q-based RL approach: This approach uses the power of Neural Networks and all these computations are done on it.


There is also a GUI Interface for the q table-based scheduler where the user can input the number of processes and the other details of them and based on that user can see the changes in the ready queue, queue table, and process stats.



Further Work
- Implementation of Q-table based scheduler in xv6.
