
#include <stdio.h>
#include <simics.h>

#define BIGBUF_SIZE 0x8000

char bigbuf[BIGBUF_SIZE] = {0};

int main(int argc, const char *argv[])
{
   int i;
   for (i = 0; i < BIGBUF_SIZE; i++) {
      bigbuf[i] = 0xda;
   }
   return 42;
}

