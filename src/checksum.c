#include "checksum.h"
#include <netinet/in.h> // For htons (though not used directly in this function)
#include <stddef.h>     // For size_t

// Generic checksum calculation function (IP-style)
// The buffer is treated as a sequence of 16-bit words.
// The result is returned in host byte order.
unsigned short calculate_checksum(const void *ptr, size_t len) { // <--- UPDATED SIGNATURE
    const unsigned short *buf = ptr;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) { // Odd byte
        sum += *(unsigned char *)buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF); // Fold high 16 bits to low 16 bits
    sum += (sum >> 16);                 // Add carry
    result = ~sum;                      // One's complement
    return result;
}