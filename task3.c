
#include "task.h"

void task3(char *filename, int quantum) {
    Process *wait_processes = NULL;

    int proc_num, proc_remaining, current_time, flag, current_process_num, total_turnaround_time, makespan;
    double max_overhead_time, total_overhead_time;
    initialize_const(&proc_num, &proc_remaining, &current_time, &flag, &current_process_num, 
                        &total_turnaround_time, &max_overhead_time, &total_overhead_time, &makespan);

    read_processes(filename, &wait_processes, &proc_num);        // read processes from the input file
    int total_processes = proc_num;                              // record the total processes
    Queue *ready_processes = initialize_ready_queue();           // initialize the ready queue

    int finished;
    int allocate = 0;
    char cur_process[MAX_PROCESS_NAME];                          // record current process name

    char *evicted_frames;
    
    int memory[512] = {0};
    int pages_capacity = 512;

    // Round-robin scheduling loop
    while (proc_num != 0 || proc_remaining != 0) {
        finished = 0;
        // check if the current time is arrival time
        if (proc_num > 0 && wait_processes[current_process_num].arrival_time <= current_time) {
            add_process_to_ready_queue_task1(ready_processes, &wait_processes[current_process_num], &flag);
            // Remove the process from the waiting queue
            current_process_num++;
            proc_num--;
            proc_remaining++;
            continue;
        }
        
        if (ready_processes->head != NULL) {  // Check if there is a process at the head of the queue.
            if (ready_processes->head->process->remaining_time <= 0) {
                evicted_frames = arrayToString(ready_processes->head->process->memory_pages, ready_processes->head->process->pages);
                printf("%d,EVICTED,evicted-frames=[%s]\n", current_time, evicted_frames);
                // Free up the memory pages used by the process.
                for (int i = 0; i < ready_processes->head->process->pages; i++) {
                    memory[ready_processes->head->process->memory_pages[i]] = 0;
                    ready_processes->head->process->memory_pages[i] = -1;
                }
                pages_capacity += ready_processes->head->process->pages;  // Reclaim the pages back to the total capacity.
                ready_processes->head->process->allocated = 0;  // Mark the process as not allocated.
                proc_remaining--;  // Decrement the count of remaining processes.
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", current_time, ready_processes->head->process->process_name, proc_remaining);
                remove_finished_process_task1(ready_processes, current_time, &makespan, &total_turnaround_time, &total_overhead_time, &max_overhead_time);
                finished = 1;  // Mark the current process handling as finished.
            }
        }
        
        if (ready_processes->head != NULL) {
            if (flag) {
                // allocate memory
                for (int i = 0; i < ready_processes->head->process->pages; i++) {
                    memory[i] = 1;
                    ready_processes->head->process->memory_pages[i] = i;
                }
                evicted_frames = arrayToString(ready_processes->head->process->memory_pages, ready_processes->head->process->pages);
                ready_processes->head->process->allocated = 1;
                pages_capacity -= ready_processes->head->process->pages;
                int memory_usage = page_calculate_usage(pages_capacity);
                printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[%s]\n", current_time, ready_processes->head->process->process_name, 
                        ready_processes->head->process->remaining_time, memory_usage, evicted_frames);
                ready_processes->head->process->remaining_time -= quantum;
                // Record the current process
                strcpy(cur_process, ready_processes->head->process->process_name);
                flag = 0;
            } else {
                if(!finished && !allocate){
                    update_reday_processes(ready_processes);
                }

                // allocated memory
                if (ready_processes->head->process->allocated == 0){
                    if (pages_capacity >= ready_processes->head->process->pages) {
                        allocate_memory_pages_task3(memory, ready_processes);  // Allocate memory for the process.
                        pages_capacity -= ready_processes->head->process->pages;  // Decrease the available pages capacity by the amount used by this process.
                        allocate = 0;
                    } else {
                        Node *current = ready_processes->head->next; // start from secode node of linked list
                        while (current != NULL) {
                            if (current->process->allocated == 1) {  // Check if the current node's process has allocated memory.
                                evicted_frames = arrayToString(current->process->memory_pages, current->process->pages);  // Retrieve and display the memory frames to be evicted.
                                printf("%d,EVICTED,evicted-frames=[%s]\n", current_time, evicted_frames);

                                free_process_memory_pages_task3(current, memory, &pages_capacity);
                                break; // Exit the loop after finding a process to evict.
                            }
                            // If allocated is not 1, continue looking for the next node
                            current = current->next;
                        }
                        allocate = 1;
                        continue;
                    }
                }
                // Execute the current process if it's not the same as the last recorded process.
                if (strcmp(ready_processes->head->process->process_name, cur_process) != 0) {
                    int memory_usage = page_calculate_usage(pages_capacity);
                    evicted_frames = arrayToString(ready_processes->head->process->memory_pages, ready_processes->head->process->pages);
                    printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[%s]\n", current_time, ready_processes->head->process->process_name, 
                                ready_processes->head->process->remaining_time, memory_usage, evicted_frames);
                    ready_processes->head->process->remaining_time -= quantum;
                    // Record the current process
                    strcpy(cur_process, ready_processes->head->process->process_name);
                } else {  // If it is the same process, simply decrease its remaining time by the quantum without further checks.
                    ready_processes->head->process->remaining_time -= quantum;
                }
            }
        }
        
        current_time += quantum;
    }
    
    print_message(&total_turnaround_time, &total_processes, &total_overhead_time, &max_overhead_time, &makespan);
    
    free(wait_processes);
    free_ready_queue(ready_processes);
}


char* arrayToString(int arr[], int pages) {
    static char str[MAX_PAGES * 2];
    int len = 0;   // Initialize the string length to 0

    // Loop through the array and format each element into a string
    for (int i = 0; i < pages; i++) {
        if (i < pages - 1) {
            // If not the last element, add comma
            len += sprintf(str + len, "%d,", arr[i]);
        } else {
            // No comma added to last element
            len += sprintf(str + len, "%d", arr[i]);
        }
    }

    return str;
}


void allocate_memory_pages_task3(int memory[], Queue *ready_processes){
    int index = 0;
    for (int i = 0; i < ready_processes->head->process->pages;) {
        if (memory[index] == 0){
            // If the value of memory[index] is 0, set it to 1 and increment index
            memory[index] = 1;
            ready_processes->head->process->memory_pages[i] = index;
            index++;
            i++;
        } else {
            // If the value of memory[index] is not 0, only index will be incremented
            index++;
        }
    }
    ready_processes->head->process->allocated = 1;
}

// Frees the memory pages allocated to a process and updates the overall pages capacity.
void free_process_memory_pages_task3(Node *current, int memory[], int *pages_capacity){
    // Loop through all pages allocated to the current process.
    for (int i = 0; i < current->process->pages; i++) {
        // Set the memory slot to 0 to indicate that the page is now free.
        memory[current->process->memory_pages[i]] = 0;
        // Reset the page index in the process's memory_pages array to -1, indicating no page is allocated.
        current->process->memory_pages[i] = -1;
    }
    // Increase the total pages capacity by the number of pages previously allocated to this process.
    *pages_capacity += current->process->pages;
    // Set the process's allocated flag to 0, indicating it no longer holds any memory.
    current->process->allocated = 0;
}

// Calculates the percentage of memory usage based on the remaining page capacity.
int page_calculate_usage(int page_capacity){
    int memory_use = 0;

    // Calculate memory usage as a percentage of the total pages (MAX_PAGES).
    double memory_usage = (double)(MAX_PAGES - page_capacity) / MAX_PAGES * 100;
    // Round up the result to the nearest integer to avoid under-reporting usage.
    memory_use = ceil(memory_usage);

    return memory_use;
}
