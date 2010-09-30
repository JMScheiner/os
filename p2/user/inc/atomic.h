/** 
* @file atomic.h
* @brief Wrappers for atomic x86 instructions. 
*
*	See chapter 7 of "intel-sys.pdf"
*
* @author Justin Scheiner
*/

#ifndef ATOMIC_XEF37AV5

#define ATOMIC_XEF37AV5

/** 
* @brief Atomic (thread-safe) exchange.
*
* @param source Data to swap in, will contain the original
* 	value of destination on return.
*
* @param destination The memory location to swap the original 
* 	value of source into.
*/
void atomic_xchg(int* source, int* destination);


/** 
* @brief Atomic exchange and add.
* 
* @param source Will be added to destination, and contain the
* 	original value of destination on return.
*
* @param destination Will be incremented by source, and have
* 	its original value placed in source.
*/
void atomic_xadd(int* source, int* destination);

/** 
* @brief Atomically compares and exchanges if the value is what we
* 	expect it to be.
* 
* @param source The value to swap into destination, will contain
* 	the original value of destination on failure.
*
* @param destination The memory location to swap into, 	
*  IF the original value is equal to comp.
*
* @param comp If destination is equal to comp, a swap will occur.
* 
* @return Zero on a successful swap, negative otherwise.
*/
int atomic_cmpxchg(int* source, int* destination, int comp);

int atomic_cmpset(int *dest, int src, int comp);
int atomic_add(int *dest, int src);

#endif /* end of include guard: ATOMIC_XEF37AV5 */



