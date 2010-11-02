
#include <syscall.h>
#include <simics.h>

int main(int argc, const char *argv[])
{
	int sleep_time = 10;
   
   while(fork() == 0)
   {
      sleep_time += 7;
      sleep(sleep_time);
   }

   while(1) 
   {
      sleep(sleep_time);
		lprintf("0x%x Back!", gettid());
	}
	return 0;
}


