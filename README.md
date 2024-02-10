# Assignment 1: COL818
## SHIVAM VERMA (2020CS50442)

**Implement the universality of consensus algorithm in C++**
 
1. Implement the lock-free and wait-free variants.
2. Write your own consensus object using the compare and swap (exchange) instruction (available in both x86 and ARM).
Use the __asm directive to implement the consensus object.
3. Implement a concurrent stack and queue using the consensus object that you created.
4. Show the correctness of your algorithm.

### Files in directory
1. `LF-universalConsensus.cpp`: Lock Free variant of universal consensus object.
2. `WF-universalConsensus.cpp`: Wait Free variant of universal consensus object.
3. `Makefile`: To compile the files
4. `checker.py`: To test the correctness of execution log. To see whether property of stack and queue are maintained?
5. `execution_log.txt`: Final execution log is printed in this file, after all threads have completed their job.
6. `run.sh`: Shell script to run the task
7. `Readme`: You are currently reading this!

### How to Run?
1. In `Makefile` either use either `SRC = LF-universalConsensus.cpp` or `SRC = WF-universalConsensus.cpp`
2. Set number of threads in the `run.sh` file.
3. `chmod +x run.sh`
4. Run `./run.sh`
5. Verification result will be printed in terminal and logs can be seen in `execution_log.txt` file.
