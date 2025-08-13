#include "task.h"

/**
 * Function to execute the round-robin scheduling algorithm.
*/
void task1(char *filename, int quantum) {
    Process *wait_processes = NULL;

    int proc_num, proc_remaining, current_time, flag, current_process_num, total_turnaround_time, makespan;
    double max_overhead_time, total_overhead_time;
    initialize_const(&proc_num, &proc_remaining, &current_time, &flag, &current_process_num, 
                        &total_turnaround_time, &max_overhead_time, &total_overhead_time, &makespan);

    read_processes(filename, &wait_processes, &proc_num);      // read processes from the input file
    int total_processes = proc_num;                            // record the total processes

    Queue *ready_processes = initialize_ready_queue();         // initialize the ready queue

    char cur_process[MAX_PROCESS_NAME];                        // record current process name
    
    // Round-robin scheduling loop
    while (proc_num != 0 || proc_remaining != 0) {
        // check if the current time is arrival time
        if (proc_num > 0 && wait_processes[current_process_num].arrival_time <= current_time) {

            add_process_to_ready_queue_task1(ready_processes, &wait_processes[current_process_num], &flag);
            // Remove the process from the waiting queue
            current_process_num++;
            proc_num--;
            proc_remaining++;
            continue;
        }
        
        if (ready_processes->head != NULL) {
            // Delete the head process
            if (ready_processes->head->process->remaining_time <= 0) {
                proc_remaining--;
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", current_time, ready_processes->head->process->process_name, proc_remaining);
                remove_finished_process_task1(ready_processes, current_time, &makespan, &total_turnaround_time, &total_overhead_time, &max_overhead_time);
            }
        }
        
        if (ready_processes->head != NULL) {
            update_running_process_task1(ready_processes, current_time, quantum, &flag, cur_process);
        }
        
        current_time += quantum;
    }
    
    print_message(&total_turnaround_time, &total_processes, &total_overhead_time, &max_overhead_time, &makespan);
    
    free(wait_processes);
    free_ready_queue(ready_processes);
}

/**
 * Function to read processes from the input file
*/
void read_processes(const char *filename, Process **processes, int *num_processes) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    *processes = NULL;
    *num_processes = 0;

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        (*num_processes)++;
        *processes = realloc(*processes, *num_processes * sizeof(Process));
        Process *process = &(*processes)[*num_processes - 1];

        // Parse the process information from the input file
        sscanf(line, "%d %s %d %d", &process->arrival_time, process->process_name, &process->service_time, &process->memory_length);
        process->remaining_time = process->service_time;

        // initialize memory_start and memory_end to -1, the memory has not been allocated yet
        process->memory_start = 0;
        process->allocated = 0;

        process->pages = process->memory_length / PAGES_SIZE;
        if (process->memory_length % PAGES_SIZE != 0) {
            process->pages++; // Round up if there's a remainder 
        }

        for(int i = 0; i < MAX_PAGES; i++){
            process->memory_pages[i] = -1;
        }
        process->allocated_pages = 0;
    }

    fclose(file);
}

/**
 * Initialize the ready queue, allocate memory and set the head and tail pointers to NULL
*/
Queue* initialize_ready_queue() {
    // Allocate memory to store the ready queue
    Queue *ready_processes = (Queue *)malloc(sizeof(Queue));
    ready_processes->head = NULL;
    ready_processes->tail = NULL;
    return ready_processes;
}

/**
 * Initializes constants used in the scheduling process.
*/
void initialize_const(int *proc_num, int *proc_remaining, int *current_time, int *flag, int *current_process_num,
                     int *total_turnaround_time, double *max_overhead_time, double *total_overhead_time, int *makespan){
    *proc_num = 0;
    *proc_remaining = 0;                                    // record how many process are runing
    *current_time = 0;                                      // current time during the execution of the round-robin
    *flag = 0;                                              // the process is ready or running
    *current_process_num = 0;                               // record currently process number
    *total_turnaround_time = 0;
    *max_overhead_time = 0.0;
    *total_overhead_time = 0.0;
    *makespan = 0;
}

/**
 * Add newly arrived processes to the tail of the ready queue.
*/
void add_process_to_ready_queue_task1(Queue *ready_processes, Process *process, int *flag) {
    // Create a new node
    Node *new_proc = (Node *)malloc(sizeof(Node));
    new_proc->process = process;
    new_proc->next = NULL;
    if (ready_processes->head == NULL) {
        ready_processes->head = new_proc;
        ready_processes->tail = new_proc;
        *flag = 1;
    } else {
        ready_processes->tail->next = new_proc;
        ready_processes->tail = new_proc;
    }
}

/**
 * Remove completed processes and update related statistics
*/
void remove_finished_process_task1(Queue *ready_processes, int current_time, int *makespan, int *total_turnaround_time, double *total_overhead_time, double *max_overhead_time) {
    *makespan = current_time;
    // record turnaround time
    int turnaround_time = current_time - ready_processes->head->process->arrival_time;
    // recording the overhead time
    double time_overhead = (double)turnaround_time / ready_processes->head->process->service_time;
    // record total turnaround time
    *total_turnaround_time += turnaround_time;
    // recoring the total overhead time
    *total_overhead_time += time_overhead;
    // get the max of overhead time
    if (time_overhead > *max_overhead_time) {
        *max_overhead_time = time_overhead;
    }
    // Save a reference to the head node
    Node *temp = ready_processes->head;
    // Set the next node as the new head node
    ready_processes->head = ready_processes->head->next;
    // If the new head node is NULL, the queue is empty, and the tail node should also be NULL
    if (ready_processes->head == NULL) {
        ready_processes->tail = NULL;
    }
    // Free the memory of the original head node
    free(temp);
}

/**
 * Update the currently running process and handle the allocation of time slices
*/
void update_running_process_task1(Queue *ready_processes, int current_time, int quantum, int *flag, char *cur_process) {
    if (*flag) {
        printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", current_time, ready_processes->head->process->process_name, ready_processes->head->process->remaining_time);
        ready_processes->head->process->remaining_time -= quantum;
        // Record the current process
        strcpy(cur_process, ready_processes->head->process->process_name);
        *flag = 0;
    } else {
        update_reday_processes(ready_processes);
        if (strcmp(ready_processes->head->process->process_name, cur_process) != 0) {
            printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", current_time, ready_processes->head->process->process_name, ready_processes->head->process->remaining_time);
            ready_processes->head->process->remaining_time -= quantum;
            // Record the current process
            strcpy(cur_process, ready_processes->head->process->process_name);
        } else {
            ready_processes->head->process->remaining_time -= quantum;
        }
    }
}

/**
 * Cycles the ready queue, moving the head to the tail to implement round-robin behavior.
*/
void update_reday_processes(Queue *ready_processes){
    ready_processes->tail->next = ready_processes->head;
    ready_processes->tail = ready_processes->head;
    ready_processes->head = ready_processes->head->next;
    ready_processes->tail->next =NULL;
}

/**
 * print the messages
*/
void print_message(int *total_turnaround_time, int *total_processes, double *total_overhead_time, double *max_overhead_time, int *makespan){

    // get the average time
    double avg_turnaround = (double)(*total_turnaround_time) / *total_processes;
    double avg_overhead_time = *total_overhead_time / *total_processes;
    avg_overhead_time = round(avg_overhead_time * 100) / 100;

    printf("Turnaround time %d\n", (int)round(avg_turnaround));
    printf("Time overhead %.2lf %.2lf\n", *max_overhead_time, avg_overhead_time);
    printf("Makespan %d\n", *makespan);
}

/**
 * Frees the memory allocated for the ready queue, including all of its nodes.
*/
void free_ready_queue(Queue *ready_processes) {
    Node *current = ready_processes->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    free(ready_processes);
}

