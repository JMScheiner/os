#include <atomic.h>
#include <stdio.h>
#include <syscall.h>

int dest;

int main(int argc, const char *argv[])
{
	dest = 1;
	//Not much we can do to test just yet.
	int val;
	while(1)
	{
		val = atomic_add(&dest, 1);
		printf("val = %d\n", val);
		sleep(1000);
	}

	return 0;
}

