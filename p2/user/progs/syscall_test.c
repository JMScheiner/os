#include <syscall.h>
#include <stdio.h>
#include <simics.h>

int main(int argc, const char *argv[])
{
	lprintf("In program.\n");
	print(4, "as\n");
	return 8;
}



