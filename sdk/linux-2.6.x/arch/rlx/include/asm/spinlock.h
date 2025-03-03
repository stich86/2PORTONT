/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999, 2000, 06 Ralf Baechle (ralf@linux-mips.org)
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_SPINLOCK_H
#define _ASM_SPINLOCK_H

/* tonywu: NOT USED in single CPU platform */

#include <linux/compiler.h>
#include <asm/barrier.h>

/*
 * Your basic SMP spinlocks, allowing only a single CPU anywhere
 *
 * Simple spin lock operations.  There are two variants, one clears IRQ's
 * on the local processor, one does not.
 *
 * These are fair FIFO ticket locks
 *
 * (the type definitions are in asm/spinlock_types.h)
 */


/*
 * Ticket locks are conceptually two parts, one indicating the current head of
 * the queue, and the other indicating the current tail. The lock is acquired
 * by atomically noting the tail and incrementing it by one (thus adding
 * ourself to the queue and noting our position), then waiting until the head
 * becomes equal to the the initial value of the tail.
 */

static inline int __raw_spin_is_locked(raw_spinlock_t *lock)
{
	u32 counters = ACCESS_ONCE(lock->lock);

	return ((counters >> 16) ^ counters) & 0xffff;
}

#define __raw_spin_lock_flags(lock, flags) __raw_spin_lock(lock)
#define __raw_spin_unlock_wait(x) \
	while (__raw_spin_is_locked(x)) { cpu_relax(); }

static inline int __raw_spin_is_contended(raw_spinlock_t *lock)
{
	u32 counters = ACCESS_ONCE(lock->lock);

	return (((counters >> 16) - counters) & 0xffff) > 1;
}
#define __raw_spin_is_contended	__raw_spin_is_contended

static inline void __raw_spin_lock(raw_spinlock_t *lock)
{
	int my_ticket;
	int tmp;
    int inc = 0x10000;

		__asm__ __volatile__ (
		"	.set push		# __raw_spin_lock	\n"
		"	.set noreorder					\n"
		"							\n"
		"1:	ll	%[ticket], %[ticket_ptr]		\n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	 nop						\n"
#endif
		"	addu	%[my_ticket], %[ticket], %[inc]		\n"
		"	sc	%[my_ticket], %[ticket_ptr]		\n"
        "       beqz    %[my_ticket], 1b                        \n"
        "        srl    %[my_ticket], %[ticket], 16             \n"
        "       andi    %[ticket], %[ticket], 0xffff            \n"
        "       andi    %[my_ticket], %[my_ticket], 0xffff      \n"
		"	bne	%[ticket], %[my_ticket], 4f		\n"
		"	 subu	%[ticket], %[my_ticket], %[ticket]	\n"
		"2:							\n"
		"	.subsection 2					\n"
		"4:	andi	%[ticket], %[ticket], 0x1fff		\n"
		"	sll	%[ticket], 5				\n"
		"							\n"
		"6:	bnez	%[ticket], 6b				\n"
		"	 subu	%[ticket], 1				\n"
		"							\n"
                "       lhu     %[ticket], %[serving_now_ptr]           \n"
		"	beq	%[ticket], %[my_ticket], 2b		\n"
		"	 subu	%[ticket], %[my_ticket], %[ticket]	\n"
		"	b	4b					\n"
		"	 subu	%[ticket], %[ticket], 1			\n"
		"	.previous					\n"
		"	.set pop					\n"
		: [ticket_ptr] "+m" (lock->lock),
          [serving_now_ptr] "+m" (lock->h.serving_now),
		  [ticket] "=&r" (tmp),
		  [my_ticket] "=&r" (my_ticket)
        : [inc] "r" (inc));
}

static inline void __raw_spin_unlock(raw_spinlock_t *lock)
{
  unsigned int serving_now = lock->h.serving_now + 1;
  wmb();
  lock->h.serving_now = (u16)serving_now;
  nudge_writes();
}

static inline unsigned int __raw_spin_trylock(raw_spinlock_t *lock)
{
	int tmp, tmp2, tmp3;
    int inc = 0x10000;

		__asm__ __volatile__ (
		"	.set push		# __raw_spin_trylock	\n"
		"	.set noreorder					\n"
		"							\n"
        "1: ll      %[ticket], %[ticket_ptr]                \n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	nop							\n"
#endif
        "   srl     %[my_ticket], %[ticket], 16             \n"
        "   andi    %[my_ticket], %[my_ticket], 0xffff      \n"
        "   andi    %[now_serving], %[ticket], 0xffff       \n"
		"	bne	%[my_ticket], %[now_serving], 3f	\n"
		"	 addu	%[ticket], %[ticket], %[inc]		\n"
		"	sc	%[ticket], %[ticket_ptr]		\n"
		"	beqz	%[ticket], 1b				\n"
		"	 li	%[ticket], 1				\n"
		"2:							\n"
		"	.subsection 2					\n"
		"3:	b	2b					\n"
		"	 li	%[ticket], 0				\n"
		"	.previous					\n"
		"	.set pop					\n"
		: [ticket_ptr] "+m" (lock->lock),
		  [ticket] "=&r" (tmp),
		  [my_ticket] "=&r" (tmp2),
          [now_serving] "=&r" (tmp3)
        : [inc] "r" (inc));

	return tmp;
}

/*
 * Read-write spinlocks, allowing multiple readers but only one writer.
 *
 * NOTE! it is quite common to have readers in interrupts but no interrupt
 * writers. For those circumstances we can "mix" irq-safe locks - any writer
 * needs to get a irq-safe write-lock, but readers can get non-irqsafe
 * read-locks.
 */

/*
 * read_can_lock - would read_trylock() succeed?
 * @lock: the rwlock in question.
 */
#define __raw_read_can_lock(rw)	((rw)->lock >= 0)

/*
 * write_can_lock - would write_trylock() succeed?
 * @lock: the rwlock in question.
 */
#define __raw_write_can_lock(rw)	(!(rw)->lock)

static inline void __raw_read_lock(raw_rwlock_t *rw)
{
	unsigned int tmp;

		__asm__ __volatile__(
		"	.set	noreorder	# __raw_read_lock	\n"
		"1:	ll	%1, %2					\n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	nop							\n"
#endif
		"	bltz	%1, 2f					\n"
		"	 addu	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 nop						\n"
		"	.subsection 2					\n"
		"2:	ll	%1, %2					\n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	nop							\n"
#endif
		"	bltz	%1, 2b					\n"
		"	 addu	%1, 1					\n"
		"	b	1b					\n"
		"	 nop						\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
}

/* Note the use of sub, not subu which will make the kernel die with an
   overflow exception if we ever try to unlock an rwlock that is already
   unlocked or is being held by a writer.  */
static inline void __raw_read_unlock(raw_rwlock_t *rw)
{
	unsigned int tmp;

		__asm__ __volatile__(
		"	.set	noreorder	# __raw_read_unlock	\n"
		"1:	ll	%1, %2					\n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	nop							\n"
#endif
		"	sub	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 2f					\n"
		"	 nop						\n"
		"	.subsection 2					\n"
		"2:	b	1b					\n"
		"	 nop						\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
}

static inline void __raw_write_lock(raw_rwlock_t *rw)
{
	unsigned int tmp;

		__asm__ __volatile__(
		"	.set	noreorder	# __raw_write_lock	\n"
		"1:	ll	%1, %2					\n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	nop							\n"
#endif
		"	bnez	%1, 2f					\n"
		"	 lui	%1, 0x8000				\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 2f					\n"
		"	 nop						\n"
		"	.subsection 2					\n"
		"2:	ll	%1, %2					\n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	nop							\n"
#endif
		"	bnez	%1, 2b					\n"
		"	 lui	%1, 0x8000				\n"
		"	b	1b					\n"
		"	 nop						\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp)
		: "m" (rw->lock)
		: "memory");
}

static inline void __raw_write_unlock(raw_rwlock_t *rw)
{
	smp_mb();

	__asm__ __volatile__(
	"				# __raw_write_unlock	\n"
	"	sw	$0, %0					\n"
	: "=m" (rw->lock)
	: "m" (rw->lock)
	: "memory");
}

static inline int __raw_read_trylock(raw_rwlock_t *rw)
{
	unsigned int tmp;
	int ret;

		__asm__ __volatile__(
		"	.set	noreorder	# __raw_read_trylock	\n"
		"	li	%2, 0					\n"
		"1:	ll	%1, %3					\n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	nop							\n"
#endif
		"	bltz	%1, 2f					\n"
		"	 addu	%1, 1					\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 1b					\n"
		"	 nop						\n"
		"	.set	reorder					\n"
		__WEAK_LLSC_MB
		"	li	%2, 1					\n"
		"2:							\n"
		: "=m" (rw->lock), "=&r" (tmp), "=&r" (ret)
		: "m" (rw->lock)
		: "memory");

	return ret;
}

static inline int __raw_write_trylock(raw_rwlock_t *rw)
{
	unsigned int tmp;
	int ret;

		__asm__ __volatile__(
		"	.set	noreorder	# __raw_write_trylock	\n"
		"	li	%2, 0					\n"
		"1:	ll	%1, %3					\n"
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181)
		"	nop							\n"
#endif
		"	bnez	%1, 2f					\n"
		"	lui	%1, 0x8000				\n"
		"	sc	%1, %0					\n"
		"	beqz	%1, 3f					\n"
		"	 li	%2, 1					\n"
		"2:							\n"
		__WEAK_LLSC_MB
		"	.subsection 2					\n"
		"3:	b	1b					\n"
		"	 li	%2, 0					\n"
		"	.previous					\n"
		"	.set	reorder					\n"
		: "=m" (rw->lock), "=&r" (tmp), "=&r" (ret)
		: "m" (rw->lock)
		: "memory");

	return ret;
}

#define __raw_read_lock_flags(lock, flags) __raw_read_lock(lock)
#define __raw_write_lock_flags(lock, flags) __raw_write_lock(lock)

#define _raw_spin_relax(lock)	cpu_relax()
#define _raw_read_relax(lock)	cpu_relax()
#define _raw_write_relax(lock)	cpu_relax()

#endif /* _ASM_SPINLOCK_H */
