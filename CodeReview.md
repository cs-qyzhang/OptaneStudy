# Code Review

## Structure

This project implemented as kernel module to avoid context switch, disable IRQ and using `wbinvd` instruction to drop all caches.

Provided two kernel modules: `lat` and `rep`, which introduce two file systems. `rep` is used to store statistics.

When the `rep` is mounted, it will setup direct access to the corresponding device, so `lat` can access it to store evaluation results later.

When the `lat` is mounted, it will create the `/proc/lattester` entry, in the `write` op of this entry, `lat` will parse the input as command, then it will create a kernel thread to run corresponding benchmark.

The functions of tests is inside `src/kernel/tasks.c`, which is ended with `_job` suffix.

## Considerations on avoiding the impaction of benchmark

1. Running in kernel to pinned to core and avoid context switch (the kernel needs to be configured as no preempt)
2. Disable IRQ when running single test
3. Use `wbinvd` to drop caches
4. Fill page table before running test
5. Use asm code to precisely control the tested instructions
6. Disable hardware prefetching (this is done by using `wrmsr` in bash script, see `src/testscript/run.example.sh`)

## Performance counters

This module use `linux/perf_event.h` to access performance counters such as cache line access, see `src/kernel/perf_util.c`.

The data of performance counters is stored percpu by using `linux/percpu_defs.h`, see <https://0xax.gitbooks.io/linux-insides/content/Concepts/linux-cpu-1.html>.

## Random read

When running random read latency test, it uses `get_random_bytes()` which is provided by linux to generate random long integers, these random integers will be used as address offset.

## Configuration

1. The `CONFIG_PREEPTION` of kernel cannot be set (avoid kthread preempt)?
2. `DIMM_SIZE` definition in `src/kernel/lattester.h`, default 256G
