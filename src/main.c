#include <stdio.h>
#include <stdlib.h> // For atoi, exit
#include "packet_crafter.h"

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <source_ip> <dest_ip> <source_port> <dest_port> [payload_string]\n", argv[0]);
        fprintf(stderr, "Example: sudo %s 192.168.1.100 192.168.1.1 12345 80 \"Hello\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *source_ip = argv[1];
    const char *dest_ip = argv[2];
    int source_port = atoi(argv[3]);
    int dest_port = atoi(argv[4]);
    const char *payload = (argc > 5) ? argv[5] : "Default TCP Payload";

    if (source_port <= 0 || source_port > 65535 || dest_port <= 0 || dest_port > 65535) {
        fprintf(stderr, "Invalid port number. Must be between 1 and 65535.\n");
        exit(EXIT_FAILURE);
    }

    printf("Initializing packet sending...\n");
    printf("Source IP: %s, Destination IP: %s\n", source_ip, dest_ip);
    printf("Source Port: %d, Destination Port: %d\n", source_port, dest_port);
    printf("Payload: \"%s\"\n", payload);

    if (create_and_send_tcp_packet(source_ip, dest_ip, source_port, dest_port, payload) != 0) {
        fprintf(stderr, "Failed to send packet.\n");
        return EXIT_FAILURE;
    }

    printf("Packet sending process completed.\n");
    return EXIT_SUCCESS;
}