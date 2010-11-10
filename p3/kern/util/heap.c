/** 
* @file heap.c
* @brief A heap designed for sleeping threads. 
*  Keys on the wakeup time. 
*
*  Each TCB keeps track of its location in the heap, so we can
*     maintain heap ordering when we want to remove a thread at
*     an arbitrary time (e.g. if the thread is killed).
*
* @author Justin Scheiner
* @author Tim Wilson
* @date 2010-10-23
*/

#include <kernel_types.h>
#include <malloc.h>
#include <malloc_wrappers.h> /* srealloc */
#include <limits.h>

#define DEFAULT_HEAP_SIZE 128
#define PARENT(index) ((index) / 2)
#define LCHILD(index) (2*(index))
#define RCHILD(index) ((2*(index)) + 1)

/** 
* @brief Initialize the sleepers heap.
* 
* @param heap The address of the heap.
*/
void heap_init(sleep_heap_t* heap)
{
	heap->data = (tcb_t**)smalloc(DEFAULT_HEAP_SIZE * sizeof(tcb_t*));
	assert(heap->data != NULL);
	heap->data[0] = NULL;
	heap->index = 1;
	heap->size = DEFAULT_HEAP_SIZE;
}

/**
 * @brief Restore heap ordering by moving the element at the given
 * index up the heap.
 *
 * @param heap The heap to operate on.
 * @param index The element to bubble up the heap.
 */
void bubble_up(sleep_heap_t *heap, int index) {
	tcb_t *tcb = heap->data[index];
	int wakeup = tcb->wakeup;
	while (1) {
		int p_index = PARENT(index);
		tcb_t *parent = heap->data[p_index];
		if (p_index > 0 && parent->wakeup > wakeup)
		{
			heap->data[index] = parent;
			parent->sleep_index = index;
		}
		else {
			break;
		}
		index = p_index;
	}
	heap->data[index] = tcb;
	tcb->sleep_index = index;
}

/**
 * @brief Restore heap ordering by moving the element at the given
 * index down the heap.
 *
 * @param heap The heap to operate on.
 * @param index The element to bubble down the heap.
 */
void bubble_down(sleep_heap_t *heap, int index) {
	tcb_t *tcb = heap->data[index];
	int wakeup = tcb->wakeup;
	while (1) {
		int wake1 = LCHILD(index) < heap->index ? 
			heap->data[LCHILD(index)]->wakeup : INT_MAX;
		int wake2 = RCHILD(index) < heap->index ? 
			heap->data[RCHILD(index)]->wakeup : INT_MAX;
		if (wake1 < wakeup && wake1 <= wake2) {
			heap->data[index] = heap->data[LCHILD(index)];
			heap->data[index]->sleep_index = index;
			index = LCHILD(index);
		}
		else if (wake2 < wakeup && wake2 <= wake1) {
			heap->data[index] = heap->data[RCHILD(index)];
			heap->data[index]->sleep_index = index;
			index = RCHILD(index);
		}
		else {
			break;
		}
	}
	heap->data[index] = tcb;
	tcb->sleep_index = index;
}

/** 
* @brief Insert a TCB into the sleepers heap. 
* 
* @param heap The sleepers heap. 
* @param key The TCB to insert. 
*/
void heap_insert(sleep_heap_t* heap, tcb_t* key)
{
   /* Double the heap size if there are a lot of sleepers. */
	if(heap->index == (heap->size - 1))
	{
		heap->data = srealloc(heap->data,   
         heap->size * sizeof(tcb_t*), 2 * heap->size * sizeof(tcb_t*));
		heap->size = 2 * heap->size;
		assert(heap->data != NULL);
	}
	heap->data[heap->index] = key;
	bubble_up(heap, heap->index++);
}

/** 
* @brief Remove and return the TCB with the earliest wakeup time. 
* 
* @param heap The sleepers heap. 
* 
* @return The TCB with the earliest wakeup time.
*/
tcb_t* heap_pop(sleep_heap_t* heap)
{
	tcb_t *tcb = heap->data[1];
	heap->data[1] = heap->data[--(heap->index)];
	bubble_down(heap, 1);
	tcb->sleep_index = 0;
	return tcb;
}

/** 
* @brief Return the TCB with the earliest wakeup time.
* 
* @param heap The sleepers heap.
* 
* @return The TCB with the earliest wakeup time, or NULL if there are no
*         sleeping tcbs.
*/
tcb_t* heap_peek(sleep_heap_t* heap)
{
   return heap->index > 1 ? heap->data[1] : NULL;
}

/** 
* @brief Remove the given TCB from the heap. 
* 
* @param heap The sleepers heap. 
* @param key The TCB to remove. 
*/
void heap_remove(sleep_heap_t* heap, tcb_t* key)
{
	int index = key->sleep_index;
	int wakeup = key->wakeup;
	key->sleep_index = 0;
	heap->data[index] = heap->data[--(heap->index)];
	if (heap->data[index]->wakeup < wakeup)
		bubble_up(heap, index);
	else
		bubble_down(heap, index);
}


