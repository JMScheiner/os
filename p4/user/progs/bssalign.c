#define MAGICONSTANT 0xfc3

char magicbuffer[MAGICONSTANT];

int main(int argc, const char *argv[])
{
   magicbuffer[MAGICONSTANT-1] = 3; 
   return magicbuffer[MAGICONSTANT-1];
}



