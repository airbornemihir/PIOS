/*
 * Kernel initialization.
 *
 * Copyright (C) 1997 Massachusetts Institute of Technology
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Derived from the MIT Exokernel and JOS.
 * Adapted for PIOS by Bryan Ford at Yale University.
 */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/gcc.h>
#include <inc/syscall.h>

#include <kern/init.h>
#include <kern/console.h>
#include <kern/debug.h>
#include <kern/mem.h>
#include <kern/cpu.h>
#include <kern/trap.h>



// User-mode stack for user(), below, to run on.
static char gcc_aligned(16) user_stack[PAGESIZE];

// Lab 3: ELF executable containing root process, linked into the kernel
#ifndef ROOTEXE_START
#endif
extern char ROOTEXE_START[];

void inittests() {
	cprintf("1024=%d\n", 1024);
}


// Called first from entry.S on the bootstrap processor,
// and later from boot/bootother.S on all other processors.
// As a rule, "init" functions in PIOS are called once on EACH processor.
void
init(void)
{
	extern char start[], edata[], end[];

	// Before anything else, complete the ELF loading process.
	// Clear all uninitialized global data (BSS) in our program,
	// ensuring that all static/global variables start out zero.
	if (cpu_onboot())
		memset(edata, 0, end - edata);

	// Initialize the console.
	// Can't call cprintf until after we do this!
	cons_init();

	// Lab 1: test cprintf and debug_trace
	cprintf("1234 decimal is %o octal!\n", 1234);
	inittests();
	debug_check();

	// Initialize and load the bootstrap CPU's GDT, TSS, and IDT.
	cpu_init();
	trap_init();

	// Physical memory detection/initialization.
	// Can't call mem_alloc until after we do this!
	mem_init();


	// Lab 1: change this so it enters user() in user mode,
	// running on the user_stack declared above,
	// instead of just calling user() directly.
	/*
	char *loc=user_stack-sizeof(trapframe);
	register int *sp asm ("esp");
	asm volatile ("mov %0 %1" : "=r"(sp) : "X"(loc));
	*/
	trapframe tf;
	//register int *csreg asm ("cs");
	//tf.tf_cs=(*csreg)|3; //setting the privilege mode
	tf.tf_esp=(uintptr_t)user_stack;
	tf.tf_eflags=read_eflags();
	//tf.tf_ebp=read_ebp();
	tf.tf_cs=read_cs();
	//
	tf.tf_cs = (CPU_GDT_UCODE) | 3;
	tf.tf_ds = (CPU_GDT_UDATA) | 3;
	tf.tf_es = tf.tf_ds;
	tf.tf_ss = tf.tf_ds;
	tf.tf_eflags = FL_IOPL_3;
	tf.tf_esp = (uintptr_t)user_stack+PAGESIZE;
	tf.tf_eip = (uint32_t)&user;
	//
	trap_return(&tf);
	//user();
}

// This is the first function that gets run in user mode (ring 3).
// It acts as PIOS's "root process",
// of which all other processes are descendants.
void
user()
{
	cprintf("in user()\n");
	assert(read_esp() > (uint32_t) &user_stack[0]);
	assert(read_esp() < (uint32_t) &user_stack[sizeof(user_stack)]);

	// Check that we're in user mode and can handle traps from there.
	trap_check_user();

	done();
}

void gcc_noreturn
done()
{
	while (1)
		;	// just spin
}

