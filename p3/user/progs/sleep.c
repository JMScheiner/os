
#include <syscall.h>
#include <simics.h>

int main(int argc, const char *argv[])
{
	int sleep_time = 10;
	if (fork() == 0)
		sleep_time += 7;
	if (fork() == 0)
		sleep_time += 7;
	if (fork() == 0)
		sleep_time += 7;
	while(1)
	{
		sleep(sleep_time);
		lprintf("Back!");
	}
	return 0;
}


