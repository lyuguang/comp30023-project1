#include "task.h"

void task4(char *filename, int quantum) {
    Process *wait_processes = NULL;
    int proc_num, proc_remaining, current_time, flag, current_process_num, total_turnaround_time, makespan;
    double max_overhead_time, total_overhead_time;
    initialize_const(&proc_num, &proc_remaining, &current_time, &flag, &current_process_num, 
                        &total_turnaround_time, &max_overhead_time, &total_overhead_time, &makespan);

    read_processes(filename, &wait_processes, &proc_num);       // read processes from the input file
    Queue *ready_processes = initialize_ready_queue();          // initialize the ready queue

    int total_processes = proc_num;                             // record the total processes
    int finished;
    int allocate = 0;
    int evicted = 0;
    char cur_process[MAX_PROCESS_NAME];                         // record current process name

    char *evicted_frames;
    
    int memory[512] = {0};
    int pages_capacity = 512;
    int need_pages = 0;                                         // int memory_capacity = MEMORY_SIZE;
    int evicted_frames_array[512] = {-1};
    int count = 0;

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
        
        if (ready_processes->head != NULL) {
            if (ready_processes->head->process->remaining_time <= 0) {
                evicted_frames = arrayToString(ready_processes->head->process->memory_pages, ready_processes->head->process->allocated_pages);
                printf("%d,EVICTED,evicted-frames=[%s]\n", current_time, evicted_frames);

                for (int i = 0; i < ready_processes->head->process->allocated_pages; i++) {
                    memory[ready_processes->head->process->memory_pages[i]] = 0;
                    ready_processes->head->process->memory_pages[i] = -1;
                }
                pages_capacity += ready_processes->head->process->allocated_pages;
                proc_remaining--;
                printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", current_time, ready_processes->head->process->process_name, proc_remaining);
                remove_finished_process_task1(ready_processes, current_time, &makespan, &total_turnaround_time, &total_overhead_time, &max_overhead_time);
                finished = 1;
            }
        }
        
        if (ready_processes->head != NULL) {
            if (flag) {
                // allocate memory
                for (int i = 0; i < ready_processes->head->process->pages; i++) {
                    memory[i] = 1;  // Mark each page in memory as used.
                    ready_processes->head->process->memory_pages[i] = i;  // Store the allocated page number in the process's page array.
                }
                ready_processes->head->process->allocated_pages = ready_processes->head->process->pages;
                evicted_frames = arrayToString(ready_processes->head->process->memory_pages, ready_processes->head->process->pages);
                pages_capacity -= ready_processes->head->process->pages;  // Reduce the total pages capacity by the number of pages allocated.
                int memory_usage = page_calculate_usage(pages_capacity);
                printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[%s]\n", current_time, ready_processes->head->process->process_name, 
                        ready_processes->head->process->remaining_time, memory_usage, evicted_frames);
                ready_processes->head->process->remaining_time -= quantum;  // Decrement the process's remaining time by the quantum.
                // Record the current process
                strcpy(cur_process, ready_processes->head->process->process_name);
                flag = 0;
            } else {
                // If no process has just finished and no new allocation is required, rotate the ready queue.
                if(!finished && !allocate){
                    update_reday_processes(ready_processes);
                }
                // allocated memory
                if (ready_processes->head->process->allocated_pages == ready_processes->head->process->pages || 
                        ready_processes->head->process->allocated_pages >= 4) {
                    if (evicted) {
                        evicted_frames = arrayToString(evicted_frames_array, count);
                        printf("%d,EVICTED,evicted-frames=[%s]\n", current_time, evicted_frames);
                        for (int i = 0; i < count; i++) {
                            evicted_frames_array[i] = -1;  // Reset the evicted frames array.
                        }
                        count = 0;
                        evicted = 0;
                    }
                    if (strcmp(ready_processes->head->process->process_name, cur_process) != 0) {
                        int memory_usage = page_calculate_usage(pages_capacity);
                        evicted_frames = arrayToString(ready_processes->head->process->memory_pages, ready_processes->head->process->allocated_pages);
                        printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[%s]\n", current_time, ready_processes->head->process->process_name, 
                                    ready_processes->head->process->remaining_time, memory_usage, evicted_frames);
                        // Record the current process
                        strcpy(cur_process, ready_processes->head->process->process_name);
                    }
                    
                    allocate = 0;
                    ready_processes->head->process->remaining_time -= quantum;
                }else{
                    // Calculate how many more pages are needed to fulfill the process's requirements.
                    need_pages = ready_processes->head->process->pages - ready_processes->head->process->allocated_pages;
                    if (pages_capacity >= need_pages) {  // If there are enough free pages, allocate them.
                        allocate_memory_pages_task4(need_pages, memory, ready_processes);
                        pages_capacity -= need_pages;
                    } else if (ready_processes->head->process->allocated_pages + pages_capacity >= 4) {
                        // If total free pages and allocated pages reach at least 4, allocate as many as possible.
                        allocate_memory_pages_task4(pages_capacity, memory, ready_processes);
                        pages_capacity = 0;
                    }else{
                        Node *current = ready_processes->head->next; // start from secode node of linked list
                        while (current != NULL) {
                            if (current->process->allocated_pages > 0) {
                                // Evict the first allocated page of the process.
                                memory[current->process->memory_pages[0]] = 0;
                                evicted_frames_array[count] = current->process->memory_pages[0];
                                count++;
                                // Shift remaining pages in the array.
                                for (int i = 0; i < current->process->allocated_pages - 1; i++) {
                                    current->process->memory_pages[i] = current->process->memory_pages[i + 1];
                                }
                                current->process->memory_pages[current->process->allocated_pages] = -1;
                                current->process->allocated_pages--;
                                pages_capacity++;
                                break; // Found it, print 1 and exit the loop
                            }
                            current = current->next;  // If allocated is not 1, continue looking for the next node
                        }
                        evicted = 1;  // Set the evicted flag to indicate that eviction has occurred.
                    }
                    allocate = 1;
                    continue;
                }
            }
        }
        current_time += quantum;
    }
    
    print_message(&total_turnaround_time, &total_processes, &total_overhead_time, &max_overhead_time, &makespan);
    
    free(wait_processes);
    free_ready_queue(ready_processes);
}


void allocate_memory_pages_task4(int pages_capacity, int memory[], Queue *ready_processes){
    int index = 0;
    for (int i = 0; i < pages_capacity;) {
        if (memory[index] == 0){
            // If the value of memory[index] is 0, set it to 1 and increment index
            memory[index] = 1;
            ready_processes->head->process->memory_pages[ready_processes->head->process->allocated_pages] = index;
            index++;
            i++;
            ready_processes->head->process->allocated_pages++;
        } else {
            // If the value of memory[index] is not 0, only index will be incremented
            index++;
        }
    }

}