
#include <debug.h>
#include <simics.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEBUG_BUF_SIZE 256

#define DEBUG_PRINT
#define DEBUG_BREAK

char *debug_print_strings[] = {
   //"children", 
   //"yield", 
   //"make_runnable", 
   //"deschedule", 
   //"scheduler", 
   //"mutex", 
   //"sleep", 
   //"loader", 
   //"wait", 
   //"vanish", 
   //"fork", 
   //"exec", 
   //"thread_fork",
   //"mm", 
   //"kvm", 
   "page", 
   //"readline", 
   //"lifecycle", 
   //"region", 
   //"malloc", 
   //"console", 
   //"memman", 
   NULL};
char *debug_break_strings[] = {NULL};

#ifdef DEBUG_PRINT
void debug_print(const char *type, const char *fmt, ...) {
   int i;
   for (i = 0; debug_print_strings[i] != NULL; i++) {
      if (strcmp(debug_print_strings[i], type) == 0) {
         char str[DEBUG_BUF_SIZE];
         snprintf(str, DEBUG_BUF_SIZE - 1, "%s: ", type);
         int end = strlen(str);
         va_list ap;
         va_start(ap, fmt);
         vsnprintf(str + end, DEBUG_BUF_SIZE - 1 - end, fmt, ap);
         va_end(ap);

         sim_puts(str);
         return;
      }
   }
}
#else
void debug_print(const char *type, const char *fmt, ...) {
   return;
}
#endif

#ifdef DEBUG_BREAK
void debug_break(const char *type) {
   int i;
   for (i = 0; debug_break_strings[i] != NULL; i++) {
      if (strcmp(debug_break_strings[i], type) == 0) {
         MAGIC_BREAK;
         return;
      }
   }
}
#else
void debug_break(const char *type) {
   return;
}
#endif

