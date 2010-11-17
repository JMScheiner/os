
#include <stdio.h>
#include <syscall.h>
#include <assert.h>
#include <simics.h>

static int N = 100;
static int nprogs = 3;
static char* progs[] = {"cho", "cho2", "cho_variant"};

int main(int argc, const char *argv[])
{
   int i, j, pid, status;
   for (i = 0; i < N; i++)
   {
      for(j = 0; j < nprogs; j++)
      {
         int pid;
         pid = fork();
         if(pid == 0)
         {
            char* args[2];
            args[0] = progs[j];
            args[1] = 0;
            exec(progs[j], args);
         }
      }
      sleep(10);
   }

   for(i = 0; i < N * nprogs; i++)
   {
      lprintf(" Calling wait. ");
      pid = wait(&status);
      if(pid < 0)
      {
         lprintf(" ********** FAILURE ********** ");
         MAGIC_BREAK;
      }
      lprintf("Collected status %d from process %d", status, pid);
   }

   return 42;
}


