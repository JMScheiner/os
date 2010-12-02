#define MAGICONSTANT 0xfe1

char buf[MAGICONSTANT];

int main(int argc, const char *argv[])
{
   buf[MAGICONSTANT-1] = 3; 
   return buf[MAGICONSTANT-1];
}

