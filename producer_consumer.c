#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/semaphore.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Flores");

static int prod = 1;
module_param(prod, int, 0);

static int cons = 1;
module_param(cons, int, 0);

static int size = 1;
module_param(size, int, 0);

static int uid = 0;
module_param(uid, int, 0);

// Task_structs for process management
struct task_struct* p;					// Array of system processes
struct task_struct** prod_threads; 		// Array of producer thread task_structs pointers
struct task_struct** cons_threads; 		// Array of consumer thread task_structs pointers
struct task_struct** zombie_buffer; 	// Array of zombie task_structs pointers
// Semaphore for buffer access synchronization
struct semaphore empty;
struct semaphore full;

int in=0;
int out=0;
size_t process_counter = 0;

// Producer thread function for loading zombie processes into the buffer
static int prod_func(void *arg)
{
	while (!kthread_should_stop())
	{// While thread is active, produce processes into the zombie_buffer

		// Get producer thread name for logging
		char thread_name[TASK_COMM_LEN] = {};
		get_task_comm(thread_name, current);

		// Handle each process
		for_each_process(p) 
		{
			// Check if process is part of the specified UID
			if (p->cred->uid.val != uid) {
				continue;
			}
			++process_counter;

			// Check if process is a zombie
			if (p->exit_state == EXIT_ZOMBIE)
			{// then add it to the buffer
				// Check semaphore for available buffer space
				down_interruptible(&empty);
				
				zombie_buffer[in] = p; 	// Load zombie process into the buffer
				in = (in + 1) % size;	// Increment index to next open buffer space

				// and "produce" the zombie process
				printk("[%s] has produced a zombie process with pid %d and parent pid %d\n", thread_name, p->pid, p->parent->pid);
				
				// Signal full semaphore that a process has been added to the buffer
				up(&full);
			}
		}
	}
	
	return 0;
}
// Consumer thread function for unloading zombie processes from the buffer
static int cons_func(void *arg)
{
	while (!kthread_should_stop())
	{// While thread is active, consume processes from the zombie_buffer

		// Get consumer thread name for logging
		char thread_name[TASK_COMM_LEN] = {};
		get_task_comm(thread_name, current);

		// Check the semaphore for buffer data to be consumed
		down_interruptible(&full);

		// and "consume" the zombie process
		printk("[%s] has consumed a zombie process with pid %d and parent pid %d\n", thread_name, zombie_buffer[out]->pid, zombie_buffer[out]->parent->pid);
		out = (out + 1) % size;	// Increment index to next full buffer space
		
		// Signal empty semaphore that a process has been removed from the buffer
		up(&empty);
	}
	return 0;
}

static struct task_struct** allocateMemory(int numThreads) {
	return kmalloc(numThreads * sizeof(struct task_struct*), GFP_KERNEL);
}

static int __init producer_consumer(void)
{
	// Allocate memory for task_struct* arrays
	prod_threads = allocateMemory(prod);
	cons_threads = allocateMemory(cons);
	zombie_buffer = allocateMemory(size);
	// Check if memory was allocated correctly
	if (!prod_threads || !cons_threads || !zombie_buffer) {
		pr_err("prod_threads  = %d", prod_threads);
		pr_err("cons_threads  = %d", cons_threads);
		pr_err("zombie_buffer = %d", zombie_buffer);
		goto fail;
	}

	// Initialize semaphores for buffer access synchronization
	sema_init(&empty, size);
	sema_init(&full, 0);

	int i;
	for (i = 0; i < prod; i++)
	{// Initialize all of the producer threads
		prod_threads[i] = kthread_run(prod_func, NULL, "Producer-%d", i+1);
		// Check if the threads were produced correctly
		if (IS_ERR(prod_threads[i])) {
			pr_err("prod_threads[%d] failed to initialize!", i);
			prod_threads[i] = NULL;
			goto fail;
		}
	}
	for (i = 0; i < cons; i++)
	{// Initialize all of the consumer threads
		cons_threads[i] = kthread_run(cons_func, NULL, "Consumer-%d", i+1);
		// Check if the threads were produced correctly
		if (IS_ERR(cons_threads[i])) {
			pr_err("cons_threads[%d] failed to initialize!", i);
			cons_threads[i] = NULL;
			goto fail;
		}
	}

	return 0;
// Catch any errors
fail:
	return -ENOMEM;
}

static void __exit producer_consumer_exit(void)
{
	// Stop the threads
	int i;
	for (i = 0; i < prod; i++) {
		if (prod_threads && prod_threads[i] && !IS_ERR(prod_threads[i])) {
			kthread_stop(prod_threads[i]);
			prod_threads[i] = NULL;
		}
	}
	for (i = 0; i < cons; i++) {
		if (cons_threads && cons_threads[i] && !IS_ERR(cons_threads[i])) {
			kthread_stop(cons_threads[i]);
			cons_threads[i] = NULL;
		}
	}

	// Free the memory
	if (prod_threads) {
		kfree(prod_threads);
		prod_threads = NULL;
	}
	if (cons_threads) {
		kfree(cons_threads);
		cons_threads = NULL;
	}
	if (zombie_buffer) {
		kfree(zombie_buffer);
		zombie_buffer = NULL;
	}
}

module_init(producer_consumer);
module_exit(producer_consumer_exit);
