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
	/*
	elem_queue q = STATIC_INIT_QUEUE(elem_queue);
	elem_queue* q_ptr = &q;

	elem_t node1;
	elem_t* n = &node1;
	
	n->a = 2;
	n->b = 4;

	ENQUEUE_FIRST(q_ptr, n);
	*/

	/*

	elem_queue* q_ptr = &q;
	elem_t* n1 = &node1;
	elem_t* n2 = &node2;
	elem_t* n3 = &node3;
	
	ENQUEUE_LAST(q_ptr, n1);
	ENQUEUE_FIRST(q_ptr, n2);
	ENQUEUE_FIRST(q_ptr, n3);

	elem_t* i;

	FOREACH(q_ptr, i)
	{
		printf("a = %d, b = %d\n", i->a, i->b);
	}*/
		
	return 0;
}
