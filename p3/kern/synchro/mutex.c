
#include <mutex.h>
#include <x86/asm.h>

boolean_t interrupts_initialized = FALSE;

int mutex_init(mutex_t *mp) {
	return 0;
}

int mutex_destroy(mutex_t *mp) {
	return 0;
}

int mutex_lock(mutex_t *mp) {
	if (interrupts_initialized)
		disable_interrupts();
	return 0;
}

int mutex_unlock(mutex_t *mp) {
	if (interrupts_initialized)
		enable_interrupts();
	return 0;
}

