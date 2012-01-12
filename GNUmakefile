#
# This makefile system follows the structuring conventions
# recommended by Peter Miller in his excellent paper:
#
#	Recursive Make Considered Harmful
#	http://aegis.sourceforge.net/auug97.pdf
#
# Copyright (C) 2003 Massachusetts Institute of Technology 
# See section "MIT License" in the file LICENSES for licensing terms.
# Primary authors: Bryan Ford, Eddie Kohler, Austin Clemens
#
OBJDIR := obj

ifdef LAB
SETTINGLAB := true
else
-include conf/lab.mk
endif

-include conf/env.mk

ifndef SOL
SOL := 0
endif
ifndef LABADJUST
LABADJUST := 0
endif


TOP = .

# Cross-compiler toolchain
#
# This Makefile will automatically use the cross-compiler toolchain
# installed as 'i386-elf-*', if one exists.  If the host tools ('gcc',
# 'objdump', and so forth) compile for a 32-bit x86 ELF target, that will
# be detected as well.  If you have the right compiler toolchain installed
# using a different name, set GCCPREFIX explicitly in conf/env.mk

# try to infer the correct GCCPREFIX
ifndef GCCPREFIX
GCCPREFIX := $(shell if i386-elf-objdump -i 2>&1 | grep '^elf32-i386$$' >/dev/null 2>&1; \
	then echo 'i386-elf-'; \
	elif objdump -i 2>&1 | grep 'elf32-i386' >/dev/null 2>&1; \
	then echo ''; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an i386-elf version of GCC/binutils." 1>&2; \
	echo "*** Is the directory with i386-elf-gcc in your PATH?" 1>&2; \
	echo "*** If your i386-elf toolchain is installed with a command" 1>&2; \
	echo "*** prefix other than 'i386-elf-', set your GCCPREFIX" 1>&2; \
	echo "*** environment variable to that prefix and run 'make' again." 1>&2; \
	echo "*** To turn off this error, run 'gmake GCCPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

# try to infer the correct QEMU
ifndef QEMU
QEMU := $(shell \
	if test -x /c/cs422/tools/bin/qemu; \
	then echo /c/cs422/tools/bin/qemu; exit; \
	elif which qemu > /dev/null; \
	then echo qemu; exit; \
	else \
	qemu=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu; \
	if test -x $$qemu; then echo $$qemu; exit; fi; fi; \
	echo "***" 1>&2; \
	echo "*** Error: Couldn't find a working QEMU executable." 1>&2; \
	echo "*** Is the directory containing the qemu binary in your PATH" 1>&2; \
	echo "*** or have you tried setting the QEMU variable in conf/env.mk?" 1>&2; \
	echo "***" 1>&2; exit 1)
endif

# try to generate unique GDB and network port numbers
GDBPORT	:= $(shell expr `id -u` % 5000 + 25000)
NETPORT := $(shell expr `id -u` % 5000 + 30000)

# Correct option to enable the GDB stub and specify its port number to qemu.
# First is for qemu versions <= 0.10, second is for later qemu versions.
#QEMUPORT := -s -p $(GDBPORT)
QEMUPORT := -gdb tcp::$(GDBPORT)

CC	:= $(GCCPREFIX)gcc -pipe
AS	:= $(GCCPREFIX)as
AR	:= $(GCCPREFIX)ar
LD	:= $(GCCPREFIX)ld
OBJCOPY	:= $(GCCPREFIX)objcopy
OBJDUMP	:= $(GCCPREFIX)objdump
NM	:= $(GCCPREFIX)nm
GDB	:= $(GCCPREFIX)gdb

# Native commands
NCC	:= gcc $(CC_VER) -pipe
TAR	:= gtar
PERL	:= perl

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS := $(CFLAGS) $(DEFS) $(LABDEFS) -O1 -fno-builtin -I$(TOP) -MD 
CFLAGS += -Wall -Wno-unused -Werror -gstabs -m32

# Add -fno-stack-protector if the option exists.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Kernel versus user compiler flags
KERN_CFLAGS := $(CFLAGS) -DPIOS_KERNEL
USER_CFLAGS := $(CFLAGS) -DPIOS_USER

# Linker flags
LDFLAGS := -m elf_i386 -e start -nostdlib

KERN_LDFLAGS := $(LDFLAGS) -Ttext=0x00100000
USER_LDFLAGS := $(LDFLAGS) -Ttext=0x40000000

GCC_LIB := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

# Lists that the */Makefrag makefile fragments will add to
OBJDIRS :=

# Make sure that 'all' is the first target
all:

# Eliminate default suffix rules
.SUFFIXES:

# Delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# make it so that no intermediate .o files are ever deleted
.PRECIOUS: %.o $(OBJDIR)/boot/%.o $(OBJDIR)/kern/%.o \
	   $(OBJDIR)/lib/%.o $(OBJDIR)/fs/%.o $(OBJDIR)/net/%.o \
	   $(OBJDIR)/user/%.o




# Include Makefrags for subdirectories
include boot/Makefrag
include kern/Makefrag
include lib/Makefrag


IMAGES = $(OBJDIR)/kern/kernel.img
QEMUOPTS = -smp 2 -hda $(OBJDIR)/kern/kernel.img -serial mon:stdio
#QEMUNET = -net socket,mcast=230.0.0.1:$(NETPORT) -net nic,model=i82559er
QEMUNET1 = -net nic,model=i82559er,macaddr=52:54:00:12:34:01 \
		-net socket,connect=:$(NETPORT) -net dump,file=node1.dump
QEMUNET2 = -net nic,model=i82559er,macaddr=52:54:00:12:34:02 \
		-net socket,listen=:$(NETPORT) -net dump,file=node2.dump

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu: $(IMAGES)
	$(QEMU) $(QEMUOPTS)

qemu-nox: $(IMAGES)
	echo "*** Use Ctrl-a x to exit"
	$(QEMU) -nographic $(QEMUOPTS)

qemu-gdb: $(IMAGES) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUPORT)

qemu-gdb-nox: $(IMAGES) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUPORT)

which-qemu:
	@echo $(QEMU)

gdb: $(IMAGES)
	$(GDB) $(OBJDIR)/kern/kernel

gdb-boot: $(IMAGS)
	$(GDB) $(OBJDIR)/boot/bootblock.elf

# For deleting the build
clean:
	rm -rf $(OBJDIR)

realclean: clean
	rm -rf lab$(LAB).tar.gz grade-log

distclean: realclean
	rm -rf conf/gcc.mk

grade: grade-lab$(LAB).sh
	$(V)$(MAKE) clean >/dev/null 2>/dev/null
	$(MAKE) all
	sh grade-lab$(LAB).sh

tarball: realclean
	tar cf - `find . -type f | grep -v '^\.*$$' | grep -v '/CVS/' | grep -v '/\.svn/' | grep -v '/\.git/' | grep -v 'lab[0-9].*\.tar\.gz'` | gzip > lab$(LAB)-handin.tar.gz

# For test runs
run-%:
	$(V)rm -f $(OBJDIR)/kern/init.o $(IMAGES)
	$(V)$(MAKE) "DEFS=-DTEST=_binary_obj_user_$*_start -DTESTSIZE=_binary_obj_user_$*_size" $(IMAGES)
	echo "*** Use Ctrl-a x to exit"
	$(QEMU) -nographic $(QEMUOPTS)

xrun-%:
	$(V)rm -f $(OBJDIR)/kern/init.o $(IMAGES)
	$(V)$(MAKE) "DEFS=-DTEST=_binary_obj_user_$*_start -DTESTSIZE=_binary_obj_user_$*_size" $(IMAGES)
	$(QEMU) $(QEMUOPTS)

# This magic automatically generates makefile dependencies
# for header files included from C source files we compile,
# and keeps those dependencies up-to-date every time we recompile.
# See 'mergedep.pl' for more information.
$(OBJDIR)/.deps: $(foreach dir, $(OBJDIRS), $(wildcard $(OBJDIR)/$(dir)/*.d))
	@mkdir -p $(@D)
	@$(PERL) mergedep.pl $@ $^

-include $(OBJDIR)/.deps

always:
	@:

.PHONY: all always \
	handin tarball clean realclean clean-labsetup distclean grade labsetup

