#include <stdio.h>
#include <syscall.h>
#include <sem.h>
#include <thread.h>
#include <atomic.h>
#include <rand.h>

#define NTHREADS 30

sem_t semaphore;
mutex_t console_lock;

void* sem_test(void* arg)
{
	int col = (int)arg;
	int row = 0;
	int len, color;

	while(1)
	{
		// Try to acquire a write spot.
		sem_wait(&semaphore);
		
		len = genrand() % 10;
		color = (genrand() % 8) << 4;
		for(row = 0; row < len; row++)
		{
			mutex_lock(&console_lock);
			set_cursor_pos(row, col);
			set_term_color(color);
			printf(" ");
			mutex_unlock(&console_lock);
		}
		
		//Release the write spot, 
		sem_signal(&semaphore);
		
		//And sleep.
		sleep(1);
	}
}

int main(int argc, const char *argv[])
{
	int i;
	
	thr_init(4096);
	mutex_init(&console_lock);
	sem_init(&semaphore, 8);
	
	for(i = 0; i < 30; i++)
	{
		thr_create(sem_test, (void*)i);
	}

	return 0;
}


