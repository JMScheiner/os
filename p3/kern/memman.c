
#include <memman.h>
#include <reg.h>
#include <mm.h>
#include <simics.h>

void new_pages_handler(volatile regstate_t reg)
{
	lprintf("Ignoring new_pages");
	MAGIC_BREAK;
   //TODO
}

void remove_pages_handler(volatile regstate_t reg)
{
	lprintf("Ignoring remove_pages");
	MAGIC_BREAK;
   //TODO
}

