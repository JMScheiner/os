
#include <hashtable.h>

/** @brief A roughly doubling sequence of prime numbers suitable
 * for use as table sizes in a hashtable. */
unsigned int prime_hashtable_sizes[] = 
  {(1 << 5)-1,
   (1 << 6)-3,
   (1 << 7)-1,
   (1 << 8)-5,
   (1 << 9)-3,
   (1 << 10)-3,
   (1 << 11)-9,
   (1 << 12)-3,
   (1 << 13)-1,
   (1 << 14)-3,
   (1 << 15)-19,
   (1 << 16)-15,
   (1 << 17)-1,
   (1 << 18)-5,
   (1 << 19)-1,
   (1 << 20)-3,
   (1 << 21)-9,
   (1 << 22)-3,
   (1 << 23)-15,
   (1 << 24)-3,
   (1 << 25)-39,
   (1 << 26)-5,
   (1 << 27)-39,
   (1 << 28)-57,
   (1 << 29)-3,
   (1 << 30)-35};

/** @brief Default identity hash function to use for simple hash tables
 *
 * @param key The key to hash
 *
 * @return The orginal key (as an unsigned value)
 */
unsigned int default_hash(int key) {
	return key;
}
