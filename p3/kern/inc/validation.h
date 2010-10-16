
#ifndef VALIDATION_H_FJWK2143
#define VALIDATION_H_FJWK2143

/**
 * @brief Perform a checked loop over a region of memory
 *
 * @param addr The variable that will loop over the memory region
 * @param inc The number of bytes to increment the address by on each iteration
 *            of the loop.
 * @param cntr A variable counting the number of iterations performed by the
 *             loop. Need not be initialized.
 * @param max The maximum number of iterations to perform.
 */
#define SAFE_LOOP(addr, inc, cntr, max) \
	for ((cntr) = 0 ; \
			      (cntr) < (max) \
			   && ((cntr) != 0) \
			   && ((SAME_PAGE(addr, addr - inc)) \
			   || mm_validate(addr)) \
			; (cntr)++, \
				((addr)) += (inc))

int v_strcpy(char *dest, char *src, int max_len);
int v_memcpy(char *dest, char *src, int max_len);

#endif
