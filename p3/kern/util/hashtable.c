/** @file hashtable.c
 *
 * @brief A hashtable mapping tids to tcbs
 *
 * @author Tim Wilson (tjwilson)
 */

#include <assert.h>
#include <stddef.h>
#include <simics.h>
#include <hashtable.h>
#include <kernel_types.h>
#include <mutex.h>
#include <malloc_wrappers.h>
#include <malloc.h>

/** @brief A roughly doubling sequence of prime numbers suitable
 * for use as table sizes in a hashtable. */
static unsigned int prime_hashtable_sizes[] = 
  {(1 << 7)-1,
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
unsigned int default_hash(int tid)
{
   return (unsigned int)tid;
}

/**
 * @brief Initialize the hashtable
 *
 * @param hashtable The hashtable to initialize
 * @param hash The hash function to use for this table.
 */
void hashtable_init(hashtable_t *hashtable, unsigned int (*hash)(int))
{
   hashtable->size = 0;
   hashtable->table_index = 0;
   hashtable->hash = hash;
   mutex_init(&hashtable->lock);
   hashtable->table = (hashtable_link_t **)
      scalloc(prime_hashtable_sizes[hashtable->table_index], 
                      sizeof(hashtable_link_t *));
}

/**
 * @brief Resize the hashtable by roughly doubling its capacity.
 *
 * @param hashtable The hashtable to resize.
 */
static void hashtable_resize(hashtable_t *hashtable)
{
   hashtable_link_t **table = (hashtable_link_t **)
      scalloc(prime_hashtable_sizes[hashtable->table_index + 1], 
            sizeof(hashtable_link_t *));
   size_t i;
   unsigned int hash;
   hashtable_link_t *link;
   for (i = 0; i < prime_hashtable_sizes[hashtable->table_index]; i++) {
      /* Copy every list in the table. */
      while (hashtable->table[i] != NULL) {
         link = hashtable->table[i]->next;
         hash = hashtable->hash(hashtable->table[i]->tid) % 
            prime_hashtable_sizes[hashtable->table_index + 1];
         hashtable->table[i]->next = table[hash];
         table[hash] = hashtable->table[i];
         hashtable->table[i] = link;
      }
   }
   /* Free the old table. */
   sfree(hashtable->table, prime_hashtable_sizes[hashtable->table_index] * 
                   sizeof(hashtable_link_t *));
   hashtable->table = table;
   hashtable->table_index++;
}

/**
 * @brief Put a tid, tcb pair in the hash table
 *
 * @param hashtable The hashtable to insert in
 * @param tid The tid to insert
 * @param tcb The tcb to insert
 */
void hashtable_put(hashtable_t *hashtable, int tid, tcb_t *tcb)
{
   size_t hash;
   hashtable_link_t *link = NULL;
   /* If the hash table has reached load factor 1, double the size of the
    * table. */
   if (hashtable->size == prime_hashtable_sizes[hashtable->table_index])
      hashtable_resize(hashtable);
   hash = hashtable->hash(tid) % 
      prime_hashtable_sizes[hashtable->table_index];
   link = (hashtable_link_t *)smalloc(sizeof(hashtable_link_t));
   link->tid = tid;
   link->tcb = tcb;
   link->next = hashtable->table[hash];
   hashtable->table[hash] = link;
   hashtable->size++;
}

/**
 * @brief Get a tcb from the hashtable
 *
 * @param hashtable The hashtable to get from.
 * @param tid The tid of the tcb to get.
 *
 * @return The tcb with the given tid, or NULL if no tcb with the given
 * tid exists in the table.
 */
tcb_t *hashtable_get(hashtable_t *hashtable, int tid)
{
   size_t hash = hashtable->hash(tid) % 
      prime_hashtable_sizes[hashtable->table_index];
   hashtable_link_t *link;
   tcb_t *tcb = NULL;
   /* Search for key in the bucket that was hashed to. */
   for (link = hashtable->table[hash]; link != NULL; link = link->next) {
      if (link->tid == tid) {
         tcb = link->tcb;
         break;
      }
   }
   return tcb;
}

/**
 * @brief Remove the tcb with the given tid from the hashtable
 *
 * @param hashtable The hashtable to remove from.
 * @param tid The tid of the tcb to remove
 *
 * @return The tcb with the given tid, or NULL if no tcb with the given
 * tid exists in the table.
 */
tcb_t *hashtable_remove(hashtable_t *hashtable, int tid)
{
   size_t hash = hashtable->hash(tid) % 
      prime_hashtable_sizes[hashtable->table_index];
   hashtable_link_t *link = hashtable->table[hash];
   hashtable_link_t *free_link;
   tcb_t *tcb = NULL;
   /* If key is the first element in the bucket, remove it and update the
    * bucket. */
   if (link != NULL && link->tid == tid) {
      tcb = link->tcb;
      hashtable->table[hash] = hashtable->table[hash]->next;
      sfree(link, sizeof(hashtable_link_t));
      hashtable->size--;
   }
   else if (link != NULL) {
      /* Otherwise search the bucket for the key and remove it. */
      for ( ; link->next != NULL; link = link->next) {
         if (link->next->tid == tid) {
            free_link = link->next;
            tcb = free_link->tcb;
            link->next = free_link->next;
            sfree(free_link, sizeof(hashtable_link_t));
            hashtable->size--;
            break;
         }
      }
   }
   return tcb;
}

