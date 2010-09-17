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
		sleep(10000);
		printf("10000 ticks have gone by!\n");
		return 15;
	}
	else
	{
		printf("Hello from parent!\n");
		wait(&status);
		printf("Child (pid = %d) exited with %d.  Should be 15.\n", pid, status);
	}

	//Run again!!!
	char* argvec[] = {"syscall_test", 0};
	exec("syscall_test", argvec);
	
	return 8;
}



