#ifndef NETWORK_STRUCTS_H
#define NETWORK_STRUCTS_H

#include <netinet/ip.h> // Defines struct ip / iphdr (macOS uses struct ip)
#include <netinet/tcp.h> // Defines struct tcphdr
#include <sys/types.h>   // For u_intXX_t types

// On macOS, <netinet/ip.h> defines 'struct ip' instead of 'struct iphdr'.
// We can use 'struct ip' directly or create an alias if we want to maintain
// the name 'iphdr' for cross-platform code later. For this macOS specific
// MVP, using 'struct ip' is fine.
// struct ip already includes:
//  ip_vhl;  /* version << 4 | header length >> 2 */ (need to handle this)
//  ip_tos;
//  ip_len;  /* total length */
//  ip_id;
//  ip_off;  /* fragment offset field */
//  ip_ttl;
//  ip_p;    /* protocol */
//  ip_sum;  /* checksum */
//  ip_src, ip_dst; /* source and dest address */

// For clarity and easier access to version and header length separately if needed:
// struct custom_iphdr {
//     u_int8_t  iph_ihl:4, iph_version:4; // Internet header length, Version
//     u_int8_t  iph_tos;                 // Type of service
//     u_int16_t iph_tot_len;             // Total length
//     u_int16_t iph_id;                  // Identification
//     u_int16_t iph_frag_off;            // Fragment offset
//     u_int8_t  iph_ttl;                 // Time to live
//     u_int8_t  iph_protocol;            // Protocol
//     u_int16_t iph_check;               // Checksum
//     struct in_addr iph_saddr;          // Source address
//     struct in_addr iph_daddr;          // Destination address
// };
// For this MVP, we'll stick to the system-provided `struct ip` and manage `ip_vhl` directly.


// Pseudo header for TCP checksum calculation
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};

#endif // NETWORK_STRUCTS_H