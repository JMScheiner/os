###########################################################################
# This is the include file for the make file.
###########################################################################
# You should have to edit only this file to get things to build.
#

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
# WARNING: Do not put extraneous test programs into the REQPROGS variables.
#          Doing so will put your grader in a bad mood which is likely to
#          make him or her less forgiving for other issues.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN
#
# The idle process is a really good thing to keep here.
#
410REQPROGS = idle

###########################################################################
# Mandatory programs provided in binary form by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in binary
# form and NECESSARY FOR THE KERNEL TO RUN
#
# You may move these to 410BINPROGS to test your syscall stubs once
# they exist, but we will grade you against the binary versions.
# This should be fine.
#
410REQBINPROGS = shell init

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# ignored.  Your kernel MUST RUN WITHOUT THEM.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory
#
410TESTS = startle mandelbrot cyclone agility_drill beady_test 
410TESTS += join_specific_test thr_exit_join largetest cvar_test 
410TESTS += juggle misbehave racer multitest

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory
#
STUDENTTESTS = syscall_test atomic_test queue_test mutex_test sem_test #overflow

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
SYSCALL_OBJS += halt.o misbehave.o


