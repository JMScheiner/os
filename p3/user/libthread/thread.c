/** @file thread.c
 *
 * @brief Thread management API for user programs
 *
 * @author Tim Wilson
 */

#include <thread.h>

#include <mutex.h>
#include <cond.h>
#include <thr_internals.h>
#include <thread_helper.h>
#include <thread_fork.h>
#include <hashtable.h>
#include <syscall.h>
#include <stdio.h>
#include <types.h>
#include <assert.h>
#include <stdlib.h>
#include <simics.h>

/** @brief The alignment of the stack pointer. This must be a power of 2. */
#define ESP_ALIGN 4

/** @brief The size of the kill stack. Must be large enough to hold a call to
 * free. */
#define KILL_STACK_SIZE 1024

/** @brief The size of the int stack. Must be large enough to hold the saved
 * registers from an INT call. */
#define INT_STACK_SIZE 32

/** @brief The size of a child stack. Initialized in thr_init. */
static unsigned int user_stack_size;

/** @brief The size we need to allocate for a child's stack to assure the stack
 * can be properly aligned. */
static unsigned int alloc_stack_size;

/** @brief Has thr_init been called? */
static boolean_t initialized = FALSE;

/** @brief The logical top of the shared kill stack. */
static char kill_stack_top[KILL_STACK_SIZE + 2*ESP_ALIGN - 1];

/** @brief The base of the shared kill stack. A thread jumps to this stack just
 * before it exits so it can free its own stack. */
static char *kill_stack;

/** @brief A safe place for traps to execute, so we don't overwrite part of 
 * the kill stack. */
static char int_stack_top[INT_STACK_SIZE + 2*ESP_ALIGN - 1];

/** @brief A pointer to the base of the kill_stack. */
static char* int_stack;

/** @brief A lock on the kill stack. */
static mutex_t kill_stack_lock;

/** @brief The tid of the thread currently on the kill stack. */
static int kill_stack_tid = NULL_TID;

/** @brief Define a hashtable type from int to tcb_t *. */
DEFINE_HASHTABLE(hashtable_t, int, tcb_t *);

/** @brief A table mapping tids to thread control blocks. The main parent
 * thread will be in this table. */
static hashtable_t tid_table;

/** @brief A lock for the tid_table. */
static mutex_t tid_table_lock;

/** @brief The maximum stack address a child thread could have. */
static char *max_child_stack_addr = NULL;

/** @brief A lock to synchronize writes to the maximum stack address. */
static mutex_t max_child_stack_addr_lock;

/** @brief The thread control block for the main parent thread. */
static tcb_t *main_thread;

static unsigned int hash(int key);

/** @brief Align an address by rounding down to the nearest address containing
 * a tcb pointer. */
#define ALIGN_DOWN_TCB(address) \
   (user_stack_size * ((unsigned int)(address) / user_stack_size))

/** @brief Align an address by rounding up to the nearest address containing
 * a tcb pointer. */
#define ALIGN_UP_TCB(address) \
   (user_stack_size * (((unsigned int)(address) + user_stack_size - 1) / user_stack_size))

/** @brief Align an address down to satisfy the word alignment requirements of
 * %esp. */
#define ALIGN_DOWN(address) \
   (((unsigned int)(address)) & ~(ESP_ALIGN - 1))

/** @brief Align an address up to satisfy the word alignment requirements of
 * %esp. */
#define ALIGN_UP(address) \
   ((((unsigned int)(address) - 1) | (ESP_ALIGN - 1)) + 1)

/** @brief Initialize the thread library
 *
 * Also, create a thread control block for the main parent thread.
 *
 * @param size The amount of stack space in bytes that each thread will
 *        have available
 *
 * @return 0 on success, less than 0 on error
 */
int thr_init(unsigned int size) {
   assert(!initialized);
   int ret = 0;
   initialized = TRUE;

   /* User stack is large enough to hold the stack, a pointer to the tcb
    * and a pointer to the stack. */
   user_stack_size = size + sizeof(tcb_t *) + sizeof(char *);
   alloc_stack_size = ALIGN_UP(2 * user_stack_size);


   /* Initialize the main_thread block. */
   main_thread = (tcb_t *)calloc(1, sizeof(tcb_t));
   if(main_thread == NULL)
   {
      lprintf("NULL NULL NULL");
      return -1;
   }
   main_thread->tid = gettid();

   mutex_debug_print("Initializing main thread lock...");
   ret |= mutex_init(&(main_thread->lock));

   mutex_debug_print("Initializing main thread condition variables...");
   ret |= cond_init(&(main_thread->init_signal));
   ret |= cond_init(&(main_thread->exit_signal));

   main_thread->initialized = TRUE;
   main_thread->exited = FALSE;

   /* Initialize the tid table and its lock. */
   STATIC_INIT_HASHTABLE(hashtable_t, tid_table, hash);

   thread_debug_print("Main: inserting %d -> %p into tid table.", main_thread->tid, main_thread);
   HASHTABLE_PUT(hashtable_t, tid_table, main_thread->tid, main_thread);
   
   mutex_debug_print("Initializing tid table lock...");
   ret |= mutex_init(&tid_table_lock);
   
   /* Initialize the max stack address lock. */
   mutex_debug_print("Initializing max child stack address lock...");
   ret |= mutex_init(&max_child_stack_addr_lock);

   /* Initialize the kill stack and its lock. */
   kill_stack = kill_stack_top + KILL_STACK_SIZE + ESP_ALIGN - 1;
   kill_stack = (char *) ALIGN_DOWN(kill_stack);
   
   mutex_debug_print("Initializing kill stack lock...");
   ret |= mutex_init(&kill_stack_lock);

   /* Initialized the "int" stack. Since interrupts are atomic, 
    *    and all information gets copied to the kernel stack, no need to lock. */
   int_stack = int_stack_top + INT_STACK_SIZE + ESP_ALIGN - 1;
   int_stack = (char *) ALIGN_DOWN(int_stack);
   
   assert(ret == 0);
   return ret;
}

/************************ Life cycle of a thread **************************
 * Parent                              * Child                            *
 **************************************************************************
 * Initializes child tcb in thr_create *                                  *
 * Call thr_fork                       *                                  *
 * Wait for child to init              * Jump to new stack                *
 *                                     * Call thr_child_init              *
 *                                     * Get tid, add self to tables      *
 *                                     * Signal parent                    *
 * Continue                            * Call func                        *
 * ...                                 * ...                              *
 * Wait for child to die               * Call thr_exit                    *
 *                                     * Set status                       *
 *                                     * Jump to kill stack               *
 *                                     * Free own stack, signal exit      *
 * Get status, free child              * Jump to int stack                *
 *                                     * Call vanish                      *
 **************************************************************************/

/** @brief Create a new thread running the given function
 *
 * @param func The function the new thread will begin running
 *
 * @param arg The sole argument to func
 *
 * @return The thread ID of the new thread on success, less than 0 on error
 */
int thr_create(void *(*func)(void *), void *arg) 
{
   assert(initialized);
   int ret = -1;
   
   /* Create a thread control block and initialize its stack, mutex, and
    * condition variable. */
   tcb_t *tcb = (tcb_t *)calloc(1, sizeof(tcb_t));
   assert(tcb);
   tcb->exited = FALSE;
   tcb->initialized = FALSE;
   
   mutex_debug_print("Initializing new tcb lock...");
   if(mutex_init(&tcb->lock)) {
      goto fail_mutex;
   }
   mutex_debug_print("Initializing new tcb condition variable...");
   if (cond_init(&tcb->init_signal)) {
      goto fail_init_cond;
   }
   if (cond_init(&tcb->exit_signal)) {
      goto fail_exit_cond;
   }
   char *stack = (char *)malloc(alloc_stack_size);
   assert(stack);

   /* Compute the base address of the child stack and place a pointer to the tcb
    * above it. */
   char *stack_base = stack + alloc_stack_size - 4;
   stack_base = (char *)ALIGN_DOWN_TCB(stack_base);
   *(tcb_t **)stack_base = tcb;
   stack_base -= 4;
   *(char **)stack_base = stack;
   stack_base -= 4;
   
   /* Update the max child stack address. */
   assert(mutex_lock(&max_child_stack_addr_lock) == 0);
   if (max_child_stack_addr < (char *)stack_base) {
      max_child_stack_addr = (char *)stack_base;
   }
   assert(mutex_unlock(&max_child_stack_addr_lock) == 0);
   
   /* Fork the new child thread. If we fail, undo all the initialization we've
    * done. */
   ret = thread_fork(func, arg, stack_base, tcb);
   thread_debug_print("[%d] Got back %d from thread_fork.", thr_getid(), ret);
   if (ret >= 0) {
      return ret;
   }
   thread_debug_print(" ******** Failed to create child thread ******** ");

   free(stack);
   assert(cond_destroy(&tcb->exit_signal) == 0);
fail_exit_cond:
   thread_debug_print(" ******** Failed to initialized condition variable ******* ");
   assert(cond_destroy(&tcb->init_signal) == 0);
fail_init_cond:
   thread_debug_print(" ******** Failed to initialized condition variable ******* ");
   assert(mutex_destroy(&tcb->lock) == 0);
fail_mutex:
   thread_debug_print(" ******** Failed to initialized mutex variable ******* ");
   free(tcb);
   return ret;
}

/** @brief Initialize a new child thread. 
 *
 * The tid still needs to be fetched, and the child needs to add its tcb to the
 * tid table.
 *
 * @param func The function this child should execute.
 * @param arg The argument to the function the child will execute.
 * @param tcb The partial thread control block of the thread. 
 */
void thr_child_init(void *(*func)(void*), void* arg, tcb_t* tcb) {
   assert(tcb);
   tcb->tid = gettid();
   
   assert(mutex_lock(&tid_table_lock) == 0);
   /*thread_debug_print("Thread: inserting %d -> %p into tid table.", tcb->tid, tcb);*/
   HASHTABLE_PUT(hashtable_t, tid_table, tcb->tid, tcb);
   assert(mutex_unlock(&tid_table_lock) == 0);
   
   /* Notify the parent that we've fully initialized ourself. */
   assert(mutex_lock(&tcb->lock) == 0);
   tcb->initialized = TRUE;
   assert(mutex_unlock(&tcb->lock) == 0);
   assert(cond_signal(&tcb->init_signal) == 0);
   
   thread_debug_print("[%d] Done initializing. About to enter function.", tcb->tid);
   thr_exit(func(arg));
}

/** @brief Wait for our child to completely initialize itself.
 *
 * We must wait for our child to move to its new stack before we continue.
 * Otherwise we might clobber parts of our stack before our child leaves it.
 * Additionally we must wait for our child to set its tid in its tcb and add
 * itself to the two tcb tables.
 *
 * @param tcb The tcb of the child we're waiting on.
 */
void wait_for_child(tcb_t *tcb) {
   assert(tcb);
   assert(mutex_lock(&tcb->lock) == 0);

   while (!tcb->initialized) 
   {
      assert(cond_wait(&tcb->init_signal, &tcb->lock) == 0);
   }
   assert(mutex_unlock(&tcb->lock) == 0);
}  

/** @brief Wait for the specified thread to exit and collect its status
 *
 * @param tid The ID of the thread to wait for. It is an error to pass a tid
 *        that has not yet been created by thr_create
 *
 * @param statusp If not NULL, the status returned by the exiting thread will
 *        be placed here
 *
 * @return 0 on success, less than 0 on error
 */
int thr_join(int tid, void **statusp) {
   assert(initialized);
   tcb_t *tcb = NULL;

   /* Get the tcb corresponding to this tid from the tid_table. */
   assert(mutex_lock(&tid_table_lock) == 0);
   HASHTABLE_GET(hashtable_t, tid_table, tid, tcb);

   /* If we're not the first to join on this thread, exit promptly. */
   if (tcb) {
      /* Remove the joined thread from the tid_table. */
      HASHTABLE_REMOVE(hashtable_t, tid_table, tid, tcb);
      assert(mutex_unlock(&tid_table_lock) == 0);

      /* Wait for the joined thread to signal completion. */
      assert(mutex_lock(&tcb->lock) == 0);
      while (!tcb->exited) {
         assert(cond_wait(&tcb->exit_signal, &tcb->lock) == 0);
      }
      assert(mutex_unlock(&tcb->lock) == 0);

      /* If the thread is inturrupted between setting exited to TRUE and
       * signalling us AND we are made runnable, we could get here before the
       * child calls cond_signal. The thread will have the kill_stack locked, so
       * lock the kill_stack so we don't free our child before they cond_signal.
       */
      assert(mutex_lock(&kill_stack_lock) == 0);

      /* Copy the joined thread's status. */
      if (statusp) {
         *statusp = tcb->statusp;
      }

      /* Deallocate the joined thread. */
      assert(mutex_destroy(&tcb->lock) == 0);
      assert(cond_destroy(&tcb->init_signal) == 0);
      assert(cond_destroy(&tcb->exit_signal) == 0);
      free(tcb);
      
      assert(mutex_unlock(&kill_stack_lock) == 0);
      return 0;
   }
   assert(mutex_unlock(&tid_table_lock) == 0);
   return -1;
}

/** @brief Exit this thread with the given status
 *
 * @param status An opaque packet to pass to a waiting thread that called 
 *        thr_join on this thread
 */
void thr_exit(void *status) {
   assert(initialized);
   
   /* Get tcb from stack table and remove it. */
   tcb_t **tcb_addr = thr_gettcb();
   tcb_t *tcb = *tcb_addr;
   int tid = tcb->tid;
   char **stack_addr = (char **)(tcb_addr - 1);

   /* Set our status. */
   tcb->statusp = status;
   assert(mutex_lock(&tcb->lock) == 0);
   tcb->exited = TRUE;
   assert(mutex_unlock(&tcb->lock) == 0);
   assert(cond_signal(&tcb->exit_signal) == 0);
   
   thread_debug_print("[%d] About to exit.", tcb->tid);
   if (tcb == main_thread) {
      /* If we're the main thread, we can simply disappear. */
      vanish();
   }
   else 
   {
      /* Otherwise we must free our stack. We call free from the stack we are
       * deallocating, so we must jump to the kill_stack dedicated for this
       * purpose. */
      thread_debug_print("[%d] Trying to lock kill stack.", tid);
      mutex_lock(&kill_stack_lock);
      thread_debug_print("[%d] Owns kill stack lock.", tid);
      switch_stacks_and_vanish(tid, *stack_addr, kill_stack);
   }
   // Shouldn't reach here
   assert(FALSE);
}

/** @brief Return the thread ID of the currently running thread
 *
 * @return the current thread's ID
 */
int thr_getid(void) {
   tcb_t **tcb_addr = thr_gettcb();
   if (tcb_addr && *tcb_addr) {
      return (*tcb_addr)->tid;
   }
   else {
      return kill_stack_tid;
   }
}

/** @brief Defer execution of this thread in favor of another
 *
 * @param tid The thread ID of the thread to defer to. If tid is -1, defer to
 *        any thread. It is an error if the given thread is not runnable
 *
 * @return 0 on success, less than 0 on error
 */
int thr_yield(int tid) {
   return yield(tid);
}

/** @brief Get our tcb from the stack_list
 *
 * @return A pointer to our tcb
 */
tcb_t **thr_gettcb() {
   assert(initialized);
   char *stack_addr = get_addr();

   /* If this is being called from the kill stack, return the tcb of the
    * thread using the kill stack. */
   if (kill_stack_top <= stack_addr && stack_addr <= kill_stack) {
      return NULL;
   }

   /* If our address is higher than the address of any child stack, then we must
    * be the parent thread. */
   if (stack_addr > max_child_stack_addr) {
      return &main_thread;
   }

   tcb_t **tcb_addr = (tcb_t **)ALIGN_UP_TCB(stack_addr);
   return tcb_addr;
}

/** @brief Identity hash function
 *
 * @param key The key to hash
 *
 * @return The hash value
 */
static unsigned int hash(int key) {
   return (unsigned int)key;
}

/** @brief Free our own stack and vanish
 *
 * @param tid Our tid.
 * @param old_stack The stack we used to be executing on.
 */
void clean_up_thread(int tid, char *old_stack) 
{
   kill_stack_tid = tid;
   free(old_stack);

   /* After unlocking the kill_stack mutex, we must vanish immediately without
    * touching the stack again. This means we can't even try to return from
    * mutex_unlock. Use the int_stack to handle information put on the user stack 
    * by INT */
   thread_debug_print("[%d] Has freed his stack, and is releasing the kill stack lock.", 
      kill_stack_tid);
   mutex_unlock_and_vanish(&kill_stack_lock, int_stack);
}
