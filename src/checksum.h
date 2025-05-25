#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <sys/types.h> // For unsigned short
#include <stddef.h>    // For size_t

/**
 * @brief Calculates the IP-style checksum for the given buffer.
 *
 * @param ptr Pointer to the data buffer.
 * @param len Length of the buffer in bytes.
 * @return The calculated checksum in host byte order (caller should htons() if needed for network).
 */
unsigned short calculate_checksum(const void *ptr, size_t len); // <--- UPDATED SIGNATURE

#endif // CHECKSUM_H