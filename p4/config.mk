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
410TESTS = ck1 knife peon coolness merchant readline_basic
410TESTS += cho cho2 cho_variant deschedule_hang exec_basic
410TESTS += exec_basic_helper exec_nonexist fork_bomb
410TESTS += fork_exit_bomb fork_test1 fork_wait fork_wait_bomb
410TESTS += halt_test make_crash make_crash_helper
410TESTS += actual_wait getpid_test1 loader_test1 loader_test2 
410TESTS += mem_eat_test mem_permissions minclone_mem
410TESTS += new_pages print_basic register_test remove_pages_test1
410TESTS += remove_pages_test2 slaughter sleep_test1 stack_test1
410TESTS += wait_getpid wild_test1 yield_desc_mkrun

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory.
#
STUDENTTESTS = gettid_test exec_test1 exec_test2 fork_test tfork_test
STUDENTTESTS += autostack sleep ls_test fault_and_die malloc_test
STUDENTTESTS += agility_drill cvar_test cyclone join_specific_test
STUDENTTESTS += juggle mandelbrot startle thr_exit_join racer
STUDENTTESTS += mycho metacho zfod_test dividezero float

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = mutex.o malloc.o atomic.o thread.o thread_helper.o thread_fork.o cond.o
THREAD_OBJS += mutex_unlock_and_vanish.o sem.o rwlock.o

# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P3" we give you can't build libthrgrp.a.  Once you install your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = sleep.o print.o fork.o exec.o set_status.o vanish.o
SYSCALL_OBJS += wait.o task_vanish.o yield.o gettid.o deschedule.o make_runnable.o
SYSCALL_OBJS += get_ticks.o new_pages.o remove_pages.o getchar.o readline.o
SYSCALL_OBJS += set_term_color.o set_cursor_pos.o get_cursor_pos.o ls.o
SYSCALL_OBJS += halt.o misbehave.o swexn.o

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

KCORE_OBJS = core/kernel.o core/loader.o 
KCORE_OBJS += core/context_switch.o core/mode_switch.o core/process.o
KCORE_OBJS += core/thread.o core/scheduler.o core/stub.o core/global.o

KDRIVER_OBJS = driver/console.o driver/keyboard.o driver/timer.o

KUTIL_OBJS = util/mutex.o util/cond.o util/vstring.o util/asm_helper.o
KUTIL_OBJS += util/hashtable.o util/heap.o util/debug.o util/atomic.o
KUTIL_OBJS += util/malloc_wrappers.o

KSYSCALL_OBJS = syscall/memman.o syscall/misc.o syscall/lifecycle.o 
KSYSCALL_OBJS += syscall/threadman.o syscall/swexn.o

KHANDLER_OBJS = handlers/handler.o handlers/handler_wrappers.o handlers/fault_handlers.o
KHANDLER_OBJS += handlers/swexn_handler.o

KMM_OBJS = mm/mm.o mm/kvm.o mm/mm_asm.o mm/region.o mm/pagefault.o 

KERNEL_OBJS = $(KCORE_OBJS) $(KDRIVER_OBJS) $(KUTIL_OBJS) 
KERNEL_OBJS += $(KSYSCALL_OBJS) $(KMM_OBJS) $(KHANDLER_OBJS)

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

