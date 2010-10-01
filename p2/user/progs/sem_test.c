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
		len = genrand() % 10;
		color = (genrand() % 8) << 4;
		for(row = 0; row < len; row++)
		{
			set_cursor_pos(row, col);
			set_term_color(color);
			printf(" ");
		}
		sleep(1);
	}
}

int main(int argc, const char *argv[])
{
	int i;
	sem_init(&semaphore, 8);
	thr_init(4096);
	
	for(i = 0; i < 30; i++)
	{
		thr_create(sem_test, (void*)i);
	}

	while(1) { }

	return 0;
}


