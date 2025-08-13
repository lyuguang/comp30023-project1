#ifndef TASK_H
#define TASK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_PROCESS_NAME 8
#define MEMORY_SIZE 2048
#define PAGES_SIZE 4
#define MAX_PAGES 512


// Structure to represent a process
typedef struct {
    int arrival_time;                        // processes start time
    char process_name[MAX_PROCESS_NAME+1];   // record the process name
    int service_time;                        // total amount of CPU time, does not to change
    int remaining_time;                      // still need to complete time
    int memory_length;                       // memory requirement of process, KB
    int memory_start;
    int allocated;
    int memory_pages[MAX_PAGES];
    int pages;
    int allocated_pages;
} Process;

typedef struct {
    int start;
    int length;
} MemoryHole;

typedef struct Node{
    Process* process;
    MemoryHole* memoryHole;
    struct Node* next;
}Node;

typedef struct {
    Node* head;
    Node* tail;
    int total_capacity;
}Queue;


// Function to read processes from the input file
void task1(char *filename, int quantum);
void read_processes(const char *filename, Process **processes, int *num_processes);
Queue* initialize_ready_queue();
void initialize_const(int *proc_num, int *proc_remaining, int *current_time, int *flag, int *current_process_num,
                     int *total_turnaround_time, double *max_overhead_time, double *total_overhead_time, int *makespan);
void add_process_to_ready_queue_task1(Queue *ready_processes, Process *process, int *flag);
void remove_finished_process_task1(Queue *ready_processes, int current_time, int *makespan, 
                                    int *total_turnaround_time, double *total_overhead_time, double *max_overhead_time);
void update_running_process_task1(Queue *ready_processes, int current_time, int quantum, int *flag, char *cur_process);
void update_reday_processes(Queue *ready_processes);
void print_message(int *total_turnaround_time, int *total_processes, double *total_overhead_time, double *max_overhead_time, int *makespan);
void free_ready_queue(Queue *ready_processes);


int allocate_memory(Queue *memory, Process *process);
void connect_hole(Queue *memory);
void deallocate_memory(Queue *memory, Process *process);
void task2(char *filename, int quantum);
void remove_finished_process(Queue *ready_processes, Queue *memory, int current_time, int *makespan, 
                                int *total_turnaround_time, double *total_overhead_time, double *max_overhead_time, int *finished);
int calculate_memory_usage(Queue *memory);
void add_process_to_ready_queue(Queue *ready_processes, Queue *memory, Process *process, int *flag);


void task3(char *filename, int quantum);
char* arrayToString(int arr[], int pages);
void allocate_memory_pages_task3(int memory[], Queue *ready_processes);
void free_process_memory_pages_task3(Node *current, int memory[], int *pages_capacity);
int page_calculate_usage(int memory_capacity);


void task4(char *filename, int quantum);
void allocate_memory_pages_task4(int pages_capacity, int memory[], Queue *ready_processes);


#endif
