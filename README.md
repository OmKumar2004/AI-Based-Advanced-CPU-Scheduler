# AI-Based-Advanced-CPU-Scheduler
This consists of simulations based on the AI Scheduler.
Our AI Scheduler is an RL-based scheduler.
- Q Table-based RL approach: This approach helps schedule the process based on multiple decision factors, by looking at it all at once and learning from the past.
                             The code will ask for the number of processes from the user and based on the input it will randomly initialize those number of processes (Assuming all the processes are in the ready queue). The q values are computed based on the weighted reward function and the Bellman equation.

- Deep Q-based RL approach: This approach uses the power of Neural Networks and all these computations are done on it.


GUI Interface for the q table-based scheduler where the user can input the number of processes and the other details of them and based on that user can see the changes in the ready queue, queue table, and process stats is also included in this repo.

xv6: Implementation of q-table based RL approach is also done up to an extent with as of now taking waiting time and memory usage as the states. It is done in Ubuntu 16.04 as in this the xv6 have access to 2 cores. In Ubuntu 22 xv6 uses only 1 core so can't get the real status of the process. For example, if I called ps then ps will become RUNNING and the process that should be running goes to RUNNABLE state. So, I will not be able to see which process is running as for seeing it I have to do ps. (There is an alternate that I can also cprintf the process whenever there is a change in the state of any process in the ready queue)

Further Work
- Complete implementation of Q-table based scheduler in xv6.
