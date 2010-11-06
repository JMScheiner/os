/** @file list.h
 *
 * @brief Macros for generating and manipulating a circular list.
 *    The front of the list is defined as a pointer into a list 
 *    structure. 
 *    When the argument is "list" we expect a pointer into the list. 
 *    When the argument is "node" we expect a member of a list. 
 *    Pointers into lists are allowed to be NULL.  Members of lists 
 *    are not.
 *    
 *    When the argument is "instance" we expect the name of the list 
 *    within a node.
 *       - Used to reference multiple lists in a single node.
 * 
 *    Attempts to remove a node from a list it does not belong to and
 *    attempts to add a node to a list it does belong to will do nothing.
 *
 * @author Tim Wilson (tjwilson)
 * @author Justin Scheiner (tjwilson)
 */

#ifndef LIST_H
#define LIST_H

#include <assert.h>
#include <stddef.h>

/** @brief Define a new type of list. 
 *
 * @param node_type The new name of the node type in the list 
 * @param data_type The existing name of the data type in the list
 */
#define DEFINE_LIST(node_type, data_type) \
   struct node_type##_list_struct { \
      data_type* next; \
      data_type* prev; \
   }; \
   typedef struct node_type##_list_struct node_type

/** @brief Initialize a new empty list
 *
 * @param list The list to initialize
 */
#define LIST_INIT_EMPTY(list) list = NULL

#define LIST_INIT_NONEMPTY(list, instance) \
	do { \
      LIST_NEXT(list, instance) = list; \
      LIST_PREV(list, instance) = list; \
	} \
	while (0)

#define LIST_INIT_NODE(node, instance) \
	do { \
      LIST_NEXT(node, instance) = NULL; \
      LIST_PREV(node, instance) = NULL; \
	} \
	while (0)

/** @brief Get the next element of the list
 *
 * @param node The current element in the list
 * @param instance The list type to get the next element from
 */
#define LIST_NEXT(node, instance) \
   ((node)->instance.next)

/** @brief Get the previous element of the list
 *
 * @param node The current element in the list
 * @param instance The list type to get the previous element from
 */
#define LIST_PREV(node, instance) \
   ((node)->instance.prev)

/** @brief Iterate over all of the elements of the list
 *
 * @param list A pointer into the list to iterate over
 * @param iter The iteration variable
 * @param instance The list type to iterate over
 */
#define LIST_FORALL(list, iter, instance) \
	for ((iter) = (list); \
			((iter) != NULL); \
			(iter) = LIST_NEXT(iter, instance) == (list) ? \
			NULL : LIST_NEXT(iter, instance))

/** @brief Insert an element into a list before the given node
 *
 * @param list A pointer to a node in the list
 * @param node The node to insert into the list
 * @param instance The type of the list to insert into
 */
#define LIST_INSERT_BEFORE(list, node, instance) \
	do { \
      if(!(list)) \
      { \
         (list) = (node); \
         LIST_NEXT(node, instance) = node; \
         LIST_PREV(node, instance) = node; \
      } \
      else if (LIST_NEXT(node, instance) == NULL) {\
         LIST_NEXT(LIST_PREV(list, instance), instance) = (node); \
         LIST_PREV(node, instance) = LIST_PREV(list, instance); \
         LIST_PREV(list, instance) = (node); \
         LIST_NEXT(node, instance) = (list); \
      }\
	} while (0)

/** @brief Insert an element into a list after the given node
 *
 * @param list A pointer to a node in the list
 * @param node The node to insert into the list
 * @param instance The type of the list to insert into
 */
#define LIST_INSERT_AFTER(list, node, instance) \
	do { \
      if(!(list)) \
      { \
         (list) = (node); \
         LIST_NEXT(list, instance) = list; \
         LIST_PREV(list, instance) = list; \
      } \
      else if (LIST_NEXT(node, instance) == NULL) \
      { \
         LIST_PREV(LIST_NEXT(list, instance), instance) = (node); \
         LIST_NEXT(node, instance) = LIST_NEXT(list, instance); \
         LIST_NEXT(list, instance) = (node); \
         LIST_PREV(node, instance) = (list); \
      } \
	} while (0)

/** @brief Remove an element from a list
 *
 * @param list The list to remove from
 * @param node The node to remove
 * @param instance The type of the list to remove from
 */
#define LIST_REMOVE(list, node, instance) \
	do { \
      if(LIST_NEXT(node, instance) == (node) ) \
      { \
			assert(list == node); \
         list = NULL; \
      } \
      else if (LIST_NEXT(node, instance) != NULL) \
      { \
         if( (list) == (node) ) \
         { \
            (list) = LIST_NEXT(node, instance); \
         } \
         LIST_NEXT(LIST_PREV(node, instance), instance) = \
				LIST_NEXT(node, instance); \
         LIST_PREV(LIST_NEXT(node, instance), instance) = \
				LIST_PREV(node, instance); \
      } \
		LIST_NEXT(node, instance) = NULL; \
		LIST_PREV(node, instance) = NULL; \
   } while (0)

#endif

