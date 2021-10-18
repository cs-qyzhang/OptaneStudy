/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LATTESTER_COMMON_H
#define LATTESTER_COMMON_H
/*
 * BRIEF DESCRIPTION
 *
 * Header for commonly used defines
 *
 * Copyright 2019 Regents of the University of California,
 * UCSD Non-Volatile Systems Lab
 */

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
#define REGISTERS "rsi", "rax", "rdx", "rcx", "r8", "r9", "ymm0"
#define REGISTERS_NONSSE "rsi", "rax", "rdx", "rcx", "r8", "r9"

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
                                "mov %%eax, %%r8d \n" \

/* r9d = old EDX
 * r8d = old EAX
 * Here is what we do to compute t1 and t2:
 * - RDX holds t2
 * - RAX holds t1
 */
#define TIMING_END              "mfence \n" \
                                "rdtscp \n" \
                                "lfence \n" \
                                "shl $32, %%rdx \n" \
                                "or  %%rax, %%rdx \n" \
                                "mov %%r9d, %%eax \n" \
                                "shl $32, %%rax \n" \
                                "or  %%r8, %%rax \n" \
                                "mov %%rax, %[t1] \n" \
                                "mov %%rdx, %[t2] \n"

#define TIMING_END_NOFENCE      "rdtscp \n" \
                                "lfence \n" \
                                "shl $32, %%rdx \n" \
                                "or  %%rax, %%rdx \n" \
                                "mov %%r9d, %%eax \n" \
                                "shl $32, %%rax \n" \
                                "or  %%r8, %%rax \n" \
                                "mov %%rax, %[t1] \n" \
                                "mov %%rdx, %[t2] \n"

uint64_t baseline(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(CLEAR_PIPELINE
                 TIMING_BEG
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 :: REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

/*
 * 64-byte benchmarks
 */
uint64_t store_64byte(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    // vmovdqa: 32-byte store to memory
    asm volatile(LOAD_VALUE
                 LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clflush(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "clflush (%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clwb(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "clwb (%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clwb_without_prefetch(char *addr)
{
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 // LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "clwb (%%rsi) \n"
                 TIMING_END
                 : [t1] "=r"(t1), [t2] "=r"(t2)
                 : [memarea] "r"(addr), [value] "m"(value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clflushopt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "clflushopt (%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /*
     * vmovntpd: 32-byte non-temporal store (check below)
     * https://software.intel.com/en-us/node/524246
     */
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovntpd %%ymm0, 0*32(%%rsi) \n"
                 "vmovntpd %%ymm0, 1*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_nt_with_cacheline_prefetched(char *addr)
{
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /*
     * vmovntpd: 32-byte non-temporal store (check below)
     * https://software.intel.com/en-us/node/524246
     */
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 // FLUSH_CACHE_LINE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovntpd %%ymm0, 0*32(%%rsi) \n"
                 "vmovntpd %%ymm0, 1*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r"(t1), [t2] "=r"(t2)
                 : [memarea] "r"(addr), [value] "m"(value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_64byte(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa 0*32(%%rsi), %%ymm0 \n"
                 "vmovdqa 1*32(%%rsi), %%ymm1 \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_64byte_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    /*
     * Requires avx2
     * https://www.felixcloutier.com/x86/MOVNTDQA.html
     */
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
                 "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}


uint64_t store_64byte_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "movq %%rdx, 0*8(%%rsi) \n"
                 "movq %%rdx, 1*8(%%rsi) \n"
                 "movq %%rdx, 2*8(%%rsi) \n"
                 "movq %%rdx, 3*8(%%rsi) \n"
                 "movq %%rdx, 4*8(%%rsi) \n"
                 "movq %%rdx, 5*8(%%rsi) \n"
                 "movq %%rdx, 6*8(%%rsi) \n"
                 "movq %%rdx, 7*8(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_64byte_clflush_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "movq %%rdx, 0*8(%%rsi) \n"
                 "movq %%rdx, 1*8(%%rsi) \n"
                 "movq %%rdx, 2*8(%%rsi) \n"
                 "movq %%rdx, 3*8(%%rsi) \n"
                 "movq %%rdx, 4*8(%%rsi) \n"
                 "movq %%rdx, 5*8(%%rsi) \n"
                 "movq %%rdx, 6*8(%%rsi) \n"
                 "movq %%rdx, 7*8(%%rsi) \n"
                 "clflush (%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_64byte_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "movq 0*8(%%rsi), %%rdx \n"
                 "movq 1*8(%%rsi), %%rdx \n"
                 "movq 2*8(%%rsi), %%rdx \n"
                 "movq 3*8(%%rsi), %%rdx \n"
                 "movq 4*8(%%rsi), %%rdx \n"
                 "movq 5*8(%%rsi), %%rdx \n"
                 "movq 6*8(%%rsi), %%rdx \n"
                 "movq 7*8(%%rsi), %%rdx \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

/*
 * 128-byte benchmarks
 */
uint64_t store_128byte(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_VALUE
                 LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 2*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 3*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_clflush(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "clflush (%%rsi) \n"
                 "vmovdqa %%ymm0, 2*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 3*32(%%rsi) \n"
                 "clflush 2*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_clwb(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "clwb (%%rsi) \n"
                 "vmovdqa %%ymm0, 2*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 3*32(%%rsi) \n"
                 "clwb 2*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_clflushopt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "clflushopt (%%rsi) \n"
                 "vmovdqa %%ymm0, 2*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 3*32(%%rsi) \n"
                 "clflushopt 2*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovntpd %%ymm0, 0*32(%%rsi) \n"
                 "vmovntpd %%ymm0, 1*32(%%rsi) \n"
                 "vmovntpd %%ymm0, 2*32(%%rsi) \n"
                 "vmovntpd %%ymm0, 3*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_128byte(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa 0*32(%%rsi), %%ymm0 \n"
                 "vmovdqa 1*32(%%rsi), %%ymm1 \n"
                 "vmovdqa 2*32(%%rsi), %%ymm1 \n"
                 "vmovdqa 3*32(%%rsi), %%ymm1 \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_128byte_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
                 "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
                 "vmovntdqa 2*32(%%rsi), %%ymm1 \n"
                 "vmovntdqa 3*32(%%rsi), %%ymm1 \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_128byte_clflush_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_128byte_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

/*
 * 256-byte benchmarks
 */
uint64_t store_256byte(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_VALUE
                 LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa %%ymm0, 0*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 1*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 2*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 3*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 4*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 5*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 6*32(%%rsi) \n"
                 "vmovdqa %%ymm0, 7*32(%%rsi) \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_clflush(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_clwb(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_clflushopt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr), [value] "m" (value)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_VALUE
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
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

uint64_t load_256byte(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovdqa 0*32(%%rsi), %%ymm0 \n"
                 "vmovdqa 1*32(%%rsi), %%ymm1 \n"
                 "vmovdqa 2*32(%%rsi), %%ymm1 \n"
                 "vmovdqa 3*32(%%rsi), %%ymm1 \n"
                 "vmovdqa 4*32(%%rsi), %%ymm1 \n"
                 "vmovdqa 5*32(%%rsi), %%ymm1 \n"
                 "vmovdqa 6*32(%%rsi), %%ymm1 \n"
                 "vmovdqa 7*32(%%rsi), %%ymm1 \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_256byte_nt(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
                 "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
                 "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
                 "vmovntdqa 2*32(%%rsi), %%ymm1 \n"
                 "vmovntdqa 3*32(%%rsi), %%ymm1 \n"
                 "vmovntdqa 4*32(%%rsi), %%ymm1 \n"
                 "vmovntdqa 5*32(%%rsi), %%ymm1 \n"
                 "vmovntdqa 6*32(%%rsi), %%ymm1 \n"
                 "vmovntdqa 7*32(%%rsi), %%ymm1 \n"
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t store_256byte_clflush_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 LOAD_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

uint64_t load_256byte_movq(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
                 FLUSH_CACHE_LINE
                 CLEAR_PIPELINE
                 TIMING_BEG
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
                 TIMING_END
                 : [t1] "=r" (t1), [t2] "=r" (t2)
                 : [memarea] "r" (addr)
                 : REGISTERS_NONSSE);
    KERNEL_END
    return t2 - t1;
}

// Kernel-level task lists
struct latency_task {
    uint64_t (*bench_func)(char *);
    char* name;
    size_t skip;
};

struct latency_task latency_tasks[] = {
    {.bench_func = baseline,                .skip = 0, .name = "baseline"},

    {.bench_func = load_64byte,             .skip = 64, .name = "load-64"},
    {.bench_func = load_64byte_nt,          .skip = 64, .name = "load-nt-64"},
    {.bench_func = store_64byte,            .skip = 64, .name = "store-64"},
    {.bench_func = store_64byte_clflush,    .skip = 64, .name = "store-clflush-64"},
#ifdef AEP_SUPPORTED
    {.bench_func = store_64byte_clwb,       .skip = 64, .name = "store-clwb-64"},
    {.bench_func = store_64byte_clwb_without_prefetch, .skip = 64, .name = "store-clwb-no-prefetch-64"},
    {.bench_func = store_64byte_clflushopt, .skip = 64, .name = "store-clflushopt-64"},
#endif
    {.bench_func = store_64byte_nt,         .skip = 64, .name = "store-nt-64"},
    {.bench_func = store_64byte_nt_with_cacheline_prefetched, .skip = 64, .name = "store-nt-prefetched-64"},
    {.bench_func = store_64byte_movq,       .skip = 64, .name = "store-movq-64"},
    {.bench_func = store_64byte_clflush_movq, .skip = 64, .name = "store-clflush-movq-64"},
    {.bench_func = load_64byte_movq,        .skip = 64, .name = "load-movq-64"},

    {.bench_func = load_128byte,            .skip = 128, .name = "load-128"},
    {.bench_func = load_128byte_nt,         .skip = 128, .name = "load-nt-128"},
    {.bench_func = store_128byte,           .skip = 128, .name = "store-128"},
    {.bench_func = store_128byte_clflush,   .skip = 128, .name = "store-clflush-128"},
#ifdef AEP_SUPPORTED
    {.bench_func = store_128byte_clwb,      .skip = 128, .name = "store-clwb-128"},
    {.bench_func = store_128byte_clflushopt,.skip = 128, .name = "store-clflushopt-128"},
#endif
    {.bench_func = store_128byte_nt,        .skip = 128, .name = "store-nt-128"},
    {.bench_func = store_128byte_movq,      .skip = 128, .name = "store-movq-128"},
    {.bench_func = store_128byte_clflush_movq,.skip = 128, .name = "store-clflush-movq-128"},
    {.bench_func = load_128byte_movq,       .skip = 128, .name = "load-movq-128"},

    {.bench_func = load_256byte,            .skip = 256, .name = "load-256"},
    {.bench_func = load_256byte_nt,         .skip = 256, .name = "load-nt-256"},
    {.bench_func = store_256byte,           .skip = 256, .name = "store-256"},
    {.bench_func = store_256byte_clflush,   .skip = 256, .name = "store-clflush-256"},
#ifdef AEP_SUPPORTED
    {.bench_func = store_256byte_clwb,      .skip = 256, .name = "store-clwb-256"},
    {.bench_func = store_256byte_clflushopt,.skip = 256, .name = "store-clflushopt-256"},
#endif
    {.bench_func = store_256byte_nt,        .skip = 256, .name = "store-nt-256"},
    {.bench_func = store_256byte_movq,      .skip = 256, .name = "store-movq-256"},
    {.bench_func = store_256byte_clflush_movq,.skip = 256, .name = "store-clflush-movq-256"},
    {.bench_func = load_256byte_movq,       .skip = 256, .name = "load-movq-256"},
};

const int BASIC_OPS_TASK_COUNT = sizeof(latency_tasks) / sizeof(struct latency_task);

/*
 * 256-byte benchmarks
 */
uint64_t repeat_256byte_ntstore(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
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
uint64_t repeat_256byte_load(char *addr) {
    uint64_t t1 = 0, t2 = 0;
    uint64_t value = 0xC0FFEEEEBABE0000;
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

#endif // LATTESTER_COMMON_H
