/*
 * CPU setup and management of key protected-mode data structures,
 * such as global descriptor table (GDT) and task state segment (TSS).
 *
 * Copyright (C) 2010 Yale University.
 * See section "MIT License" in the file LICENSES for licensing terms.
 *
 * Primary author: Bryan Ford
 */

#include <inc/assert.h>
#include <inc/string.h>

#include <kern/mem.h>
#include <kern/cpu.h>
#include <kern/init.h>



cpu cpu_boot = {

	// Global descriptor table for bootstrap CPU.
	// The GDTs for other CPUs are copied from this and fixed up.
	//
	// The kernel and user segments are identical except for the DPL.
	// To load the SS register, the CPL must equal the DPL.  Thus,
	// we must duplicate the segments for the user and the kernel.
	//
	// The only descriptor that differs across CPUs is the TSS descriptor.
	//
	gdt: {
		// 0x0 - unused (always faults: for trapping NULL far pointers)
		[0] = SEGDESC_NULL,

		// 0x08 - kernel code segment
		[CPU_GDT_KCODE >> 3] = SEGDESC32(STA_X | STA_R, 0x0,
					0xffffffff, 0),

		// 0x10 - kernel data segment
		[CPU_GDT_KDATA >> 3] = SEGDESC32(STA_W, 0x0,
					0xffffffff, 0),

		// 0x18 - user code segment
		[CPU_GDT_UCODE >> 3] = SEGDESC32(STA_X | STA_R, 0x0,
					0xffffffff, 3),

		// 0x20 - user data segment
		[CPU_GDT_UDATA >> 3] = SEGDESC32(STA_W, 0x0,
					0xffffffff, 3),
	},

	magic: CPU_MAGIC
};


void cpu_init()
{
	cpu *c = cpu_cur();

	(c->tss).ts_esp0=(uintptr_t)&c->kstackhi;
	(c->tss).ts_ss0=CPU_GDT_KDATA;

	c->gdt[CPU_GDT_TSS >> 3]=SEGDESC16(STS_T32A, (uint32_t)&(c->tss),
					sizeof(struct taskstate), 0);
	c->gdt[CPU_GDT_TSS >> 3].sd_s=0;

	// Load the GDT
	struct pseudodesc gdt_pd = {
		sizeof(c->gdt) - 1, (uint32_t) c->gdt };
	asm volatile("lgdt %0" : : "m" (gdt_pd));

	// Reload all segment registers.
	asm volatile("movw %%ax,%%gs" :: "r" (CPU_GDT_UDATA|3));
	asm volatile("movw %%ax,%%fs" :: "r" (CPU_GDT_UDATA|3));
	asm volatile("movw %%ax,%%es" :: "r" (CPU_GDT_KDATA));
	asm volatile("movw %%ax,%%ds" :: "r" (CPU_GDT_KDATA));
	asm volatile("movw %%ax,%%ss" :: "r" (CPU_GDT_KDATA));
	asm volatile("ljmp %0,$1f\n 1:\n" :: "i" (CPU_GDT_KCODE)); // reload CS

	// We don't need an LDT.
	asm volatile("lldt %%ax" :: "a" (0));

	ltr(CPU_GDT_TSS);
	
}


