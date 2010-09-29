/** @file list.c
 *
 * @brief A list that internally uses spin locks to provide concurrent access.
 *
 * @author Tim Wilson (tjwilson)
 */

#include <atomic.h>
#include <malloc.h>
#include <thr_internals.h>
#include <list.h>

typedef struct list_link {
	unsigned int key;
	tcb_t *val;
	struct list_link *next;
	boolean_t locked;
} link_t;

struct linked_list {
	link_t *head;
};

/** @brief Spin on a lock for a link in the linked list. Atomically lock 
 *         it as soon as it unlocks */
#define LOCK(link) \
	while (atomic_cmpset((int *)&link->locked, TRUE, FALSE) != 0)

/** @brief Unlock the spin lock for this link in the linked list. */
#define UNLOCK(link) \
	link->locked = FALSE

/** @brief Lock and return the head of the list. */
static link_t *get_head(list_t list) {
	LOCK(list->head);
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
static link_t *get_next(link_t *link) {
	if (link->next != NULL) {
		LOCK(link->next);
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
	return list;
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
	link_t *curr = get_head(list);
	link_t *prev = curr;
	while (curr != NULL && curr->key != key) {
		curr = get_next(prev);
		UNLOCK(prev);
		prev = curr;
	}
	if (curr != NULL) {
		val = curr->val;
		UNLOCK(curr);
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
	link_t *curr = get_head(list);
	link_t *prev = curr;
	while (curr != NULL) {
		curr = get_next(prev);
		if (curr->key == key) {
			prev->next = curr->next;
			UNLOCK(prev);
			break;
		}
		UNLOCK(prev);
		prev = curr;
	}
	if (curr != NULL) {
		val = curr->val;
		free(curr);
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
	link_t *head = get_head(list);
	link_t *first_link = get_next(head);
	head->next = link;
	link->next = first_link;
	UNLOCK(head);
	UNLOCK(first_link);
}

