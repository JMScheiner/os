
#include <syscall.h>
#include <simics.h>

int main(int argc, char **argv)
{
	lprintf("I received %d arguments, they are", argc);
	int i;
	MAGIC_BREAK;
	for (i = 0; i < argc; i++) {
		lprintf("%s", argv[i]);
	}
	while (1);
}
