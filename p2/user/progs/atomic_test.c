#include <atomic.h>
#include <stdio.h>
#include <syscall.h>

int dest;

int main(int argc, const char *argv[])
{
	dest = 0;
	//Not much we can do to test just yet.
	int source = 0;
	while(1)
	{
		atomic_xchg(&source, &dest);
		printf("source = %d\n", source);
		sleep(1000);
		source++;	
	}

	return 0;
}

