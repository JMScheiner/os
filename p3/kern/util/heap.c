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
* @bug Not thoroughly tested yet. 
*/

#include <kernel_types.h>
#include <malloc.h>

#define DEFAULT_HEAP_SIZE 128
#define HEAP_LEFT(index) (index * 2)
#define HEAP_RIGHT(index) ((index * 2) + 1)

#define HEAP_SWAP(x, y) {  \
   (x) = (tcb_t*)(((int)(x)) ^ ((int)(y)));        \
   (y) = (tcb_t*)(((int)(x)) ^ ((int)(y)));        \
   (x) = (tcb_t*)(((int)(x)) ^ ((int)(y)));        \
}

/** 
* @brief Initialize the sleepers heap.
* 
* @param heap The address of the heap.
*/
void heap_init(sleep_heap_t* heap)
{
   heap->data = (tcb_t**)malloc(DEFAULT_HEAP_SIZE * sizeof(tcb_t*));
   heap->data[0] = NULL;
   heap->index = 1;
   heap->size = DEFAULT_HEAP_SIZE;
}

/** 
* @brief Insert a TCB into the sleepers heap. 
* 
* @param heap The sleepers heap. 
* @param key The TCB to insert. 
*/
void heap_insert(sleep_heap_t* heap, tcb_t* key)
{
   if(heap->index == (heap->size - 1))
   {
      heap->size = 2 * heap->size;
      heap->data = realloc(heap->data, heap->size * sizeof(tcb_t*));
   }
   
   tcb_t** data;
   int index, parent;
   
   data = heap->data;
   index = heap->index;
   parent = index / 2;
   data[index] = key;
   
   for(index = heap->index; 
       index > 0 && parent > 0 && data[index]->wakeup < data[parent]->wakeup; 
       index = parent, parent = index / 2)
   {
      HEAP_SWAP(data[index], data[parent]);
      data[index]->sleep_index = index;
      data[parent]->sleep_index = parent;
   }
   
   heap->index++;
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
   tcb_t** data = heap->data;
   tcb_t* ret;
   
   int top, index, next, leftchild, rightchild;
   
   top = heap->index - 1;
   
   index = 1;
   leftchild = HEAP_LEFT(index); 
   rightchild = HEAP_RIGHT(index);
   
   ret = data[1];
   data[1] = data[top];
   
   while(rightchild < top && 
      ((data[index]->wakeup > data[leftchild]->wakeup) || 
       (data[index]->wakeup > data[rightchild]->wakeup)))
   {
      /* Bubble down left or right */
      next = data[leftchild]->wakeup < data[rightchild]->wakeup ? leftchild : rightchild;
   
      /* Swap into the next spot. */
      HEAP_SWAP(data[index], data[next]);
      data[index]->sleep_index = index;
      data[next]->sleep_index = next;
      
      /* Update indexes */
      index = next;
      leftchild = HEAP_LEFT(index); 
      rightchild = HEAP_RIGHT(index); 
   }

   if((leftchild < top) && (data[index] > data[leftchild]))
   {
      HEAP_SWAP(data[index], data[leftchild]);
      data[index]->sleep_index = index;
      data[leftchild]->sleep_index = leftchild;
   }

   heap->index = top;
   return ret;
}

/** 
* @brief Return the TCB with the earliest wakeup time.
* 
* @param heap The sleepers heap.
* 
* @return The TCB with the earliest wakeup time.
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
   tcb_t** data = heap->data;
   int top, index, next, leftchild, rightchild;
   
   top = heap->index - 1;
   
   index = key->sleep_index;
   leftchild = HEAP_LEFT(index); 
   rightchild = HEAP_RIGHT(index);
   
   data[index] = data[top];
   
   while(rightchild < top && 
      ((data[index]->wakeup > data[leftchild]->wakeup) || 
       (data[index]->wakeup > data[rightchild]->wakeup)))
   {
      /* Bubble down left or right */
      next = data[leftchild]->wakeup < data[rightchild]->wakeup ? leftchild : rightchild;
   
      /* Swap into the next spot. */
      HEAP_SWAP(data[index], data[next]);
      data[index]->sleep_index = index;
      data[next]->sleep_index = next;
      
      /* Update indexes */
      index = next;
      leftchild = HEAP_LEFT(index); 
      rightchild = HEAP_RIGHT(index); 
   }

   if((leftchild < top) && (data[index] > data[leftchild]))
   {
      HEAP_SWAP(data[index], data[leftchild]);
      data[index]->sleep_index = index;
      data[leftchild]->sleep_index = leftchild;
   }

   heap->index = top;
}


