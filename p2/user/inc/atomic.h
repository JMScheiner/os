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
* @brief TODO
* 
* @param source The value to swap into destination.
*
* @param destination The memory location to swap into, 	
*  IF the original value is equal to comp.
*
* @param comp If destination is equal to comp, it will be swapped.
* 
* @return One on a successful swap, zero otherwise.
*/
int atomic_cmpxchg(int source, int* destination, int comp);

#endif /* end of include guard: ATOMIC_XEF37AV5 */



