/* SPDX-License-Identifier: GPL-2.0 */
/*
 * BRIEF DESCRIPTION
 *
 * Header for commonly used defines
 *
 * Copyright 2019 Regents of the University of California,
 * UCSD Non-Volatile Systems Lab
 */
#include <linux/printk.h>
#include "common.h"

// #ifdef __KERNEL__
#if 0
  #include <asm/fpu/api.h>
  #define KERNEL_BEGIN \
	 kernel_fpu_begin();
  #define KERNEL_END \
	 kernel_fpu_end();
#else
  #define KERNEL_BEGIN do { } while (0);
  #define KERNEL_END do { } while (0);
#endif

/* Assembly instructions utilize the following registers:
 * rsi: memory address
 * rax, rdx, rcx, r8d and r9d: timing
 * rdx: populating cache-lines
 * ymm0: streaming instructions
 */
#define REGISTERS "rsi", "rax", "rdx", "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "ymm0"
#define REGISTERS_NONSSE "rsi", "rax", "rdx", "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"

/* ymm0: 256-bit register (requires AVX support)
 * vbroadcastsd: VEX.256-bit version (r[0] = r[1] = r[2] = r[3] = v)
 */
#define LOAD_VALUE              "vbroadcastsd %[value], %%ymm0 \n"
#define LOAD_ADDR               "mov %[memarea], %%rsi \n" \
                                "mfence \n"

#define FLUSH_CACHE_LINE        "clflush 0*32(%%rsi) \n" \
                                "clflush 2*32(%%rsi) \n" \
                                "clflush 4*32(%%rsi) \n" \
                                "clflush 6*32(%%rsi) \n" \
                                "mfence \n"

#define LOAD_CACHE_LINE         "movq 0*8(%%rsi), %%rdx \n" \
                                "movq 1*8(%%rsi), %%rdx \n" \
                                "movq 2*8(%%rsi), %%rdx \n" \
                                "movq 3*8(%%rsi), %%rdx \n" \
                                "movq 4*8(%%rsi), %%rdx \n" \
                                "movq 5*8(%%rsi), %%rdx \n" \
                                "movq 6*8(%%rsi), %%rdx \n" \
                                "movq 7*8(%%rsi), %%rdx \n" \
                                "movq 8*8(%%rsi), %%rdx \n" \
                                "movq 9*8(%%rsi), %%rdx \n" \
                                "movq 10*8(%%rsi), %%rdx \n" \
                                "movq 11*8(%%rsi), %%rdx \n" \
                                "movq 12*8(%%rsi), %%rdx \n" \
                                "movq 13*8(%%rsi), %%rdx \n" \
                                "movq 14*8(%%rsi), %%rdx \n" \
                                "movq 15*8(%%rsi), %%rdx \n" \
                                "movq 16*8(%%rsi), %%rdx \n" \
                                "movq 17*8(%%rsi), %%rdx \n" \
                                "movq 18*8(%%rsi), %%rdx \n" \
                                "movq 19*8(%%rsi), %%rdx \n" \
                                "movq 20*8(%%rsi), %%rdx \n" \
                                "movq 21*8(%%rsi), %%rdx \n" \
                                "movq 22*8(%%rsi), %%rdx \n" \
                                "movq 23*8(%%rsi), %%rdx \n" \
                                "movq 24*8(%%rsi), %%rdx \n" \
                                "movq 25*8(%%rsi), %%rdx \n" \
                                "movq 26*8(%%rsi), %%rdx \n" \
                                "movq 27*8(%%rsi), %%rdx \n" \
                                "movq 28*8(%%rsi), %%rdx \n" \
                                "movq 29*8(%%rsi), %%rdx \n" \
                                "movq 30*8(%%rsi), %%rdx \n" \
                                "movq 31*8(%%rsi), %%rdx \n" \
                                "mfence \n"

#define CLEAR_PIPELINE          "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n"

/* rdtscp: reads current timestamp to EDX:EAX and also sets ECX
 * higher 32-bits of RAX, RDX and RCX are cleared
 */

// see https://stackoverflow.com/a/58146426/7640227
#define TIMING_BEG              "mfence \n " \
                                "lfence \n " \
                                "rdtsc \n" \
                                "mov %%edx, %%r9d \n" \
                                "mov %%eax, %%r8d \n"

/* r9d = old EDX
 * r8d = old EAX
 * Here is what we do to compute t1 and t2:
 * - RDX holds t2
 * - RAX holds t1
 */
#define TIMING_END              "rdtscp \n" \
                                "lfence \n" \
                                "shl $32, %%rdx \n" \
                                "or  %%rax, %%rdx \n" \
                                "mov %%r9d, %%eax \n" \
                                "shl $32, %%rax \n" \
                                "or  %%r8, %%rax \n" \
                                "mov %%rax, %[t1] \n" \
                                "mov %%rdx, %[t2] \n"

#define MFENCE                  "mfence \n"
#define LFENCE                  "lfence \n"
#define SFENCE                  "sfence \n"

int baseline(char *addr, long* results) {
    long t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(CLEAR_PIPELINE
                 TIMING_BEG
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 :: REGISTERS_NONSSE);
    KERNEL_END
    results[0] = t2 - t1;
    return 1;
}

// we need to use mfence instead of sfence here,
// if we use sfence, the rdtscp will be reordered with
// store, which will cause a very high performance store
#define store_help(name, instruction) \
    int store_##name(char* addr, long* results) { \
        long t1 = 0, t2 = 0; \
        long value = 0xC0FFEEEEBABE0000; \
        KERNEL_BEGIN \
        asm volatile(LOAD_VALUE \
                     LOAD_ADDR \
                     LOAD_CACHE_LINE \
                     CLEAR_PIPELINE \
                     TIMING_BEG \
                     instruction \
                     MFENCE \
                     TIMING_END \
                     : [t1] "=r" (t1), [t2] "=r" (t2) \
                     : [memarea] "r" (addr), [value] "m" (value) \
                     : REGISTERS); \
        KERNEL_END \
        results[0] = t2 - t1; \
        return 1; \
    } \
    int store_##name##_no_prefetch(char* addr, long* results) { \
        long t1 = 0, t2 = 0; \
        long value = 0xC0FFEEEEBABE0000; \
        KERNEL_BEGIN \
        /* vmovdqa: 32-byte store to memory */ \
        asm volatile(LOAD_VALUE \
                     LOAD_ADDR \
                     "movq %%rsi, %%r10\n" \
                     "and $0xffffffffffffffc0, %%r10\n" \
                     CLEAR_PIPELINE \
                     TIMING_BEG \
                     instruction \
                     MFENCE \
                     TIMING_END \
                     : [t1] "=r" (t1), [t2] "=r" (t2) \
                     : [memarea] "r" (addr), [value] "m" (value) \
                     : REGISTERS); \
        KERNEL_END \
        results[0] = t2 - t1; \
        return 1; \
    }

#define load_help(name, instructions) \
    int load_##name(char *addr, long* results) { \
        long t1 = 0, t2 = 0; \
        KERNEL_BEGIN \
        asm volatile(LOAD_ADDR \
                     FLUSH_CACHE_LINE \
                     CLEAR_PIPELINE \
                     TIMING_BEG \
                     instructions \
                     LFENCE \
                     TIMING_END \
                     : [t1] "=r" (t1), [t2] "=r" (t2) \
                     : [memarea] "r" (addr) \
                     : REGISTERS); \
        KERNEL_END \
        results[0] = t2 - t1; \
        return 1; \
    }

/***************** Latency Tasks *****************/
store_help(
    4,
    "mov %%eax, 0*32(%%rsi) \n"
)

store_help(
    8,
    "movq %%rax, 0*8(%%rsi) \n"
)

#define REC_TIME(reg)   "mfence \n " \
                        "rdtscp \n" \
                        "lfence \n " \
                        "shl $32, %%rdx \n" \
                        "or  %%rax, %%rdx \n" \
                        "mov %%rdx, %"#reg" \n"

int read_8_clwb(char* addr, long* result) {
    long t1, t2, t3, t4, t5, t6, t7, t8, t9;
    long value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /* vmovdqa: 32-byte store to memory */
    asm volatile(LOAD_VALUE
                    LOAD_ADDR
                    CLEAR_PIPELINE
                    REC_TIME(%r8)
                    "movq 0*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r9)
                    "movq 1*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r10)
                    "movq 2*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r11)
                    "movq 3*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r12)
                    "movq 4*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r13)
                    "movq 5*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r14)
                    "movq 6*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r15)
                    "movq 7*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME([t9])
                    "mov %%r8, %[t1] \n"
                    "mov %%r9, %[t2] \n"
                    "mov %%r10, %[t3] \n"
                    "mov %%r11, %[t4] \n"
                    "mov %%r12, %[t5] \n"
                    "mov %%r13, %[t6] \n"
                    "mov %%r14, %[t7] \n"
                    "mov %%r15, %[t8] \n"
                    : [t1] "=m" (t1), [t2] "=m" (t2), [t3] "=m" (t3), [t4] "=m" (t4), [t5] "=m" (t5), [t6] "=m" (t6), [t7] "=m" (t7), [t8] "=m" (t8), [t9] "=m" (t9)
                    : [memarea] "r" (addr), [value] "m" (value)
                    : REGISTERS);
    KERNEL_END
    result[0] = t2 - t1;
    result[1] = t3 - t2;
    result[2] = t4 - t3;
    result[3] = t5 - t4;
    result[4] = t6 - t5;
    result[5] = t7 - t6;
    result[6] = t8 - t7;
    result[7] = t9 - t8;
    return 8;
}

int store_8_clwb_read_1(char* addr, long* result) {
    long t1, t2, t3;
    long value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /* vmovdqa: 32-byte store to memory */
    asm volatile(LOAD_VALUE
                    LOAD_ADDR
                    CLEAR_PIPELINE
                    REC_TIME(%r8)
                    "movq %%rax, 0*32(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r9)
                    "movq 1*32(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r10)
                    "mov %%r8, %[t1] \n"
                    "mov %%r9, %[t2] \n"
                    "mov %%r10, %[t3] \n"
                    : [t1] "=m" (t1), [t2] "=m" (t2), [t3] "=m" (t3)
                    : [memarea] "r" (addr), [value] "m" (value)
                    : REGISTERS);
    KERNEL_END
    result[0] = t2 - t1;
    result[1] = t3 - t2;
    return 2;
}

int store_8_clwb_read(char* addr, long* result) {
    long t1, t2, t3, t4, t5, t6, t7, t8, t9;
    long value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /* vmovdqa: 32-byte store to memory */
    asm volatile(LOAD_VALUE
                    LOAD_ADDR
                    CLEAR_PIPELINE
                    REC_TIME(%r8)
                    "movq %%rax, 0*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r9)
                    "movq 1*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r10)
                    "movq 2*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r11)
                    "movq 3*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r12)
                    "movq 4*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r13)
                    "movq 5*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r14)
                    "movq 6*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r15)
                    "movq 7*8(%%rsi), %%rax \n"
                    "clwb (%%rsi) \n"
                    REC_TIME([t9])
                    "mov %%r8, %[t1] \n"
                    "mov %%r9, %[t2] \n"
                    "mov %%r10, %[t3] \n"
                    "mov %%r11, %[t4] \n"
                    "mov %%r12, %[t5] \n"
                    "mov %%r13, %[t6] \n"
                    "mov %%r14, %[t7] \n"
                    "mov %%r15, %[t8] \n"
                    : [t1] "=m" (t1), [t2] "=m" (t2), [t3] "=m" (t3), [t4] "=m" (t4), [t5] "=m" (t5), [t6] "=m" (t6), [t7] "=m" (t7), [t8] "=m" (t8), [t9] "=m" (t9)
                    : [memarea] "r" (addr), [value] "m" (value)
                    : REGISTERS);
    KERNEL_END
    result[0] = t2 - t1;
    result[1] = t3 - t2;
    result[2] = t4 - t3;
    result[3] = t5 - t4;
    result[4] = t6 - t5;
    result[5] = t7 - t6;
    result[6] = t8 - t7;
    result[7] = t9 - t8;
    return 8;
}

int store_8_skip_8_no_prefetch(char* addr, long* result) {
    long t1, t2, t3, t4, t5, t6, t7, t8, t9;
    long value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /* vmovdqa: 32-byte store to memory */
    asm volatile(LOAD_VALUE
                    LOAD_ADDR
                    CLEAR_PIPELINE
                    REC_TIME(%r8)
                    "movq %%rax, 0*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r9)
                    "movq %%rax, 1*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r10)
                    "movq %%rax, 2*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r11)
                    "movq %%rax, 3*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r12)
                    "movq %%rax, 4*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r13)
                    "movq %%rax, 5*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r14)
                    "movq %%rax, 6*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r15)
                    "movq %%rax, 7*8(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME([t9])
                    "mov %%r8, %[t1] \n"
                    "mov %%r9, %[t2] \n"
                    "mov %%r10, %[t3] \n"
                    "mov %%r11, %[t4] \n"
                    "mov %%r12, %[t5] \n"
                    "mov %%r13, %[t6] \n"
                    "mov %%r14, %[t7] \n"
                    "mov %%r15, %[t8] \n"
                    : [t1] "=m" (t1), [t2] "=m" (t2), [t3] "=m" (t3), [t4] "=m" (t4), [t5] "=m" (t5), [t6] "=m" (t6), [t7] "=m" (t7), [t8] "=m" (t8), [t9] "=m" (t9)
                    : [memarea] "r" (addr), [value] "m" (value)
                    : REGISTERS);
    KERNEL_END
    result[0] = t2 - t1;
    result[1] = t3 - t2;
    result[2] = t4 - t3;
    result[3] = t5 - t4;
    result[4] = t6 - t5;
    result[5] = t7 - t6;
    result[6] = t8 - t7;
    result[7] = t9 - t8;
    return 8;
}

int store_8_skip_16_no_prefetch(char* addr, long* result) {
    long t1, t2, t3, t4, t5;
    long value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /* vmovdqa: 32-byte store to memory */
    asm volatile(LOAD_VALUE
                    LOAD_ADDR
                    CLEAR_PIPELINE
                    REC_TIME(%r8)
                    "movq %%rax, 0*16(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r9)
                    "movq %%rax, 1*16(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r10)
                    "movq %%rax, 2*16(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r11)
                    "movq %%rax, 3*16(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r12)
                    "mov %%r8, %[t1] \n"
                    "mov %%r9, %[t2] \n"
                    "mov %%r10, %[t3] \n"
                    "mov %%r11, %[t4] \n"
                    "mov %%r12, %[t5] \n"
                    : [t1] "=m" (t1), [t2] "=m" (t2), [t3] "=m" (t3), [t4] "=m" (t4), [t5] "=m" (t5)
                    : [memarea] "r" (addr), [value] "m" (value)
                    : REGISTERS);
    KERNEL_END
    result[0] = t2 - t1;
    result[1] = t3 - t2;
    result[2] = t4 - t3;
    result[3] = t5 - t4;
    return 4;
}

int store_8_skip_32_no_prefetch(char* addr, long* result) {
    long t1, t2, t3;
    long value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /* vmovdqa: 32-byte store to memory */
    asm volatile(LOAD_VALUE
                    LOAD_ADDR
                    CLEAR_PIPELINE
                    REC_TIME(%r8)
                    "movq %%rax, 0*32(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r9)
                    "movq %%rax, 1*32(%%rsi) \n"
                    "clwb (%%rsi) \n"
                    REC_TIME(%r10)
                    "mov %%r8, %[t1] \n"
                    "mov %%r9, %[t2] \n"
                    "mov %%r10, %[t3] \n"
                    : [t1] "=m" (t1), [t2] "=m" (t2), [t3] "=m" (t3)
                    : [memarea] "r" (addr), [value] "m" (value)
                    : REGISTERS);
    KERNEL_END
    result[0] = t2 - t1;
    result[1] = t3 - t2;
    return 2;
}

store_help(
    8_skip_8_clwb,
    "movq %%rax, 0*8(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 1*8(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 2*8(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 3*8(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 4*8(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 5*8(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 6*8(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 7*8(%%rsi) \n"
    "clwb (%%r10) \n"
)

store_help(
    8_skip_16_clwb,
    "movq %%rax, 0*16(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 1*16(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 2*16(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 3*16(%%rsi) \n"
    "clwb (%%r10) \n"
)

store_help(
    8_skip_32_clwb,
    "movq %%rax, 0*32(%%rsi) \n"
    "clwb (%%r10) \n"
    "movq %%rax, 1*32(%%rsi) \n"
    "clwb (%%r10) \n"
)

store_help(
    16,
    "movq %%rax, 0*8(%%rsi) \n"
    "movq %%rax, 1*8(%%rsi) \n"
)

store_help(
    32,
    "movq %%rax, 0*8(%%rsi) \n"
    "movq %%rax, 1*8(%%rsi) \n"
    "movq %%rax, 2*8(%%rsi) \n"
    "movq %%rax, 3*8(%%rsi) \n"
)

store_help(
    4_clwb,
    "mov %%eax, 0*32(%%rsi) \n"
    "clwb (%%r10) \n"
)

store_help(
    8_clwb,
    "movq %%rax, 0*32(%%rsi) \n"
    "clwb (%%r10) \n"
)

store_help(
    16_clwb,
    "movq %%rax, 0*8(%%rsi) \n"
    "movq %%rax, 1*8(%%rsi) \n"
    "clwb (%%r10) \n"
)

store_help(
    32_clwb,
    "movq %%rax, 0*8(%%rsi) \n"
    "movq %%rax, 1*8(%%rsi) \n"
    "movq %%rax, 2*8(%%rsi) \n"
    "movq %%rax, 3*8(%%rsi) \n"
    "clwb (%%r10) \n"
)

store_help(
    64,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
)

store_help(
    64_clflush,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clflush (%%rsi) \n"
)

store_help(
    64_clwb,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clwb (%%rsi) \n"
)

store_help(
    64_clflushopt,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clflushopt (%%rsi) \n"
)

store_help(
    64_nt,
    "vmovntpd %%ymm0, 0*32(%%rsi) \n"
    "vmovntpd %%ymm0, 1*32(%%rsi) \n"
)

load_help(
    64,
    "vmovdqa 0*32(%%rsi), %%ymm0 \n"
    "vmovdqa 1*32(%%rsi), %%ymm1 \n"
)

load_help(
    64_nt,
    "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
    "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
)

store_help(
    64_movq,
    "movq %%rdx, 0*8(%%rsi) \n"
    "movq %%rdx, 1*8(%%rsi) \n"
    "movq %%rdx, 2*8(%%rsi) \n"
    "movq %%rdx, 3*8(%%rsi) \n"
    "movq %%rdx, 4*8(%%rsi) \n"
    "movq %%rdx, 5*8(%%rsi) \n"
    "movq %%rdx, 6*8(%%rsi) \n"
    "movq %%rdx, 7*8(%%rsi) \n"
)

store_help(
    64_clflush_movq,
    "movq %%rdx, 0*8(%%rsi) \n"
    "movq %%rdx, 1*8(%%rsi) \n"
    "movq %%rdx, 2*8(%%rsi) \n"
    "movq %%rdx, 3*8(%%rsi) \n"
    "movq %%rdx, 4*8(%%rsi) \n"
    "movq %%rdx, 5*8(%%rsi) \n"
    "movq %%rdx, 6*8(%%rsi) \n"
    "movq %%rdx, 7*8(%%rsi) \n"
    "clflush (%%rsi) \n"
)

load_help(
    64_movq,
    "movq 0*8(%%rsi), %%rdx \n"
    "movq 1*8(%%rsi), %%rdx \n"
    "movq 2*8(%%rsi), %%rdx \n"
    "movq 3*8(%%rsi), %%rdx \n"
    "movq 4*8(%%rsi), %%rdx \n"
    "movq 5*8(%%rsi), %%rdx \n"
    "movq 6*8(%%rsi), %%rdx \n"
    "movq 7*8(%%rsi), %%rdx \n"
)

store_help(
    128,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "vmovdqa %%ymm0, 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 3*32(%%rsi) \n"
)

store_help(
    128_clflush,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clflush (%%rsi) \n"
    "vmovdqa %%ymm0, 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 3*32(%%rsi) \n"
    "clflush 2*32(%%rsi) \n"
)

store_help(
    128_clwb,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clwb (%%rsi) \n"
    "vmovdqa %%ymm0, 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 3*32(%%rsi) \n"
    "clwb 2*32(%%rsi) \n"
)

store_help(
    128_clflushopt,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clflushopt (%%rsi) \n"
    "vmovdqa %%ymm0, 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 3*32(%%rsi) \n"
    "clflushopt 2*32(%%rsi) \n"
)

store_help(
    128_nt,
    "vmovntpd %%ymm0, 0*32(%%rsi) \n"
    "vmovntpd %%ymm0, 1*32(%%rsi) \n"
    "vmovntpd %%ymm0, 2*32(%%rsi) \n"
    "vmovntpd %%ymm0, 3*32(%%rsi) \n"
)

load_help(
    128,
    "vmovdqa 0*32(%%rsi), %%ymm0 \n"
    "vmovdqa 1*32(%%rsi), %%ymm1 \n"
    "vmovdqa 2*32(%%rsi), %%ymm1 \n"
    "vmovdqa 3*32(%%rsi), %%ymm1 \n"
)

load_help(
    128_nt,
    "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
    "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
    "vmovntdqa 2*32(%%rsi), %%ymm1 \n"
    "vmovntdqa 3*32(%%rsi), %%ymm1 \n"
)

store_help(
    128_movq,
    "movq %%rdx, 0*8(%%rsi) \n"
    "movq %%rdx, 1*8(%%rsi) \n"
    "movq %%rdx, 2*8(%%rsi) \n"
    "movq %%rdx, 3*8(%%rsi) \n"
    "movq %%rdx, 4*8(%%rsi) \n"
    "movq %%rdx, 5*8(%%rsi) \n"
    "movq %%rdx, 6*8(%%rsi) \n"
    "movq %%rdx, 7*8(%%rsi) \n"
    "movq %%rdx, 8*8(%%rsi) \n"
    "movq %%rdx, 9*8(%%rsi) \n"
    "movq %%rdx, 10*8(%%rsi) \n"
    "movq %%rdx, 11*8(%%rsi) \n"
    "movq %%rdx, 12*8(%%rsi) \n"
    "movq %%rdx, 13*8(%%rsi) \n"
    "movq %%rdx, 14*8(%%rsi) \n"
    "movq %%rdx, 15*8(%%rsi) \n"
)

store_help(
    128_clflush_movq,
    "movq %%rdx, 0*8(%%rsi) \n"
    "movq %%rdx, 1*8(%%rsi) \n"
    "movq %%rdx, 2*8(%%rsi) \n"
    "movq %%rdx, 3*8(%%rsi) \n"
    "movq %%rdx, 4*8(%%rsi) \n"
    "movq %%rdx, 5*8(%%rsi) \n"
    "movq %%rdx, 6*8(%%rsi) \n"
    "movq %%rdx, 7*8(%%rsi) \n"
    "clflush (%%rsi) \n"
    "movq %%rdx, 8*8(%%rsi) \n"
    "movq %%rdx, 9*8(%%rsi) \n"
    "movq %%rdx, 10*8(%%rsi) \n"
    "movq %%rdx, 11*8(%%rsi) \n"
    "movq %%rdx, 12*8(%%rsi) \n"
    "movq %%rdx, 13*8(%%rsi) \n"
    "movq %%rdx, 14*8(%%rsi) \n"
    "movq %%rdx, 15*8(%%rsi) \n"
    "clflush 8*8(%%rsi) \n"
)

load_help(
    128_movq,
    "movq 0*8(%%rsi), %%rdx \n"
    "movq 1*8(%%rsi), %%rdx \n"
    "movq 2*8(%%rsi), %%rdx \n"
    "movq 3*8(%%rsi), %%rdx \n"
    "movq 4*8(%%rsi), %%rdx \n"
    "movq 5*8(%%rsi), %%rdx \n"
    "movq 6*8(%%rsi), %%rdx \n"
    "movq 7*8(%%rsi), %%rdx \n"
    "movq 8*8(%%rsi), %%rdx \n"
    "movq 9*8(%%rsi), %%rdx \n"
    "movq 10*8(%%rsi), %%rdx \n"
    "movq 11*8(%%rsi), %%rdx \n"
    "movq 12*8(%%rsi), %%rdx \n"
    "movq 13*8(%%rsi), %%rdx \n"
    "movq 14*8(%%rsi), %%rdx \n"
    "movq 15*8(%%rsi), %%rdx \n"
)

/*
 * 256- benchmarks
 */
store_help(
    256,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "vmovdqa %%ymm0, 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 3*32(%%rsi) \n"
    "vmovdqa %%ymm0, 4*32(%%rsi) \n"
    "vmovdqa %%ymm0, 5*32(%%rsi) \n"
    "vmovdqa %%ymm0, 6*32(%%rsi) \n"
    "vmovdqa %%ymm0, 7*32(%%rsi) \n"
)

store_help(
    256_clflush,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clflush 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 3*32(%%rsi) \n"
    "clflush 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 4*32(%%rsi) \n"
    "vmovdqa %%ymm0, 5*32(%%rsi) \n"
    "clflush 4*32(%%rsi) \n"
    "vmovdqa %%ymm0, 6*32(%%rsi) \n"
    "vmovdqa %%ymm0, 7*32(%%rsi) \n"
    "clflush 6*32(%%rsi) \n"
)

store_help(
    256_clwb,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clwb 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 3*32(%%rsi) \n"
    "clwb 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 4*32(%%rsi) \n"
    "vmovdqa %%ymm0, 5*32(%%rsi) \n"
    "clwb 4*32(%%rsi) \n"
    "vmovdqa %%ymm0, 6*32(%%rsi) \n"
    "vmovdqa %%ymm0, 7*32(%%rsi) \n"
    "clwb 6*32(%%rsi) \n"
)

store_help(
    256_clflushopt,
    "vmovdqa %%ymm0, 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 1*32(%%rsi) \n"
    "clflushopt 0*32(%%rsi) \n"
    "vmovdqa %%ymm0, 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 3*32(%%rsi) \n"
    "clflushopt 2*32(%%rsi) \n"
    "vmovdqa %%ymm0, 4*32(%%rsi) \n"
    "vmovdqa %%ymm0, 5*32(%%rsi) \n"
    "clflushopt 4*32(%%rsi) \n"
    "vmovdqa %%ymm0, 6*32(%%rsi) \n"
    "vmovdqa %%ymm0, 7*32(%%rsi) \n"
    "clflushopt 6*32(%%rsi) \n"
)

store_help(
    256_nt,
    "vmovntpd %%ymm0, 0*32(%%rsi) \n"
    "vmovntpd %%ymm0, 1*32(%%rsi) \n"
    "vmovntpd %%ymm0, 2*32(%%rsi) \n"
    "vmovntpd %%ymm0, 3*32(%%rsi) \n"
    "vmovntpd %%ymm0, 4*32(%%rsi) \n"
    "vmovntpd %%ymm0, 5*32(%%rsi) \n"
    "vmovntpd %%ymm0, 6*32(%%rsi) \n"
    "vmovntpd %%ymm0, 7*32(%%rsi) \n"
)

load_help(
    256,
    "vmovdqa 0*32(%%rsi), %%ymm0 \n"
    "vmovdqa 1*32(%%rsi), %%ymm1 \n"
    "vmovdqa 2*32(%%rsi), %%ymm1 \n"
    "vmovdqa 3*32(%%rsi), %%ymm1 \n"
    "vmovdqa 4*32(%%rsi), %%ymm1 \n"
    "vmovdqa 5*32(%%rsi), %%ymm1 \n"
    "vmovdqa 6*32(%%rsi), %%ymm1 \n"
    "vmovdqa 7*32(%%rsi), %%ymm1 \n"
)

load_help(
    256_nt,
    "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
    "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
    "vmovntdqa 2*32(%%rsi), %%ymm1 \n"
    "vmovntdqa 3*32(%%rsi), %%ymm1 \n"
    "vmovntdqa 4*32(%%rsi), %%ymm1 \n"
    "vmovntdqa 5*32(%%rsi), %%ymm1 \n"
    "vmovntdqa 6*32(%%rsi), %%ymm1 \n"
    "vmovntdqa 7*32(%%rsi), %%ymm1 \n"
)

store_help(
    256_movq,
    "movq %%rdx, 0*8(%%rsi) \n"
    "movq %%rdx, 1*8(%%rsi) \n"
    "movq %%rdx, 2*8(%%rsi) \n"
    "movq %%rdx, 3*8(%%rsi) \n"
    "movq %%rdx, 4*8(%%rsi) \n"
    "movq %%rdx, 5*8(%%rsi) \n"
    "movq %%rdx, 6*8(%%rsi) \n"
    "movq %%rdx, 7*8(%%rsi) \n"
    "movq %%rdx, 8*8(%%rsi) \n"
    "movq %%rdx, 9*8(%%rsi) \n"
    "movq %%rdx, 10*8(%%rsi) \n"
    "movq %%rdx, 11*8(%%rsi) \n"
    "movq %%rdx, 12*8(%%rsi) \n"
    "movq %%rdx, 13*8(%%rsi) \n"
    "movq %%rdx, 14*8(%%rsi) \n"
    "movq %%rdx, 15*8(%%rsi) \n"
    "movq %%rdx, 16*8(%%rsi) \n"
    "movq %%rdx, 17*8(%%rsi) \n"
    "movq %%rdx, 18*8(%%rsi) \n"
    "movq %%rdx, 19*8(%%rsi) \n"
    "movq %%rdx, 20*8(%%rsi) \n"
    "movq %%rdx, 21*8(%%rsi) \n"
    "movq %%rdx, 22*8(%%rsi) \n"
    "movq %%rdx, 23*8(%%rsi) \n"
    "movq %%rdx, 24*8(%%rsi) \n"
    "movq %%rdx, 25*8(%%rsi) \n"
    "movq %%rdx, 26*8(%%rsi) \n"
    "movq %%rdx, 27*8(%%rsi) \n"
    "movq %%rdx, 28*8(%%rsi) \n"
    "movq %%rdx, 29*8(%%rsi) \n"
    "movq %%rdx, 30*8(%%rsi) \n"
    "movq %%rdx, 31*8(%%rsi) \n"
)

store_help(
    256_clflush_movq,
    "movq %%rdx, 0*8(%%rsi) \n"
    "movq %%rdx, 1*8(%%rsi) \n"
    "movq %%rdx, 2*8(%%rsi) \n"
    "movq %%rdx, 3*8(%%rsi) \n"
    "movq %%rdx, 4*8(%%rsi) \n"
    "movq %%rdx, 5*8(%%rsi) \n"
    "movq %%rdx, 6*8(%%rsi) \n"
    "movq %%rdx, 7*8(%%rsi) \n"
    "clflush 0*8(%%rsi) \n"
    "movq %%rdx, 8*8(%%rsi) \n"
    "movq %%rdx, 9*8(%%rsi) \n"
    "movq %%rdx, 10*8(%%rsi) \n"
    "movq %%rdx, 11*8(%%rsi) \n"
    "movq %%rdx, 12*8(%%rsi) \n"
    "movq %%rdx, 13*8(%%rsi) \n"
    "movq %%rdx, 14*8(%%rsi) \n"
    "movq %%rdx, 15*8(%%rsi) \n"
    "clflush 8*8(%%rsi) \n"
    "movq %%rdx, 16*8(%%rsi) \n"
    "movq %%rdx, 17*8(%%rsi) \n"
    "movq %%rdx, 18*8(%%rsi) \n"
    "movq %%rdx, 19*8(%%rsi) \n"
    "movq %%rdx, 20*8(%%rsi) \n"
    "movq %%rdx, 21*8(%%rsi) \n"
    "movq %%rdx, 22*8(%%rsi) \n"
    "movq %%rdx, 23*8(%%rsi) \n"
    "clflush 16*8(%%rsi) \n"
    "movq %%rdx, 24*8(%%rsi) \n"
    "movq %%rdx, 25*8(%%rsi) \n"
    "movq %%rdx, 26*8(%%rsi) \n"
    "movq %%rdx, 27*8(%%rsi) \n"
    "movq %%rdx, 28*8(%%rsi) \n"
    "movq %%rdx, 29*8(%%rsi) \n"
    "movq %%rdx, 30*8(%%rsi) \n"
    "movq %%rdx, 31*8(%%rsi) \n"
    "clflush 24*8(%%rsi) \n"
)

load_help(
    256_movq,
    "movq 0*8(%%rsi), %%rdx \n"
    "movq 1*8(%%rsi), %%rdx \n"
    "movq 2*8(%%rsi), %%rdx \n"
    "movq 3*8(%%rsi), %%rdx \n"
    "movq 4*8(%%rsi), %%rdx \n"
    "movq 5*8(%%rsi), %%rdx \n"
    "movq 6*8(%%rsi), %%rdx \n"
    "movq 7*8(%%rsi), %%rdx \n"
    "movq 8*8(%%rsi), %%rdx \n"
    "movq 9*8(%%rsi), %%rdx \n"
    "movq 10*8(%%rsi), %%rdx \n"
    "movq 11*8(%%rsi), %%rdx \n"
    "movq 12*8(%%rsi), %%rdx \n"
    "movq 13*8(%%rsi), %%rdx \n"
    "movq 14*8(%%rsi), %%rdx \n"
    "movq 15*8(%%rsi), %%rdx \n"
    "movq 16*8(%%rsi), %%rdx \n"
    "movq 17*8(%%rsi), %%rdx \n"
    "movq 18*8(%%rsi), %%rdx \n"
    "movq 19*8(%%rsi), %%rdx \n"
    "movq 20*8(%%rsi), %%rdx \n"
    "movq 21*8(%%rsi), %%rdx \n"
    "movq 22*8(%%rsi), %%rdx \n"
    "movq 23*8(%%rsi), %%rdx \n"
    "movq 24*8(%%rsi), %%rdx \n"
    "movq 25*8(%%rsi), %%rdx \n"
    "movq 26*8(%%rsi), %%rdx \n"
    "movq 27*8(%%rsi), %%rdx \n"
    "movq 28*8(%%rsi), %%rdx \n"
    "movq 29*8(%%rsi), %%rdx \n"
    "movq 30*8(%%rsi), %%rdx \n"
    "movq 31*8(%%rsi), %%rdx \n"
)

struct latency_task latency_tasks[] = {
    {.bench_func = baseline,                    .skip = 0,   .name = "baseline"},

    {.bench_func = read_8_clwb,                 .skip = 0, .name = "read-8-advance-0"},
    {.bench_func = store_8_clwb_read,           .skip = 0, .name = "store-1-read-7-advance-0"},
    {.bench_func = store_8_clwb_read_1,         .skip = 0, .name = "store-1-read-1-advance-0"},
    {.bench_func = store_8_skip_8_no_prefetch,  .skip = 0, .name = "store-skip-8-advance-0"},
    {.bench_func = store_8_skip_16_no_prefetch, .skip = 0, .name = "store-skip-16-advance-0"},
    {.bench_func = store_8_skip_32_no_prefetch, .skip = 0, .name = "store-skip-32-advance-0"},
    {.bench_func = store_8_clwb_no_prefetch,    .skip = 0, .name = "store-skip-64-advance-0"},
    {.bench_func = read_8_clwb,                 .skip = 64, .name = "read-8-advance-64"},
    {.bench_func = store_8_clwb_read,           .skip = 64, .name = "store-1-read-7-advance-64"},
    {.bench_func = store_8_clwb_read_1,         .skip = 64, .name = "store-1-read-1-advance-64"},
    {.bench_func = store_8_skip_8_no_prefetch,  .skip = 64, .name = "store-skip-8-advance-64"},
    {.bench_func = store_8_skip_16_no_prefetch, .skip = 64, .name = "store-skip-16-advance-64"},
    {.bench_func = store_8_skip_32_no_prefetch, .skip = 64, .name = "store-skip-32-advance-64"},
    {.bench_func = store_8_clwb_no_prefetch,    .skip = 64, .name = "store-skip-64-advance-64"},
    {.bench_func = read_8_clwb,                 .skip = 128, .name = "read-8-advance-128"},
    {.bench_func = store_8_clwb_read,           .skip = 128, .name = "store-1-read-7-advance-128"},
    {.bench_func = store_8_clwb_read_1,         .skip = 128, .name = "store-1-read-1-advance-128"},
    {.bench_func = store_8_skip_8_no_prefetch,  .skip = 128, .name = "store-skip-8-advance-128"},
    {.bench_func = store_8_skip_16_no_prefetch, .skip = 128, .name = "store-skip-16-advance-128"},
    {.bench_func = store_8_skip_32_no_prefetch, .skip = 128, .name = "store-skip-32-advance-128"},
    {.bench_func = store_8_clwb_no_prefetch,    .skip = 128, .name = "store-skip-64-advance-128"},
};

/*
struct latency_task latency_tasks_all[] = {
    {.bench_func = baseline,                    .skip = 0,   .name = "baseline"},

    {.bench_func = store_8_clwb_no_prefetch,    .skip = 8,   .name = "store-8-clwb-no-prefetch-skip-8"},
    {.bench_func = store_8_clwb_no_prefetch,    .skip = 16,   .name = "store-8-clwb-no-prefetch-skip-16"},
    {.bench_func = store_8_clwb_no_prefetch,    .skip = 32,   .name = "store-8-clwb-no-prefetch-skip-32"},
    {.bench_func = store_8_clwb_no_prefetch,    .skip = 64,   .name = "store-8-clwb-no-prefetch-skip-64"},
    {.bench_func = store_8_no_prefetch,         .skip = 8,   .name = "store-8-no-prefetch-skip-8"},
    {.bench_func = store_8_no_prefetch,         .skip = 16,  .name = "store-8-no-prefetch-skip-16"},
    {.bench_func = store_8_no_prefetch,         .skip = 32,  .name = "store-8-no-prefetch-skip-32"},
    {.bench_func = store_8_no_prefetch,         .skip = 64,  .name = "store-8-no-prefetch-skip-64"},
    {.bench_func = load_64,                     .skip = 64,  .name = "load-64"},
    {.bench_func = load_64_movq,                .skip = 64,  .name = "load-movq-64"},
    {.bench_func = load_64_nt,                  .skip = 64,  .name = "load-nt-64"},
    {.bench_func = store_64,                    .skip = 64,  .name = "store-64"},
    {.bench_func = store_64_movq,               .skip = 64,  .name = "store-movq-64"},
    {.bench_func = store_64_nt,                 .skip = 64,  .name = "store-nt-64"},
    {.bench_func = store_64_nt_no_prefetch,     .skip = 64,  .name = "store-nt-no-prefetch-64"},
    {.bench_func = store_64_clflush,            .skip = 64,  .name = "store-clflush-64"},
    {.bench_func = store_64_clflush_movq,       .skip = 64,  .name = "store-clflush-movq-64"},
#ifdef AEP_SUPPORTED
    {.bench_func = store_64_clwb,               .skip = 64,  .name = "store-clwb-64"},
    {.bench_func = store_4_clwb_no_prefetch,    .skip = 4,   .name = "store-4-clwb-no-prefetch"},
    {.bench_func = store_8_clwb_no_prefetch,    .skip = 8,   .name = "store-8-clwb-no-prefetch"},
    {.bench_func = store_16_clwb_no_prefetch,   .skip = 16,  .name = "store-16-clwb-no-prefetch"},
    {.bench_func = store_32_clwb_no_prefetch,   .skip = 32,  .name = "store-32-clwb-no-prefetch"},
    {.bench_func = store_64_clwb_no_prefetch,   .skip = 64,  .name = "store-64-clwb-no-prefetch"},
    {.bench_func = store_64_no_prefetch,        .skip = 64,  .name = "store-64-no-prefetch"},
    {.bench_func = store_32_no_prefetch,        .skip = 32,  .name = "store-32-no-prefetch"},
    {.bench_func = store_16_no_prefetch,        .skip = 16,  .name = "store-16-no-prefetch"},
    {.bench_func = store_4_no_prefetch,         .skip = 4,   .name = "store-4-no-prefetch"},
    {.bench_func = store_64_clflushopt,         .skip = 64,  .name = "store-64-clflushopt"},
#endif

    {.bench_func = load_128,                    .skip = 128, .name = "load-128"},
    {.bench_func = load_128_movq,               .skip = 128, .name = "load-movq-128"},
    {.bench_func = load_128_nt,                 .skip = 128, .name = "load-nt-128"},
    {.bench_func = store_128,                   .skip = 128, .name = "store-128"},
    {.bench_func = store_128_movq,              .skip = 128, .name = "store-movq-128"},
    {.bench_func = store_128_nt,                .skip = 128, .name = "store-nt-128"},
    {.bench_func = store_128_nt_no_prefetch,    .skip = 128, .name = "store-nt-no-prefetch-128"},
    {.bench_func = store_128_clflush,           .skip = 128, .name = "store-clflush-128"},
    {.bench_func = store_128_clflush_movq,      .skip = 128, .name = "store-clflush-movq-128"},
#ifdef AEP_SUPPORTED
    {.bench_func = store_128_clwb,              .skip = 128, .name = "store-clwb-128"},
    {.bench_func = store_128_clwb_no_prefetch,  .skip = 128, .name = "store-clwb-no-prefetch-128"},
    {.bench_func = store_128_clflushopt,        .skip = 128, .name = "store-clflushopt-128"},
#endif

    {.bench_func = load_256,                    .skip = 256, .name = "load-256"},
    {.bench_func = load_256_movq,               .skip = 256, .name = "load-movq-256"},
    {.bench_func = load_256_nt,                 .skip = 256, .name = "load-nt-256"},
    {.bench_func = store_256,                   .skip = 256, .name = "store-256"},
    {.bench_func = store_256_movq,              .skip = 256, .name = "store-movq-256"},
    {.bench_func = store_256_nt,                .skip = 256, .name = "store-nt-256"},
    {.bench_func = store_256_nt_no_prefetch,    .skip = 256, .name = "store-nt-no-prefetch-256"},
    {.bench_func = store_256_clflush,           .skip = 256, .name = "store-clflush-256"},
    {.bench_func = store_256_clflush_movq,      .skip = 256, .name = "store-clflush-movq-256"},
#ifdef AEP_SUPPORTED
    {.bench_func = store_256_clwb,              .skip = 256, .name = "store-clwb-256"},
    {.bench_func = store_256_clwb_no_prefetch,  .skip = 256, .name = "store-clwb-no-prefetch-256"},
    {.bench_func = store_256_clflushopt,        .skip = 256, .name = "store-clflushopt-256"},
#endif
};
*/

const int BASIC_OPS_TASK_COUNT = sizeof(latency_tasks) / sizeof(struct latency_task);

/*
 * 256- benchmarks
 */
long repeat_256byte_ntstore(char *addr) {
    long t1 = 0, t2 = 0;
    long value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_VALUE
        LOAD_ADDR
        TIMING_BEG
        "vmovntpd %%ymm0, 0*32(%%rsi) \n"
        "vmovntpd %%ymm0, 1*32(%%rsi) \n"
        "vmovntpd %%ymm0, 2*32(%%rsi) \n"
        "vmovntpd %%ymm0, 3*32(%%rsi) \n"
        "vmovntpd %%ymm0, 4*32(%%rsi) \n"
        "vmovntpd %%ymm0, 5*32(%%rsi) \n"
        "vmovntpd %%ymm0, 6*32(%%rsi) \n"
        "vmovntpd %%ymm0, 7*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

/*
 * 256-byte benchmarks
 */
long repeat_256byte_load(char *addr) {
    long t1 = 0, t2 = 0;
    long value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_VALUE
        LOAD_ADDR
        TIMING_BEG
        "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
        "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
        "vmovntdqa 2*32(%%rsi), %%ymm2 \n"
        "vmovntdqa 3*32(%%rsi), %%ymm3 \n"
        "vmovntdqa 4*32(%%rsi), %%ymm4 \n"
        "vmovntdqa 5*32(%%rsi), %%ymm5 \n"
        "vmovntdqa 6*32(%%rsi), %%ymm6 \n"
        "vmovntdqa 7*32(%%rsi), %%ymm7 \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}
