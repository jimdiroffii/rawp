#ifndef PACKET_CRAFTER_H
#define PACKET_CRAFTER_H

#include "network_structs.h" // For struct ip, tcphdr, pseudo_header
#include <netinet/in.h>      // For struct sockaddr_in

/**
 * @brief Creates and sends a TCP packet.
 *
 * @param source_ip Source IP address string.
 * @param dest_ip Destination IP address string.
 * @param source_port Source port number.
 * @param dest_port Destination port number.
 * @param payload Payload string for the TCP packet.
 * @return 0 on success, -1 on failure.
 */
int create_and_send_tcp_packet(const char *source_ip, const char *dest_ip,
                               int source_port, int dest_port, const char *payload);

#endif // PACKET_CRAFTER_H