/** @file hashtable.h
 *
 * @brief Macros for generating and manipulating a hashtable
 *
 * @author Tim Wilson (tjwilson)
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <assert.h>
#include <stddef.h>
#include <simics.h>

/** @brief List of primes below powers of 2 to be used as table sizes. */
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

/** @def DEFINE_HASHTABLE(hashtable_type, key_type, val_type)
 *
 * @brief Generate the definition for a new type of hashtable mapping keys to
 *        values of the given types.
 *
 * @param hashtable_type The resulting pointer type of the hashtable.
 *        hashtable_type must be an undefined type.
 *
 * @param key_type The type of the keys to be used in this hashtable.
 *
 * @param val_type The type of the values to be used in this hashtable.
 */
#define DEFINE_HASHTABLE(hashtable_type, key_type, val_type) \
	struct hashtable_type##_table_struct { \
		size_t size; \
		size_t table_index; \
		unsigned int (*hash)(key_type); \
		struct hashtable_type##_link_struct { \
			key_type key; \
			val_type val; \
			struct hashtable_type##_link_struct *next; \
		} **table; \
	}; \
	typedef struct hashtable_type##_table_struct hashtable_type

/** @def STATIC_INIT_HASHTABLE(hashtable_type, hashtable_name, hash_function)
 *
 * @brief Statically instantiate a new empty hashtable that will use the given
 *        hash function. The hashtable should eventually be passed to 
 *        STATIC_FREE_HASHTABLE.
 *
 * NOTE: Although the hashtable is itself allocated statically, internal parts
 * of it need to expand dynamically. To dispose of the hashtable,
 * STATIC_FREE_HASHTABLE must be called to prevent a memory leak.
 *
 * @param hashtable_type The type of the hashtable to instantiate.
 *
 * @param hashtable_name The name of the hashtable to instantiate.
 *
 * @param hash_function The function to hash keys to integers for this
 *        hashtable.
 */
#define STATIC_INIT_HASHTABLE(hashtable_type, hashtable_name, hash_function) \
	do { \
		(hashtable_name).size = 0; \
		(hashtable_name).table_index = 0; \
		(hashtable_name).hash = (hash_function); \
		(hashtable_name).table = (struct hashtable_type##_link_struct **)calloc( \
				prime_hashtable_sizes[0], sizeof(struct hashtable_type##_link_struct *)); \
		assert((hashtable_name).table); \
	} while (0)

/** @def DYNAMIC_INIT_HASHTABLE(hashtable_type, hashtable_name, hash_function)
 *
 * @brief Dynamically instantiate a new empty hashtable that will use the given
 *        hash function. The hashtable should eventually be passed to 
 *        DYNAMIC_FREE_HASHTABLE.
 *
 * @param hashtable_type The type of the hashtable to instantiate.
 *
 * @param hashtable_name The name of the hashtable to instantiate.
 *
 * @param hash_function The function to hash keys to integers for this
 *        hashtable.
 */
#define DYNAMIC_INIT_HASHTABLE(hashtable_type, hashtable_name, hash_function) \
	do { \
		hashtable_name = (hashtable_type)malloc(sizeof(struct hashtable_type##struct)); \
		assert(hashtable_name); \
		STATIC_INIT_HASHTABLE(hashtable_type, *hashtable_name, hash_function); \
	} while (0)

/** @def STATIC_FREE_HASHTABLE(hashtable_name)
 *
 * @brief Free a hashtable that was allocated statically.
 *
 * @param hashtable_name The hashtable to free.
 */
#define STATIC_FREE_HASHTABLE(hashtable_name) \
	free((hashtable_name).table)

/** @def DYNAMIC_FREE_HASHTABLE(hashtable_name)
 *
 * @brief Free a hashtable that was allocated dynamically.
 *
 * @param hashtable_name The hashtable to free.
 */
#define DYNAMIC_FREE_HASHTABLE(hashtable_name) \
	do { \
		STATIC_FREE_HASHTABLE(*hashtable_name); \
		free(hashtable_name); \
	} while (0)

/* Check that some macros we want aren't already defined elsewhere. */
#ifdef _INDEX_
#error _INDEX_ redefined in hashtable.h
#endif
#ifdef _TABLE_
#error _TABLE_ redefined in hashtable.h
#endif
#ifdef _LINK_
#error _LINK_ redefined in hashtable.h
#endif
#ifdef _HASH_
#error _HASH_ redefined in hashtable.h
#endif

/* Generate (hopefully) unique identifers. */
#define _INDEX_ _hashtable_index_
#define _TABLE_ _hashtable_table_
#define _LINK_ _hashtable_link_
#define _HASH_ _hashtable_hash_

/** @def HASHTABLE_PUT(hashtable_type, hashtable_name, key_name, val_name)
 *
 * @brief Add a new key, val pair to the hashtable, or update an existing pair
 *        if the key is already in the table.
 *
 * @param hashtable_type The type of the hashtable.
 * @param hashtable_name The hashtable.
 * @param key_name The key to access the table with.
 * @param val_name The value to place in the table.
 */
#define HASHTABLE_PUT(hashtable_type, hashtable_name, key_name, val_name) \
	do { \
		size_t _HASH_; \
		struct hashtable_type##_link_struct *_LINK_ = NULL; \
		/* If the hash table has reached load factor 1, double the size of the
		 * table. */ \
		if ((hashtable_name).size == prime_hashtable_sizes[(hashtable_name).table_index]) { \
			/* Allocate a new table. */ \
			struct hashtable_type##_link_struct **_TABLE_ = \
				(struct hashtable_type##_link_struct **)calloc( \
						prime_hashtable_sizes[(hashtable_name).table_index + 1], \
						sizeof(struct hashtable_type##_link_struct *)); \
			size_t _INDEX_; \
			for (_INDEX_ = 0; \
						_INDEX_ < prime_hashtable_sizes[(hashtable_name).table_index]; \
						_INDEX_++) { \
				/* Copy every list in the table. */ \
				while ((hashtable_name).table[_INDEX_] != NULL) { \
					_LINK_ = (hashtable_name).table[_INDEX_]->next; \
					_HASH_ = (hashtable_name).hash( \
							(hashtable_name).table[_INDEX_]->key) % \
							prime_hashtable_sizes[(hashtable_name).table_index + 1]; \
					(hashtable_name).table[_INDEX_]->next = _TABLE_[_HASH_]; \
					_TABLE_[_HASH_] = (hashtable_name).table[_INDEX_]; \
					(hashtable_name).table[_INDEX_] = _LINK_; \
				} \
			} \
			/* Free the old table. */ \
			free((hashtable_name).table); \
			(hashtable_name).table = _TABLE_; \
			(hashtable_name).table_index++; \
		} \
		_HASH_ = (hashtable_name).hash(key_name) % \
				prime_hashtable_sizes[(hashtable_name).table_index]; \
		/* Search for the key and update its value if already in the table. */ \
		for (_LINK_ = (hashtable_name).table[_HASH_]; \
					_LINK_ != NULL; \
					_LINK_ = _LINK_->next) { \
			if (_LINK_->key == (key_name)) { \
				_LINK_->val = (val_name); \
				break; \
			} \
		} \
		/* Add the key, value to the table if the key is not in the table. */ \
		if (_LINK_ == NULL) { \
			_LINK_ = (struct hashtable_type##_link_struct *)malloc( \
					sizeof(struct hashtable_type##_link_struct)); \
			_LINK_->key = key_name; \
			_LINK_->val = val_name; \
			_LINK_->next = (hashtable_name).table[_HASH_]; \
			(hashtable_name).table[_HASH_] = _LINK_; \
			(hashtable_name).size++; \
		} \
	} while (0)

/** @def HASHTABLE_GET(hashtable_type, hashtable_name, key_name, val_name)
 *
 * @brief Retrieve a value from the hashtable given its key. If the given key
 *        is not in the table, do not change the given value.
 *
 * @param hashtable_type The type of the hashtable.
 * @param hashtable_name The hashtable.
 * @param key_name The key to access the table with.
 * @param val_name The variable to place the value in.
 */
#define HASHTABLE_GET(hashtable_type, hashtable_name, key_name, val_name) \
	do { \
		size_t _HASH_ = (hashtable_name).hash(key_name) % \
				prime_hashtable_sizes[(hashtable_name).table_index]; \
		struct hashtable_type##_link_struct *_LINK_; \
		/* Search for key in the bucket that was hashed to. */ \
		for (_LINK_ = (hashtable_name).table[_HASH_]; \
					_LINK_ != NULL; \
					_LINK_ = _LINK_->next) { \
			if (_LINK_->key == (key_name)) { \
				val_name = _LINK_->val; \
				break; \
			} \
		} \
	} while (0)

/** @def HASHTABLE_REMOVE(hashtable_type, hashtable_name, key_name, val_name)
 *
 * @brief Remove a key, value pair from the hashtable, placing the value in
 *        val_name. Do nothing if the specified key is not in the table.
 *
 * @param hashtable_type The type of the hashtable.
 * @param hashtable_name The hashtable.
 * @param key_name The key to remove from the hashtable.
 * @param val_name The variable to place the value in.
 */
#define HASHTABLE_REMOVE(hashtable_type, hashtable_name, key_name, val_name) \
	do { \
		size_t _HASH_ = (hashtable_name).hash(key_name) % \
				prime_hashtable_sizes[(hashtable_name).table_index]; \
		struct hashtable_type##_link_struct *_LINK_ = (hashtable_name).table[_HASH_]; \
		/* If key is the first element in the bucket, remove it and update the
		 * bucket. */ \
		if (_LINK_ != NULL && _LINK_->key == (key_name)) { \
			(val_name) = _LINK_->val; \
			(hashtable_name).table[_HASH_] = (hashtable_name).table[_HASH_]->next; \
			free(_LINK_); \
		} \
		else { \
			/* Otherwise search the bucket for the key and remove it. */ \
			for ( ; _LINK_ != NULL; _LINK_ = _LINK_->next) { \
				if (_LINK_->next != NULL && _LINK_->next->key == (key_name)) { \
					(val_name) = _LINK_->next->val; \
					free(_LINK_->next); \
					_LINK_->next = _LINK_->next->next; \
					break; \
				} \
			} \
		} \
	} while (0)

#endif

