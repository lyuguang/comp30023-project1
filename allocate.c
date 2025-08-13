#include "task.h"

int main(int argc, char *argv[]) {
    // check if
    if (argc != 7 || strcmp(argv[1], "-f") != 0 || strcmp(argv[3], "-q") != 0 || strcmp(argv[5], "-m") != 0) {
        fprintf(stderr, "Usage: %s -f <filename> -q (1 | 2 | 3) -m (infinite | first-fit | paged | virtual)\n", argv[0]);
        return 1;
    }

    // Save command line parameters to variables
    char *filename = argv[2];
    int quantum = atoi(argv[4]);
    char *memory_strategy = argv[6];

    // Call different tasks according to memory_strategy
    if (strcmp(memory_strategy, "infinite") == 0) {
        task1(filename, quantum);
    } else if (strcmp(memory_strategy, "first-fit") == 0) {
        task2(filename, quantum);
    } else if (strcmp(memory_strategy, "paged") == 0) {
        task3(filename, quantum);
    } else if (strcmp(memory_strategy, "virtual") == 0) {
        task4(filename, quantum);
    } else {
        fprintf(stderr, "Invalid memory strategy\n");
        return 1;
    }

    return 0;
}