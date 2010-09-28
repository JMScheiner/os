#include <stdio.h>
#include <rwlock.h>

int main(int argc, const char *argv[])
{
	//Just here to get rwlocks to build.
	rwlock_t test_lock;
	rwlock_init(&test_lock);
	return 0;
}
