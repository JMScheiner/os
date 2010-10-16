
#include <mutex.h>

int mutex_init(mutex_t *mp) {
	return 0;
}

int mutex_destroy(mutex_t *mp) {
	return 0;
}

int mutex_lock(mutex_t *mp) {
	disable_interrupts();
	return 0;
}

int mutex_unlock(mutex_t *mp) {
	enable_interrupts();
	return 0;
}

