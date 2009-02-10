#ifndef KERNELCOMPAT_H
#define KERNELCOMPAT_H

#include "compiler.h"

#include <string.h>

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define KERN_DEBUG ""
#define KERN_NOTICE ""
#define KERN_INFO ""
#define KERN_ERR ""
#define KERN_ALERT ""

/* ERROR OPS */

#define MAX_ERRNO	4095

#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)

static inline void *ERR_PTR(long error)
{
	return (void *) error;
}

static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}


/* FIXED SIZE INTEGERS */

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define min_t(type, x, y) ({                    \
	type __min1 = (x);                      \
	type __min2 = (y);                      \
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({                    \
	type __max1 = (x);                      \
	type __max2 = (y);                      \
	__max1 > __max2 ? __max1: __max2; })


/* MUTEXES */

#include <pthread.h>

#define DEFINE_MUTEX(m) pthread_mutex_t (m) = PTHREAD_MUTEX_INITIALIZER;

#define mutex_lock(m) pthread_mutex_lock(m)

#define mutex_unlock(m) pthread_mutex_unlock(m)

/* MALLOCATION */

#include <stdlib.h>

#define kmalloc(s, t) malloc(s)
#define kzalloc(s, t) malloc(s)
#define kfree(p) free((void *)p)
#define kstrdup(s, t) strdup(s)

/* PRINTK */

#include <stdio.h>
#define printk(fmt, args...) printf(fmt, ## args)

/* MEMORY BARRIERS */

#define smp_rmb() do {} while(0)
#define smp_wmb() do {} while(0)
#define smp_mb() do {} while(0)
#define smp_mb__after_atomic_inc() do {} while(0)

#define read_barrier_depends() do {} while(0)
#define smp_read_barrier_depends() do {} while(0)

/* RCU */

#define rcu_assign_pointer(a, b) do {} while(0)
#define call_rcu_sched(a,b) do {} while(0)
#define rcu_barrier_sched() do {} while(0)

/* ATOMICITY */

#include <signal.h>

typedef struct { sig_atomic_t counter; } atomic_t;

static inline int atomic_dec_and_test(atomic_t *p)
{
	(p->counter)--;
	return !p->counter;
}

static inline void atomic_set(atomic_t *p, int v)
{
	p->counter=v;
}

static inline void atomic_inc(atomic_t *p)
{
	p->counter++;
}

static int atomic_read(atomic_t *p)
{
	return p->counter;
}

/* LOCAL OPS */



/* ATTRIBUTES */

#define ____cacheline_aligned
#define __init
#define __exit

/* MATH */

static inline unsigned int hweight32(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res + (res >> 4)) & 0x0F0F0F0F;
	res = res + (res >> 8);
	return (res + (res >> 16)) & 0x000000FF;
}

static inline int fls(int x)
{
        int r;
//ust// #ifdef CONFIG_X86_CMOV
        asm("bsrl %1,%0\n\t"
            "cmovzl %2,%0"
            : "=&r" (r) : "rm" (x), "rm" (-1));
//ust// #else
//ust//         asm("bsrl %1,%0\n\t"
//ust//             "jnz 1f\n\t"
//ust//             "movl $-1,%0\n"
//ust//             "1:" : "=r" (r) : "rm" (x));
//ust// #endif
        return r + 1;
}

static __inline__ int get_count_order(unsigned int count)
{
	int order;
	
	order = fls(count) - 1;
	if (count & (count - 1))
		order++;
	return order;
}




#include <unistd.h>

#define ALIGN(x,a)		__ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define PAGE_ALIGN(addr) ALIGN(addr, PAGE_SIZE)
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)




/* ARRAYS */

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* TRACE CLOCK */

static inline u64 trace_clock_read64(void)
{
	return 0LL;
}

static inline unsigned int trace_clock_frequency(void)
{
	return 0LL;
}

static inline u32 trace_clock_freq_scale(void)
{
	return 0;
}

#endif /* KERNELCOMPAT_H */
