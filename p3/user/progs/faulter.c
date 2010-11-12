/* Includes */
#include <syscall.h>  /* was for exit */
#include <stdlib.h>   /* for exit */
#include <stdio.h>    /* for lprintf */
#include "410_tests.h"

int rand_address = 0x12345678;

/* Main */
int main(int argc, char *argv[])
{
	int i;
	int status;
	int pid;
	for (i = 0; i < 10; i++)
		fork();
	while (1) {
		if (fork() == 0) {
			rand_address = rand_address * gettid() + 0x342;
			*(int *)rand_address = 0;
		}
		else {
			if ((pid = wait(&status)) < 0) {
				MAGIC_BREAK;
			}
			lprintf("Collected status %d from process %d", status, pid);
		}
	}

	exit( -1 );
}
