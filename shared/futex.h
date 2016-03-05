#ifndef BBGL_SHARED_FUTEX_HDR
#define BBGL_SHARED_FUTEX_HDR

#include <linux/futex.h>

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

static int futex(void *addr1, int op, int val1, void *timeout, void *addr2, int val3) {
    return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

static inline unsigned int xchg(void *ptr, unsigned int x) {
    __asm__ __volatile__("xchgl %0, %1"
        : "=r" (x)
        : "m" (*(volatile unsigned int *)ptr), "0" (x)
        : "memory");
    return x;
}

#define cmpxchg(P, O, N) __sync_val_compare_and_swap((P), (O), (N))
#define atomic_inc(P) __sync_add_and_fetch((P), 1)
#define atomic_dec(P) __sync_add_and_fetch((P), -1)
#define atomic_add(P, V) __sync_add_and_fetch((P), (V))

#endif
