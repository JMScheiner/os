float do_stuff(float a, float b);

int main(int argc, const char *argv[])
{
   float a = 3.4;
   float b = do_stuff(a, 4.5);
   return (int)b;
}
float do_stuff(float a, float b) {
   return a * b;
}
