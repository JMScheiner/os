
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>

int main()
{
	lprintf("My tid before exec is 0x%x", gettid());
	char *arg = NULL;
	int err = exec("gettid_test", &arg);
	lprintf("FAIL - exec returned with error %d", err);
	while (1);
	return 1;
}
