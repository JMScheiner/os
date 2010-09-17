#include <syscall.h>
#include <stdio.h>
#include <simics.h>

int main(int argc, const char *argv[])
{
	int pid, status;
	lprintf("In program.\n");
	print(14, "Hello World!\n");

	if((pid = fork()) == 0)
	{
		printf("Hello from child!\n");
		return 15;
	}
	else
	{
		printf("Hello from parent!\n");
		wait(&status);
		printf("Child (pid = %d) exited with %d.  Should be 15.\n", pid, status);

	}
	return 8;
}



