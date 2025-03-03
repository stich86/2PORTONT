/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 95, 96, 97, 98, 99, 2003, 06 by Ralf Baechle
 * Copyright (C) 1996 by Paul M. Antoine
 * Copyright (C) 1999 Silicon Graphics
 * Kevin D. Kissell, kevink@mips.org and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.
 */
#ifndef _ASM_SYSTEM_H
#define _ASM_SYSTEM_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/irqflags.h>
#include <linux/linkage.h>

#include <asm/addrspace.h>
#include <asm/barrier.h>
#include <asm/cmpxchg.h>
#include <asm/cpu-features.h>


/*
 * switch_to(n) should switch tasks to task nr n, first
 * checking that n isn't the current task, in which case it does nothing.
 */
extern asmlinkage void *resume(void *last, void *next, void *next_ti);

struct task_struct;

#define switch_to(prev, next, last)					\
do {									\
	(last) = resume(prev, next, task_thread_info(next));		\
} while (0)

#ifdef CONFIG_CPU_HAS_TLS
# define __save_userlocal(x)	write_lxc0_userlocal((x))
#else
# define __save_userlocal(x)	do { } while (0)
#endif

#if defined(CONFIG_CPU_HAS_WATCH)
#define finish_arch_switch(prev)				\
do {								\
	__save_userlocal(current_thread_info()->tp_value);	\
	__restore_watch();					\
} while (0)
#else
#define finish_arch_switch(prev)				\
do {								\
	__save_userlocal(current_thread_info()->tp_value);	\
} while (0)
#endif



static inline unsigned long __xchg_u32(volatile int * m, unsigned int val)
{
	__u32 retval;

#ifdef CONFIG_CPU_HAS_LLSC
		unsigned long dummy;

		__asm__ __volatile__(
		"1:	ll	%0, %3			# xchg_u32	\n"
		"	move	%2, %z4					\n"
		"	sc	%2, %1					\n"
		"	beqz	%2, 2f					\n"
		"	.subsection 2					\n"
		"2:	b	1b					\n"
		"	.previous					\n"
		: "=&r" (retval), "=m" (*m), "=&r" (dummy)
		: "R" (*m), "Jr" (val)
		: "memory");
#else
		unsigned long flags;

		raw_local_irq_save(flags);
		retval = *m;
		*m = val;
		raw_local_irq_restore(flags);	/* implies memory barrier  */
#endif

	return retval;
}

extern __u64 __xchg_u64_unsupported_on_32bit_kernels(volatile __u64 * m, __u64 val);
#define __xchg_u64 __xchg_u64_unsupported_on_32bit_kernels

static __always_inline unsigned long __xchg(unsigned long x, volatile void * ptr, int size)
{
	switch (size) {
	case 4:
		return __xchg_u32(ptr, x);
	case 8:
		return __xchg_u64(ptr, x);
	}

	return x;
}

#define xchg(ptr, x)                                            \
({                                                              \
    BUILD_BUG_ON(sizeof(*(ptr)) & ~0xc);                        \
                                                                \
    ((__typeof__(*(ptr)))                                       \
          __xchg((unsigned long)(x), (ptr), sizeof(*(ptr))));   \
})

extern void set_handler(unsigned long offset, void *addr, unsigned long len);
extern void set_uncached_handler(unsigned long offset, void *addr, unsigned long len);

extern void *set_except_vector(int n, void *addr);
extern unsigned long ebase;
extern void per_cpu_trap_init(void);

/*
 * See include/asm-ia64/system.h; prevents deadlock on SMP
 * systems.
 */
#define __ARCH_WANT_UNLOCKED_CTXSW

extern unsigned long arch_align_stack(unsigned long sp);

#endif /* _ASM_SYSTEM_H */
