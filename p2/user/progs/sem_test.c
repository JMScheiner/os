#include <stdio.h>
#include <sem.h>

int main(int argc, const char *argv[])
{
	sem_t semaphore;
	sem_init(&semaphore, 1);
	return 0;
}
