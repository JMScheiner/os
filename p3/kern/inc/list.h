/** @file list.h
 *
 * @brief Macros for generating and manipulating a circular list.
 *    The front of the list is defined as a pointer into a list structure. 
 *    When the argument is "list" we expect a pointer into the list. 
 *    When the argument is "node" we expect a member of a list. 
 *    Pointers into lists are allowed to be NULL.  Members of lists are not.
 *    
 *    When the argument is "instance" we expect the name of the list within a node.
 *       - Used to reference multiple lists in a single node.
 * 
 *    Attempts to insert into a list multiple times will end in catastrophe. (probably)
 *    You must know what list a node belongs to. 
 *
 * @author Tim Wilson (tjwilson)
 * @author Justin Scheiner (tjwilson)
 */

#ifndef LIST_H
#define LIST_H

#include <assert.h>
#include <stddef.h>

#define DEFINE_LIST(node_type, data_type) \
   struct node_type##_list_struct { \
      data_type* next; \
      data_type* prev; \
   }; \
   typedef struct node_type##_list_struct node_type

#define INIT_LIST(list) \
   do {                 \
      list = NULL;      \
   } while(0)

/* gcc grumbles at instance being wrapped in () here. */
#define LIST_NEXT(node, instance) \
   ((node)->instance.next)

#define LIST_PREV(node, instance) \
   ((node)->instance.prev)


#define LIST_INIT_NODE(node, instance) \
	do { \
      LIST_NEXT(node, instance) = (node); \
      LIST_PREV(node, instance) = (node); \
	} \
	while (0)

#define LIST_INSERT_BEFORE(list, node, instance) \
	do { \
      if(!(list)) \
      { \
         (list) = (node); \
         LIST_NEXT(node, instance) = node; \
         LIST_PREV(node, instance) = node; \
      } \
      else {\
         LIST_NEXT(LIST_PREV(list, instance), instance) = (node); \
         LIST_PREV(node, instance) = LIST_PREV(list, instance); \
         LIST_PREV(list, instance) = (node); \
         LIST_NEXT(node, instance) = (list); \
      }\
	} while (0)

#define LIST_INSERT_AFTER(list, node, instance) \
	do { \
      if(!(list)) \
      { \
         (list) = (node); \
         LIST_NEXT(list, instance) = list; \
         LIST_PREV(list, instance) = list; \
      } \
      else \
      { \
         LIST_PREV(LIST_NEXT(list, instance), instance) = (node); \
         LIST_NEXT(node, instance) = LIST_NEXT(list, instance); \
         LIST_NEXT(list, instance) = (node); \
         LIST_PREV(node, instance) = (list); \
      } \
	} while (0)

#define LIST_REMOVE(list, node, instance) \
	do { \
      if( (list) == (node) && (LIST_NEXT(node, instance) == (node)) ) \
      { \
         list = NULL; \
      } \
      else \
      { \
         LIST_NEXT(LIST_PREV(node, instance), instance) = LIST_NEXT(node, instance); \
         LIST_PREV(LIST_NEXT(node, instance), instance) = LIST_PREV(node, instance); \
      } \
   } while (0)

#endif

