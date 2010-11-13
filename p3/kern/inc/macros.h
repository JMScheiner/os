/** @file macros.h
 *
 * @brief Helpful macros
 *
 * @author Tim Wilson
 * @author Justin Scheiner
 */

#ifndef MACROS_H_DWQIU23
#define MACROS_H_DWQIU23

/** @brief Set a flag in a bit vector. */
#define SET(bit_vector, flag) \
   bit_vector |= flag

/** @brief Unset a flag in a bit vector. */
#define UNSET(bit_vector, flag) \
   bit_vector &= ~flag

/**
 * @brief Align an address by rounding up to an alignment boundary.
 *
 * @param addr The address to align.
 * @param align The number of bytes to align to.
 *
 * @return The aligned address.
 */
#define ALIGN_UP(addr, align) \
   (void *)((((unsigned int)(addr) + (align) - 1) / (align)) * (align))

/**
 * @brief Align an address by rounding down to an alignment boundary.
 *
 * @param addr The address to align.
 * @param align The number of bytes to align to.
 *
 * @return The aligned address.
 */
#define ALIGN_DOWN(addr, align) \
   (void *)(((unsigned int)(addr) / (align)) * (align))

#endif
