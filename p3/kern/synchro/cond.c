/** @file cond.c
 *
 * @brief A very basic condition variable that supports only one waiting
 * thread.
 *
 * @author Tim Wilson (tjwilson)
 * @author Justin Scheiner (jscheine)
 */
#include <types.h>
#include <cond.h>
#include <scheduler.h>
#include <thread.h>

/**
 * @brief Initialize a condition variable.
 *
 * @param cv The condition variable to initialize.
 */
void cond_init(cond_t *cv) {
	assert(cv);
	cv->initialized = TRUE;
	cv->tcb = NULL;
}

/**
 * @brief Destroy a condition variable by uninitializing cv.
 *
 * @param cv The condition variable to destroy.
 */
void cond_destroy(cond_t *cv) {
	assert(cv);
	assert(cv->tcb == NULL);
	cv->initialized = FALSE;
}

/**
 * @brief Wait for another thread to call cond_signal on this thread. This must
 * be called with interrupts disabled, otherwise we might block and never be
 * rescheduled if a signal preempts us.
 *
 * This should be used with the following pattern.
 *
 * disable_interrupts();
 * if (we_need_to_wait) {
 *   cond_wait(&signal);
 * }
 * enable_interrupts();
 *
 * @param cv The condition variable to wait on.
 */
void cond_wait(cond_t *cv) {
	assert(cv);
	assert(cv->initialized);

	cv->tcb = get_tcb();
	scheduler_block_me(cv->tcb);
}

/**
 * @brief Signal the waiting thread if one exists. This must be called with
 * interrupts disabled (typically from a trap gate handler).
 *
 * @param cv The condition varaible to signal.
 */
void cond_signal(cond_t *cv) {
	assert(cv);
	assert(cv->initialized);

	if (cv->tcb != NULL) {
		scheduler_make_runnable(cv->tcb);
		cv->tcb = NULL;
	}
}

