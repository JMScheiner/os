#include <stdio.h>
#include <mutex.h>

int main(int argc, const char *argv[])
{
   printf("Starting up...\n");
   mutex_t mutex;
   
   mutex_init(&mutex);
   printf("init'd...\n");

   mutex_lock(&mutex);
   printf("lock'd...\n");
   
   mutex_unlock(&mutex);
   printf("unlock'd...\n");
   
   mutex_destroy(&mutex);
   printf("destroy'd...\n");
   return 1;
}


