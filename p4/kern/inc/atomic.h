/** 
* @file atomic.h
* @brief Wrappers for atomic x86 instructions. 
*
*  See chapter 7 of "intel-sys.pdf"
*
* @author Justin Scheiner
*/

#ifndef ATOMIC_XEF37AV5

#define ATOMIC_XEF37AV5

/** @brief Atomic add.
 *
 * @param dest Will be incremented by src.
 *
 * @param src The quantity to add to dest.
 *
 * @return The original value of dest.
 */
int atomic_add(int *dest, int src);

/** @brief Atomic add for the timer (uses a different type)
 *
 * @param dest Will be incremented by src.
 *
 * @param src The quantity to add to dest.
 *
 * @return The original value of dest.
 */
int atomic_add_volatile(volatile unsigned int* dest, int src);

#endif /* end of include guard: ATOMIC_XEF37AV5 */



