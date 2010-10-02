#include <stdio.h>
#include <rwlock.h>
#include <thread.h>
#include <simics.h>
#include <syscall.h>

#define NTHREADS 100
#define BUF_SIZE 4096


char shared_buf[BUF_SIZE];

mutex_t console_lock;
rwlock_t rwlock;
int readers = 0;

void* read(void* arg)
{
	int i;
	while(1)
	{
		rwlock_lock(&rwlock, RWLOCK_READ);
		mutex_lock(&console_lock);
		for(i = 0; i < 5; i++)
			printf("%c", shared_buf[0]);
		mutex_unlock(&console_lock);
		rwlock_unlock(&rwlock);
	}
}

void* write(void* arg)
{
	int i, j, n;
	
	n = (int)arg;
	while(1)
	{
		for(i = 0; i < 13; i++)
		{
			rwlock_lock(&rwlock, RWLOCK_WRITE);
			
			for (j = 0; j < BUF_SIZE; j++) 
				shared_buf[j] = 'A' + i + n;
			
			rwlock_unlock(&rwlock);
			sleep(1);
		}
	}
}

int main(int argc, const char *argv[])
{
	//Just here to get rwlocks to build.
	rwlock_init(&rwlock);
	mutex_init(&console_lock);
	int i;
	thr_init(4096);
	for (i = 0; i < NTHREADS; i++) 
	{
		thr_create(read, NULL);
	}

	thr_create(write, (void*)13);

	return 0;
}



