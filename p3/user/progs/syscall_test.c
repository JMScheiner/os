#include <syscall.h>
#include <stdio.h>
#include <simics.h>

/** 
* @brief Test syscalls.
*/
int main(int argc, const char *argv[])
{
	int pid, status;
	lprintf("In program.\n");
	print(14, "Hello World!\n");

	if((pid = fork()) == 0)
	{
		printf("Hello from child!\n");
		sleep(5000);
		printf("5000 ticks have gone by!\n");
		return 15;
	}
	else
	{
		printf("Hello from parent!\n");
		wait(&status);
		printf("Child (pid = %d) exited with %d.  Should be 15.\n", pid, status);
		printf("My tid is %d\n", gettid());

		//I haven't run this long enough to see if it
		// ever shows anything not in increments of 5000
		printf("Ticks since boot: %d\n", get_ticks());
		yield(-1); 
	}

	//Run again!!!
	char* argvec[] = {"syscall_test", 0};
	exec("syscall_test", argvec);
	
	return 8;
}



