
#include <syscall.h>
#include <simics.h>

int main()
{
	lprintf("My tid is %d", gettid());
	while (1) {
	}
}
