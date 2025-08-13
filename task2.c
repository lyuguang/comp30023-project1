#include "task.h"

// Function to execute round-robin scheduling with memory management.
void task2(char *filename, int quantum) {
    Process *wait_processes = NULL;

    // Initialize scheduling and memory management variables.
    int proc_num, proc_remaining, current_time, flag, current_process_num, total_turnaround_time, makespan;
    double max_overhead_time, total_overhead_time;
    initialize_const(&proc_num, &proc_remaining, &current_time, &flag, &current_process_num, 
                        &total_turnaround_time, &max_overhead_time, &total_overhead_time, &makespan);
    
    // Read process data from the file specified by filename.
    read_processes(filename, &wait_processes, &proc_num); 
    int total_processes = proc_num; // Total number of processes to be managed.

    // Initialize the queue for processes ready to run and the memory queue.
    Queue *ready_processes = initialize_ready_queue();
    Queue *memory = initialize_ready_queue(); // This queue will manage memory allocations.
    memory->total_capacity = MEMORY_SIZE; // Total memory capacity available.

    int finished;
    char cur_process[MAX_PROCESS_NAME]; // Current process name for tracking.

    // Main loop for round-robin scheduling.
    while (proc_num != 0 || proc_remaining != 0) {
        finished = 0;

        // Check if a new process has arrived and needs to be added to the ready queue.
        if (proc_num > 0 && wait_processes[current_process_num].arrival_time <= current_time) {
            add_process_to_ready_queue(ready_processes, memory, &wait_processes[current_process_num], &flag);
            current_process_num++;
            proc_num--;
            proc_remaining++;
            continue;
        }

        // Process termination condition.
        if (ready_processes->head != NULL && ready_processes->head->process->remaining_time <= 0) {
            proc_remaining--;
            printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", current_time, ready_processes->head->process->process_name, proc_remaining);
            remove_finished_process(ready_processes, memory, current_time, &makespan, &total_turnaround_time, &total_overhead_time, &max_overhead_time, &finished);
        }

        // Process execution condition.
        if (ready_processes->head != NULL) {
            if (flag) {
                // Allocate memory and prepare the process for running.
                memory->head->process->memory_start = 0; // Initialize memory starting point.
                memory->head->memoryHole->start = 0;
                memory->head->memoryHole->length = memory->head->process->memory_length;
                memory->total_capacity -= memory->head->process->memory_length; // Deduct the allocated memory from the total capacity.
                if (memory->total_capacity != 0) {
                    Node *new_hole = (Node *)malloc(sizeof(Node));
                    new_hole->memoryHole = (MemoryHole *)malloc(sizeof(MemoryHole));

                    new_hole->process = NULL;
                    new_hole->memoryHole->start = memory->head->process->memory_length;
                    new_hole->memoryHole->length = MEMORY_SIZE - memory->head->process->memory_length;
                    new_hole->next = NULL;
                    
                    memory->head->next = new_hole;
                    memory->tail = new_hole;
                }
                ready_processes->head->process->allocated = 1;
                int memory_usage = calculate_memory_usage(memory);
                printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,allocated-at=%d\n", current_time, 
                    ready_processes->head->process->process_name, ready_processes->head->process->remaining_time, memory_usage, ready_processes->head->process->memory_start);
                ready_processes->head->process->remaining_time -= quantum;
                strcpy(cur_process, ready_processes->head->process->process_name); // Update current process tracking.
                flag = 0;
            } else {
                // Rotate the ready queue to simulate round-robin scheduling.
                if (!finished) {
                    update_reday_processes(ready_processes);
                }

                // Memory allocation check and assignment.
                if (ready_processes->head->process->allocated == 0) {
                    if (allocate_memory(memory, ready_processes->head->process) == 1) {
                        ready_processes->head->process->allocated = 1;
                        memory->total_capacity -= ready_processes->head->process->memory_length;
                    } else {
                        strcpy(cur_process, ready_processes->head->process->process_name);
                        continue;
                    }
                }

                // Update the running state of the process.
                if (strcmp(ready_processes->head->process->process_name, cur_process) != 0) {
                    int memory_usage = calculate_memory_usage(memory);
                    printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,allocated-at=%d\n", current_time, 
                        ready_processes->head->process->process_name, ready_processes->head->process->remaining_time, memory_usage, ready_processes->head->process->memory_start);
                    ready_processes->head->process->remaining_time -= quantum;
                    strcpy(cur_process, ready_processes->head->process->process_name);
                } else {
                    ready_processes->head->process->remaining_time -= quantum;
                }
            }
        }
        current_time += quantum; // Advance the scheduling clock by one quantum.
    }
    // Print final statistics and clean up resources.
    print_message(&total_turnaround_time, &total_processes, &total_overhead_time, &max_overhead_time, &makespan);

    free(wait_processes);
    free_ready_queue(ready_processes);
    free_ready_queue(memory);
}

// Adds a new process to the ready queue and potentially to the memory management queue.
void add_process_to_ready_queue(Queue *ready_processes, Queue *memory, Process *process, int *flag){
    Node *new_proc = (Node *)malloc(sizeof(Node)); // Dynamically allocate a new node for the ready queue.
    new_proc->process = process;
    new_proc->next = NULL;

    // Check if the ready queue is empty.
    if (ready_processes->head == NULL){
        // Initialize the ready queue with the new process.
        ready_processes->head = new_proc;
        ready_processes->tail = new_proc;

        // Also initialize the memory management queue.
        Node *new_memory = (Node *)malloc(sizeof(Node));
        new_memory->process = malloc(sizeof(Process)); // Allocate memory for the process's details.
        new_memory->memoryHole = (MemoryHole *)malloc(sizeof(MemoryHole)); // Allocate memory for the memory hole associated with this process.
        memcpy(new_memory->process, new_proc->process, sizeof(Process)); // Copy process details into the memory queue.
        new_memory->next = NULL;

        // Set this new node as the head and tail of the memory queue.
        memory->head = new_memory;
        memory->tail = new_memory;

        *flag = 1; // Set flag to indicate a new process has been added.
    }
    else{
        // Append the new process to the end of the ready queue.
        ready_processes->tail->next = new_proc;
        ready_processes->tail = new_proc;
    }
}

// Allocates memory for a process, adjusting memory holes accordingly.
int allocate_memory(Queue *memory, Process *process){
    Node *current = memory->head;

    while (current != NULL){
        // Check for an empty memory hole that can accommodate the process.
        if (current->process == NULL){
            // Exact fit to the memory hole.
            if (current->memoryHole->length == process->memory_length){
                current->process = process;
                process->memory_start = current->memoryHole->start;
                return 1;
            }
            // Fit process into a larger hole and adjust the hole size.
            else if (current->memoryHole->length > process->memory_length){
                current->process = process;
                current->process->memory_start = current->memoryHole->start;

                // Create a new hole for the remaining part of the memory.
                Node *new_hole = (Node *)malloc(sizeof(Node));
                new_hole->memoryHole = (MemoryHole *)malloc(sizeof(MemoryHole));

                new_hole->process = NULL;
                new_hole->memoryHole->start = current->process->memory_start + current->process->memory_length;
                new_hole->memoryHole->length = current->memoryHole->length - current->process->memory_length;
                current->memoryHole->length = current->process->memory_length;
                new_hole->next = current->next;
                current->next = new_hole;
                if (new_hole->next == NULL){
                    memory->tail = new_hole; // Update the tail if necessary.
                }
                return 1;
            }
        }
        current = current->next;
    }
    return 0; // Return 0 if no suitable memory hole was found.
}

// Coalesces consecutive memory holes into a single larger hole.
void connect_hole(Queue *memory) {
    Node *current = memory->head;
    Node *next = memory->head->next;

    while (next != NULL){
        // Merge adjacent free memory holes.
        if (current->process == NULL && next->process == NULL){
            current->memoryHole->length += next->memoryHole->length;
            Node *tmp = next;
            next = next->next;
            current->next = next;

            free(tmp); // Free the merged node.
            continue;
        }
        current = next;
        next = current->next;
    }
}

// Deallocates memory assigned to a process and attempts to coalesce memory holes.
void deallocate_memory(Queue *memory, Process *process) {
    Node *current = memory->head;

    while (current != NULL) {
        if (current->process != NULL && strcmp(current->process->process_name, process->process_name) == 0) {
            current->process = NULL; // Mark the memory hole as free.
            connect_hole(memory); // Attempt to merge adjacent free memory holes.
            return;
        }
        current = current->next;
    }
}


void remove_finished_process(Queue *ready_processes, Queue *memory, int current_time, int *makespan, 
                            int *total_turnaround_time, double *total_overhead_time, double *max_overhead_time, int *finished){
    *makespan = current_time;
    int turnaround_time = current_time - ready_processes->head->process->arrival_time;                // record turnaround time
    double time_overhead = (double) turnaround_time / ready_processes->head->process->service_time;   // recording the overhead time
    *total_turnaround_time += turnaround_time;                                                         // record total turnaround time
    *total_overhead_time += time_overhead;                                                             // recoring the total overhead time

    // get the max of overhead time
    if(time_overhead > *max_overhead_time) {
        *max_overhead_time = time_overhead;
    }
    deallocate_memory(memory,ready_processes->head->process);
    memory->total_capacity += ready_processes->head->process->memory_length;

    Node *temp = ready_processes->head;                                                               // Save a reference to the head node
    ready_processes->head = ready_processes->head->next;                                              // Set the next node as the new head node
    // If the new head node is NULL, the queue is empty, and the tail node should also be NULL
    if (ready_processes->head == NULL){
        ready_processes->tail = NULL;
    }

    free(temp);   // Free the memory of the original head node
    *finished = 1;
}

int calculate_memory_usage(Queue *memory){
    int memory_use = 0;

    double memory_usage = (double)(MEMORY_SIZE - memory->total_capacity) / MEMORY_SIZE * 100;
    memory_use = ceil(memory_usage);

    return memory_use;
}