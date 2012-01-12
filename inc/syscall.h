/*
 * PIOS system call definitions.
 *
 * Copyright (C) 2010 Yale University.
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Primary author: Bryan Ford
 */

#ifndef PIOS_INC_SYSCALL_H
#define PIOS_INC_SYSCALL_H

#include <inc/trap.h>


// System call command codes (passed in EAX)
#define SYS_TYPE	0x00000003	// Basic operation type
#define SYS_CPUTS	0x00000000	// Write debugging string to console
#define SYS_PUT		0x00000001	// Push data to child and start it
#define SYS_PUT		0x00000001	// Push data to child and start it
#define SYS_GET		0x00000002	// Pull results from child
#define SYS_RET		0x00000003	// Return to parent

#define SYS_START	0x00000010	// Put: start child running

#define SYS_REGS	0x00001000	// Get/put register state
#define SYS_FPU		0x00002000	// Get/put FPU state


// Register conventions for CPUTS system call:
//	EAX:	System call command
//	EBX:	User pointer to string to output to console
#define SYS_CPUTS_MAX	256	// Max buffer length cputs will accept


// Register conventions on GET/PUT system call entry:
//	EAX:	System call command/flags (SYS_*)
//	EDX:	bits 7-0: Child process number to get/put
//	EBX:	Get/put CPU state pointer for SYS_REGS and/or SYS_FPU)
//	ECX:	Get/put memory region size
//	ESI:	Get/put local memory region start
//	EDI:	Get/put child memory region start
//	EBP:	reserved


#ifndef __ASSEMBLER__

// CPU state save area format for GET/PUT with SYS_REGS flags
typedef struct cpustate {
	trapframe	tf;		// general registers
	fxsave		fx;		// x87/MMX/XMM registers
} cpustate;


// Prototypes for user-level syscalls stubs defined in lib/syscall.c
void sys_cputs(const char *s);
void sys_put(uint32_t flags, uint16_t child, cpustate *cpu,
		void *localsrc, void *childdest, size_t size);
void sys_get(uint32_t flags, uint16_t child, cpustate *cpu,
		void *childsrc, void *localdest, size_t size);
void sys_ret(void);

#endif /* !__ASSEMBLER__ */

#endif /* !PIOS_INC_SYSCALL_H */
