
#include <syscall.h>
#include <simics.h>

int main()
{
	lprintf("My tid is 0x%x", gettid());
	while (1) {
	}
}
