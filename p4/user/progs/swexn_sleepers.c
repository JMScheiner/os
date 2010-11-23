/**
 * @file swexn_sleepers.c
 * @brief Tests concurrent use of exception stacks in multithreaded
 * programs is controlled.
 *
 * @author tjwilson
 * @for swexn p4
 * @covers swexn
 */

#include <thread.h>
#include <stddef.h>
#include <stdlib.h>
#include <syscall.h>

#include "simics.h"

#define STACK_SIZE 2048

/* Create 3 exception stacks to be shared by all threads. */
char swexn_stack1[STACK_SIZE];
char swexn_stack2[STACK_SIZE];
char swexn_stack3[STACK_SIZE];
#define SWEXN_STACK1 ((void*)(swexn_stack1 + STACK_SIZE - 8))
#define SWEXN_STACK2 ((void*)(swexn_stack2 + STACK_SIZE - 8))
#define SWEXN_STACK3 ((void*)(swexn_stack3 + STACK_SIZE - 8))

/* A list of tick values so threads can sleep for varying lengths of time. */
int tick_pos = 0;
int tick_arr[] = {3, 11, 7, 4, 10, 2, 3, 5, 14, 8, 2, 6, 9};
int stack_pos = 0;
void *stack_arr[] = {SWEXN_STACK1, SWEXN_STACK2, SWEXN_STACK3};

/* Default number of threads and repetitions to use. */
int threads = 25;
int reps = 25;


void handler(void *arg, ureg_t *uregs)
{
   int ret;
   int tid = gettid();
   void *stack = arg;
   tick_pos = (tick_pos + 1) % (sizeof(tick_arr) / sizeof(tick_arr[0]));
   int ticks = tick_arr[tick_pos];
   int i;

   for (i = 0; i < 3; i++) {
      lprintf("Thread %d on stack %p for %d ticks", 
            tid, stack, (3-i)*ticks);
      sleep(ticks);
      assert(tid == gettid());
   }
   lprintf("Thread %d done on stack %p", tid, stack);

   // Jump over the null dereference that caused the fault
   lprintf("Faulting instruction was %x", uregs->esp);
   uregs->eip += 2;
   stack_pos = (stack_pos + 1) % (sizeof(stack_arr) / sizeof(stack_arr[0]));

   stack = stack_arr[stack_pos];
   ret = swexn(stack, handler, stack, uregs);
   if (ret < 0)
   {
      lprintf("Failed to install handler");
      exit(-2);
   }
   else {
      lprintf("Failed to leave exception stack");
      exit(-2);
   }
}

void *dumb(void *arg) {
   int i;
   stack_pos = (stack_pos + 1) % (sizeof(stack_arr) / sizeof(stack_arr[0]));
   void *stack = stack_arr[stack_pos];
   int ret = swexn(stack, handler, stack, NULL);
   if (ret < 0)
   {
      lprintf("Failed to install first handler");
      exit(-2);
   }

   for (i = 0; i < reps; i++) {
      lprintf("Thread %d about to do something stupid", gettid());
      ret += *(int *)NULL;
      lprintf("Yay, back from exception handler in iteration %d", i);
   }

   thr_exit(arg);
   // Force the compiler to keep the null dereference.
   lprintf("%d", ret);
   assert(0);
   return arg;
}

int main(int argc, char **argv)
{
   int i;
   switch (argc) {
      case 3:
         i = atoi(argv[2]);
         if (i > 0)
            reps = i;
      case 2:
         i = atoi(argv[1]);
         if (i > 3)
            threads = i;
      default:
         break;
   }

   thr_init(STACK_SIZE);
   for (i = 0; i < threads; i++) {
      if (thr_create(dumb, (void *)i) < 0) {
         lprintf("Failed to create thread %d", i);
         exit(-2);
      }
   }

   thr_exit(0);
   assert(0);
   return 0;
}
