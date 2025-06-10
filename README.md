
# Bounded buffer Producer Consumer
## Objective
Demonstrate my knowledge of Linux kernel multithreading and synchronization tools.
## Function
Creates "producer" and "consumer" threads and two semaphore objects.\
Producer threads write to a `task_struct*` array (buffer) with zombie processes.\
Consumer threads read from the same `task_struct*` array (buffer) of zombie processes.\
The two semaphore objects synchronize the producer/consumer threads' access of the shared buffer.
## Usage
Foreword: the testing utility may not work as it was used on a virtual machine. Use at your own risk.\
Download all project files and run the `test_module.sh` script provided by my university course.\
Origin repository can be found here: https://github.com/CSE330-OS/CSE330-Summer-2025/blob/project-3
### Usage of test_module.sh and expected output:

Usage: Replace `/path/to/code/` with the directory which has your `producer_consumer.c` and `Makefile`:
```bash
Usage: ./test_module.sh /path/to/your/submission/ <prod> <cons> <size> <regular> <zombies>
 <prod>    - the number of producer threads for the kernel module
 <cons>    - the number of consumer threads for the kernel module
 <size>    - the size of the buffer for the kernel module
 <regular> - the number of regular processes to spawn for the process generator
 <zombies> - the number of zombie processes to spawn for the process generator
```

Expected output (from test case 3):
```
[log]: Creating user TestP4...
[log]: Look for Makefile
[log]: ─ file /home/cse330/gta-repo/GTA-CSE330-Summer-2025/Project3/Makefile found
[log]: Look for source file (producer_consumer.c)
[log]: ─ file /home/cse330/gta-repo/GTA-CSE330-Summer-2025/Project3/producer_consumer.c found
[log]: Compile the kernel module
[log]: ─ Compiled successfully
[log]: Starting 0 normal processes
[log]: Load the kernel module
[log]: ─ Loaded successfully
[log]: Starting zombie processes ...
[log]: ─ Total zombies spawned so far: 10/10
[log]: Checking the counts of the running kernel threads
[log]: ─ Found all expected threads
[log]: We will now wait some time to give your kernel module time to cleanup
[log]: └─ We will wait 10 seconds
[log]: Checking the pids of all remaining processes against your output
[log]: - All 10 zombie processes were correctly produced.
[log]: - All 10 zombies were successfully consumed.
[log]: - All zombie PIDs were validly produced.
[log]: - All zombie PIDs were validly consumed.
[log]: - All zombie PIDs correctly produced and consumed.
[log]: ┬─ All zombies were consumed
[log]: └─ None of the regular processes were consumed
[log]: Unload the kernel module
[log]: ─ Kernel module unloaded sucessfully
[log]: Checking to make sure kthreads are terminated
[log]: ─ All threads have been stopped
[zombie_finder]: Passed
[final score]: 25.00/25
[log]: Deleting user TestP4...
```
