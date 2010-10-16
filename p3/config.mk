###########################################################################
# This is the include file for the make file.
###########################################################################
# You should have to edit only this file to get things to build.

###########################################################################
# The method for acquiring project updates.
###########################################################################
# This should be "afs" for any Andrew machine, "web" for non-andrew machines
# and "offline" for machines with no network access.
#
# "offline" is strongly not recommended as you may miss important project
# updates.
#
UPDATE_METHOD = afs

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# blanked.  Your kernel MUST BOOT AND RUN if 410TESTS and STUDENTTESTS
# are blank.  It would be wise for you to test that this works.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory.
#
410TESTS = ck1

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory.
#
STUDENTTESTS = gettid_test

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = #mutex.o malloc.o atomic.o thread.o thread_helper.o thread_fork.o cond.o
#THREAD_OBJS += mutex_unlock_and_vanish.o sem.o rwlock.o

# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P3" we give you can't build libthrgrp.a.  Once you install your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
#410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = sleep.o print.o fork.o exec.o set_status.o vanish.o
SYSCALL_OBJS += wait.o task_vanish.o yield.o gettid.o deschedule.o make_runnable.o
SYSCALL_OBJS += get_ticks.o new_pages.o remove_pages.o getchar.o readline.o
SYSCALL_OBJS += set_term_color.o set_cursor_pos.o get_cursor_pos.o ls.o
SYSCALL_OBJS += halt.o misbehave.o 

###########################################################################
# Parts of your kernel
###########################################################################
#
# Kernel object files you want included from 410kern/
#
410KERNEL_OBJS = load_helper.o
#
# Kernel object files you provide in from kern/
#
KERNEL_OBJS = console.o kernel.o loader.o malloc_wrappers.o mm/mm.o mm/mm_asm.o mm/region.o
KERNEL_OBJS += handlers/handler.o handlers/handler_wrappers.o handlers/fault_handlers.o
KERNEL_OBJS += context_switch.o mode_switch.o process.o thread.o atomic.o
KERNEL)OBJS += synchro/mutex.c synchro/cond.c
KERNEL_OBJS += hashtable.o asm_helper.o mm/pagefault.o mutex.o keyboard.o timer.o
KERNEL_OBJS += handlers/exec_handler.o scheduler.o

###########################################################################
# WARNING: Do not put **test** programs into the REQPROGS variables.  Your
#          kernel will probably not build in the test harness and you will
#          lose points.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# The shell is a really good thing to keep here.  Don't delete idle
# or init unless you are writing your own, and don't do that unless
# you have a really good reason to do so.
#
410REQPROGS = idle init shell

###########################################################################
# Mandatory programs whose source is provided by you
###########################################################################
# A list of the programs in user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# Leave this blank unless you are writing custom init/idle/shell programs
# (not generally recommended).  If you use STUDENTREQPROGS so you can
# temporarily run a special debugging version of init/idle/shell, you
# need to be very sure you blank STUDENTREQPROGS before turning your
# kernel in, or else your tweaked version will run and the test harness
# won't.
#
STUDENTREQPROGS =

