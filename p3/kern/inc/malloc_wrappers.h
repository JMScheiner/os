#ifndef MALLOC_WRAPPER_H_FD234GHJ
#define MALLOC_WRAPPER_H_FD234GHJ

void alloc_init();
void *scalloc(size_t nmemb, size_t size);
void *srealloc(void* buf, size_t current_size, size_t new_size);

#endif

