#include <queue.h>
#include <stdio.h>

struct elem
{
	int a;
	int b;
	struct elem* next;
	struct elem* prev;
};

typedef struct elem elem_t;

DEFINE_QUEUE(elem_queue, elem_t*);

int main(int argc, const char *argv[])
{
	printf("Starting up.\n");

	/*elem_queue q;	
	STATIC_INIT_QUEUE(&q);

	elem_t n1;
	elem_t n2;
	elem_t n3;
	
	n1.a = 1;
	n1.b = 1;
	n2.a = 2;
	n2.b = 2;
	n3.a = 3;
	n3.b = 3;

	ENQUEUE_FIRST(&q, (&n1));
	ENQUEUE_FIRST(&q, (&n2));
	ENQUEUE_FIRST(&q, (&n3));
	
	elem_t* i;

	FOREACH(&q, i)
	{
		printf("a = %d, b = %d\n", i->a, i->b);
	}*/
		
	return 0;
}
