
#include <syscall.h>
#include <simics.h>
#include <stdio.h>

int main()
{
	printf("My tid is 0x%x", gettid());
   return 7;
}
