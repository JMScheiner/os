
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
   lprintf("exec_test1 executing");
   lprintf("I received %d arguments, they are", argc);
   int i;
   for (i = 0; i < argc; i++) {
      lprintf("%s", argv[i]);
   }
   lprintf("exec_test1 executing");
   lprintf("My tid before exec is 0x%x", gettid());
   char *args[5];
   args[0] = "arg1";
   args[1] = "arg2";
   args[2] = "arg3";
   args[3] = "arg4";
   args[4] = NULL;
   int err = exec("exec_test2", args);
   lprintf("FAIL - exec returned with error %d", err);
   return 1;
}
