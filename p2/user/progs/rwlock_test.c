#include <stdio.h>
#include <rwlock.h>
#include <thread.h>
#include <simics.h>
#include <syscall.h>

#define BUF_SIZE 4096


char shared_buf[13];

mutex_t console_lock;
rwlock_t rwlock;
int readers = 0;

void* read(void* arg)
{
	int n = (int)arg;
	while(1)
	{
		rwlock_lock(&rwlock, RWLOCK_READ);
		printf("%c", shared_buf[n]);
		rwlock_unlock(&rwlock);
	}
}

void* write(void* arg)
{
	int i, n;
	
	n = (int)arg;
	while(1)
	{
		rwlock_lock(&rwlock, RWLOCK_WRITE);
		for(i = 0; i < 13; i++)
		{
			shared_buf[i] = 'A' + i + n;
		}
		rwlock_unlock(&rwlock);
		sleep(2);
	}
}

int main(int argc, const char *argv[])
{
	//Just here to get rwlocks to build.
	rwlock_init(&rwlock);
	mutex_init(&console_lock);
	int i;
	thr_init(4096);
	
	thr_create(write, (void*)13);
	thr_create(write, (void*)0);

	for (i = 0; i < 13; i++) 
		thr_create(read, (void*)i );

	return 0;
}



