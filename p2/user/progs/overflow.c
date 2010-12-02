#include <stdio.h>
#include <syscall.h>

void overflow(char* p)
{
	char asdf[0X100];
	overflow(asdf);
}

int main(int argc, const char *argv[])
{
	new_pages((void*)0xff000000, 0x1000);

	overflow(NULL);
	return 0;
}
