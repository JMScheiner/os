
#include <syscall.h>
#include <simics.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	printf("I received %d arguments, they are", argc);
	int i;
	for (i = 0; i < argc; i++) {
		lprintf("%s", argv[i]);
	}
   return 8;
}
