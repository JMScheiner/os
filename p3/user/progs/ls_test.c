
#include <stdio.h>
#include <syscall.h>
#include <string.h>
#include <simics.h>

#define BUFSIZE 0x500
char buf[BUFSIZE];

int main(int argc, const char *argv[])
{
   int size;
   char* bufptr = buf;
   memset(buf, 0xff, BUFSIZE);
   
   ls(BUFSIZE, bufptr);
   
   int n = 1;
   while((size = strlen(bufptr)) > 0)
   {
      printf("%s, ", bufptr);
      bufptr += size + 1; 
      if(!(n++ % 4)) printf("\n");
   }

   return 152352;
}

