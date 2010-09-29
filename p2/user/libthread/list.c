/** @file list.c
 *
 * @brief A list that internally uses read write spin locks to provide
 * concurrent access.
 *
 * @author Tim Wilson (tjwilson)
 */

#include <atomic.h>
#include <malloc.h>
#include <thr_internals.h>
#include <list.h>
#include <limits.h>

/** @brief A link in the linked list */
typedef struct list_link {
	/** @brief The key identifying the link. */
	unsigned int key;

	/** @brief The value associated with the key in this link. */
	tcb_t *val;

	/** @brief The next link in the list. */
	struct list_link *next;

	/** @brief A rwlock for this link. If lock < 0, then the lock is held by a
	 * writer. If lock == 0, then the lock is not held. If lock > 0, then lock is
	 * the number of readers holding the lock. */
	int lock;

	/** @brief True iff (for the most part) a writer is using the lock, or
	 * waiting to use the lock. */
	boolean_t writer_present;
} link_t;

/** @brief A linked list. */
struct linked_list {
	link_t *head;
};

/** @brief Apply a writer lock to link.
 *
 * While a writer lock is held on link, no other thread may hold a writer lock
 * or reader lock on link.
 *
 * Try not to starve if there are excessive numbers of readers by registering
 * an intent to apply the writer lock. This isn't guaranteed, since
 * WRITE_UNLOCK needs to unset the intent, but it should be extremely unlikely.
 *
 * @param link The link to lock
 */
#define WRITE_LOCK(link) \
	do { \
		do { \
			link->writer_present = TRUE; \
		} while (atomic_cmpset(&link->lock, INT_MIN, 0) != 0); \
		link->writer_present = TRUE; \
	} while (0)

/** @brief Apply a reader lock to link.
 *
 * Any number of reader locks may be held concurrently, but no writer locks may
 * be held at the same time. If a writer as registered an intent to take the
 * lock, wait for the writer to go.
 *
 * @param link The link to lock
 */
#define READ_LOCK(link) \
	do { \
		while (link->writer_present) \
			; \
		if (atomic_add(&link->lock, 1) <= 0) { \
			READ_UNLOCK(link); \
		} \
		else { \
			break; \
		} \
	} while (1)

/** @brief Unlock a writer lock
 *
 * Indicate that there is no longer a writer waiting on the lock. If there is,
 * the spin lock in WRITE_LOCK will reregister the intent.
 *
 * @param link The link to unlock
 */
#define WRITE_UNLOCK(link) \
	do { \
		link->writer_present = FALSE; \
		link->lock = 0; \
	} while (0)

/** @brief Unlock a reader lock. 
 *
 * @param link The link to unlock
 */
#define READ_UNLOCK(link) \
	atomic_add(&link->lock, -1)

/** @brief Lock and return the head of the list. */
static link_t *get_head(list_t list, boolean_t reader) {
	if (reader) {
		READ_LOCK(list->head);
	}
	else {
		WRITE_LOCK(list->head);
	}
	return list->head;
}

/** @brief Return the link following the given one in the list.
 *
 * The link that is returned will be locked.
 *
 * @param link The link to get the successor of. This link MUST be locked
 *             before calling get_next.
 *
 * @param return The successor of link.
 */
static link_t *get_next(link_t *link, boolean_t reader) {
	if (link->next != NULL) {
		if (reader) {
			READ_LOCK(link->next);
		}
		else {
			WRITE_LOCK(link->next);
		}
	}
	link_t *next = link->next;
	return next;
}

/** @brief Allocate and initialize a new empty list
 *
 * @return The new empty list
 */
list_t list_new() {
	list_t list = (list_t)calloc(1, sizeof(struct linked_list));
	assert(list);
	link_t *head = (link_t *)calloc(1, sizeof(link_t));
	assert(head);
	list->head = head;
	return list;
}

/** @brief Traverse the list looking for the given key
 *
 * @param list The list to search.
 * @param key The key to look for.
 * @param reader True iff the results of this search will only be read, not
 *               written.
 * @param previous Address to place the link before the link containg the key.
 * @param current Address to place the link containing the key.
 */
static void list_search(list_t list, unsigned int key, boolean_t reader, 
                        link_t **previous, link_t **current) {
	link_t *curr = get_head(list, reader);
	link_t *prev = curr;
	while (curr != NULL) {
		curr = get_next(prev, TRUE);
		if (curr != NULL && curr->key == key) {
			break;
		}
		if (reader) {
			READ_UNLOCK(prev);
		}
		else {
			WRITE_UNLOCK(prev);
		}
		prev = curr;
	}
	*previous = prev;
	*current = curr;
}

/** @brief Lookup a key in the linked list.
 *
 * @param list The list to search.
 * @param key The key to look for.
 *
 * @return The value associated with the key if the key is in the table,
 *         otherwise NULL.
 */
tcb_t *list_lookup(list_t list, unsigned int key) {
	tcb_t *val = NULL;
	link_t *curr;
	link_t *prev;
	list_search(list, key, TRUE, &prev, &curr);
	READ_UNLOCK(prev);

	if (curr != NULL) {
		val = curr->val;
	}
	return val;
}

/** @brief Delete a key from the linked list.
 *
 * @param list The list to search.
 * @param key The key to look for.
 *
 * @return The value associated with the key if the key is in the table,
 *         otherwise NULL.
 */
tcb_t *list_delete(list_t list, unsigned int key) {
	tcb_t *val = NULL;
	link_t *curr;
	link_t *prev;
	list_search(list, key, FALSE, &prev, &curr);

	if (curr != NULL) {
		prev->next = curr->next;
		WRITE_UNLOCK(prev);
		val = curr->val;
		free(curr);
	}
	else {
		WRITE_UNLOCK(prev);
	}
	return val;
}

/** @brief Insert a new link into the linked list.
 *
 * @param list The list to insert into.
 * @param link The link to insert.
 */
void list_insert(list_t list, unsigned int key, tcb_t *val) {
	link_t *link = (link_t *)malloc(sizeof(link_t));
	assert(link);
	link->key = key;
	link->val = val;
	link_t *head = get_head(list, FALSE);
	link_t *first_link = get_next(head, FALSE);
	head->next = link;
	link->next = first_link;
	WRITE_UNLOCK(head);
	WRITE_UNLOCK(first_link);
}

