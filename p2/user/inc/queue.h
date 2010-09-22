/** @file queue.h
 *
 * @brief Macros for generating and manipulating a double-ended queue
 *
 * @author Tim Wilson (tjwilson)
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <assert.h>
#include <stddef.h>

/** @def DEFINE_QUEUE(queue_type, node_type)
 *
 * @brief Generate the definition for a new type of queue holding nodes
 *
 * @param queue_type The resulting pointer type to the queue.
 *        queue_type must be an undefined type.
 *
 * @param node_type The pointer type of the nodes in the queue.
 *        node_type must be an existing pointer to struct type. The struct 
 *        type must define next and prev pointers.
 */
#define DEFINE_QUEUE(queue_type, node_type) \
	struct queue_type##_queue_struct { \
		node_type first; \
		node_type last; \
	}; \
	typedef struct queue_type##_queue_struct queue_type

/** @def STATIC_INIT_QUEUE(queue)
 *
 * @brief Statically instantiate a new empty queue
 *
 * @param queue The queue to instantiate.
 */
#define STATIC_INIT_QUEUE(queue) \
	do { \
		(queue)->first = NULL; \
		(queue)->last = NULL; \
	} \
	while (0)

/** @def DYNAMIC_INIT_QUEUE(queue_type, queue_name)
 *
 * @brief Dynamically instantiate a new empty queue. The
 *        queue should eventually be passed to free.
 *
 * @param queue_type The type of the queue to instantiate.
 * @param queue_name The queue to instantiate.
 */
#define DYNAMIC_INIT_QUEUE(queue_type, queue_name) \
	do { \
		queue_name = (queue_type)calloc(sizeof(struct queue_type##_queue_struct)); \
		assert(queue_name) \
	} \
	while (0)

/** @def ENQUEUE_FIRST(queue, node)
 *
 * @brief Enqueue node as the first node in the queue.
 *
 * @param queue The queue to place the node on.
 * @param node The node to place on the queue.
 */
#define ENQUEUE_FIRST(queue, node) \
	do { \
		assert(queue); \
		assert(node); \
		if ((queue)->first) \
			(queue)->first->prev = node; \
		else \
			(queue)->last = node; \
		(node)->next = (queue)->first; \
		(node)->prev = NULL; \
		(queue)->first = node; \
	} while (0)

/** @def ENQUEUE_LAST(queue, node)
 *
 * @brief Enqueue node as the last node in the queue.
 *
 * @param queue The queue to place the node on.
 * @param node The node to place on the queue.
 */
#define ENQUEUE_LAST(queue, node) \
	do { \
		assert(queue); \
		assert(node); \
		if ((queue)->last) \
			(queue)->last->next = node; \
		else \
			(queue)->first = node; \
		(node)->prev = (queue)->last; \
		(node)->next = NULL; \
		(queue)->last = node; \
	} while (0)

/** @def ENQUEUE_BEFORE(queue, queue_node, new_node)
 *
 * @brief Enqueue new_node before queue_node in the queue.
 *
 * @param queue The queue to place new_node on.
 * @param queue_node The node that will be after new_node in queue.
 * @param new_node The node to place before queue_node in the queue.
 */
#define ENQUEUE_BEFORE(queue, queue_node, new_node) \
	do { \
		assert(queue); \
		assert(new_node); \
		if ((queue_node) == (queue)->first) { \
			ENQUEUE_FIRST(queue, new_node); \
		} \
		else if ((queue_node) == NULL) { \
			ENQUEUE_LAST(queue, new_node); \
		} \
		else { \
			(new_node)->prev = (queue_node)->prev; \
			(new_node)->next = queue_node; \
			(queue_node)->prev->next = new_node; \
			(queue_node)->prev = new_node; \
		} \
	} while (0)

/** @def ENQUEUE_AFTER(queue, queue_node, new_node)
 *
 * @brief Enqueue new_node after queue_node in the queue.
 *
 * @param queue The queue to place new_node on.
 * @param queue_node The node that will be before new_node in queue.
 * @param new_node The node to place after queue_node in the queue.
 */
#define ENQUEUE_AFTER(queue, queue_node, new_node) \
	do { \
		assert(queue); \
		assert(new_node); \
		if ((queue_node) == (queue)->last) { \
			ENQUEUE_LAST(queue, new_node); \
		} \
		else if ((queue_node) == NULL) { \
			ENQUEUE_FIRST(queue, new_node); \
		} \
		else { \
			(new_node)->next = (queue_node)->next; \
			(new_node)->prev = queue_node; \
			(queue_node)->next->prev = new_node; \
			(queue_node)->next = new_node; \
		} \
	} while (0)

/** @def DEQUEUE_FIRST(queue, node)
 *
 * @brief Remove the first node from queue and assign it to node.
 *
 * @param queue The queue to remove from.
 * @param node The node to assign the removed head to. If node is NULL then the
 *        removed head is discarded.
 */
#define DEQUEUE_FIRST(queue, node) \
	do { \
		assert(queue); \
		(node) = (queue)->first; \
		if ((queue)->first) { \
			if ((queue)->first == (queue)->last) { \
				(queue)->first = NULL; \
				(queue)->last = NULL; \
			} \
			else { \
				(queue)->first = (queue)->first->next; \
				(queue)->first->prev = NULL; \
			} \
		} \
	} while (0)

/** @def DEQUEUE_LAST(queue, node)
 *
 * @brief Remove the last node from queue and assign it to node.
 *
 * @param queue The queue to remove from.
 * @param node The node to assign the removed tail to. If node is NULL then the
 *        removed tail is discarded.
 */
#define DEQUEUE_LAST(queue, node) \
	do { \
		assert(queue); \
		if (node) { \
			(node) = (queue)->last; \
		} \
		if ((queue)->last) { \
			if ((queue)->last == (queue)->first) { \
				(queue)->first = NULL; \
				(queue)->last = NULL; \
			} \
			else { \
				(queue)->last = (queue)->last->prev; \
				(queue)->last->next = NULL; \
			} \
		} \
	} while (0)

/** @def DEQUEUE_ELEM(queue, node)
 *
 * @brief Remove the given node from the queue.
 *
 * @param queue The queue to remove from.
 * @param node The node to remove.
 */
#define DEQUEUE_ELEM(queue, node) \
	do { \
		assert(queue); \
		assert(node); \
		if ((node) == (queue)->first) { \
			DEQUEUE_FIRST(queue, node); \
		} \
		else if ((node) == (queue)->last) { \
			DEQUEUE_LAST(queue, node); \
		} \
		else { \
			(node)->next->prev = (node)->prev; \
			(node)->prev->next = (node)->next; \
		} \
	} while (0)

/** @def PEEK_FIRST(queue, node)
 *
 * @brief Get the first node of the queue.
 *
 * @param queue The queue.
 *
 * @param node The variable to place the first node in.
 */
#define PEEK_FIRST(queue, node) \
	do { \
		assert(queue);  \
		node = (queue)->first; \
	} while (0)

/** @def PEEK_LAST(queue, node)
 *
 * @brief Get the last node of the queue.
 *
 * @param queue The queue.
 *
 * @param node The variable to place the last node in.
 */
#define PEEK_LAST(queue) \
	do { \
		assert(queue);  \
		node = (queue)->flast; \
	} while (0)

/** @def FOREACH(queue, node) \
 *
 * @brief Iterate over each node in the queue
 *
 * @param queue The queue to iterate over.
 *
 * @param node The variable to use for iteration. node must already be defined.
 *        node will be NULL after the iteration if iteration completes.
 */
#define FOREACH(queue, node) \
	assert(queue); \
	assert(node); \
	for ((node) = (queue)->first; (node) != NULL; (node) = (node)->next)

#endif

