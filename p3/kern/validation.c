
/**
 * @brief Perform a validated strcpy.
 *
 * Copy a string from user memory to kernel memory, verifying that each byte is
 * in a mapped address. If the copy is terminated early, either by trying to
 * access an unmapped byte, or reaching the maximum number of bytes to copy,
 * the string placed in dest will not be null terminated.
 *
 * @param dest The location to copy into (kernel memory)
 * @param src The location to copy from (user memory)
 * @param max_len The maximum number of characters to copy.
 *
 * @return The number of characters copied.
 */
int v_strcpy(char *dest, char *src, int max_len) {
	int i;
	SAFE_LOOP(src, sizeof(char), i, max_len) {
		if ((dest[i] = *src) == '\0') {
			i++;
			break;
		}
	}
	return i;
}

/**
 * @brief Perform a validated memcpy
 *
 * @param dest The location to copy into (kernel memory)
 * @param src The location to copy from (user memory)
 * @param max_len The maximum number of bytes to copy.
 *
 * @return The number of bytes copied.
 */
int v_memcpy(char *dest, char *src, int max_len) {
	int i;
	SAFE_LOOP(src, sizeof(char), i, max_len) {
		dest[i] = *src;
	}
	return i;
}

