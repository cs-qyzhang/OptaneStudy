#ifndef LATTESTER_COMMON_H
#define LATTESTER_COMMON_H

// Kernel-level task lists
struct latency_task {
    int (*bench_func)(char *, long*);
    char* name;
    long skip;
};

extern struct latency_task latency_tasks[];

extern const int BASIC_OPS_TASK_COUNT;
extern long repeat_256byte_load(char *addr);
extern long repeat_256byte_ntstore(char *addr);

#endif