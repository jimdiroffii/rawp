#include "packet_crafter.h"
#include "checksum.h"
#include "network_structs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>

#define IP4_VERSION 4
#define IP4_HDR_LEN_WORDS 5

// Function to get the primary network interface IP
int get_interface_ip(char *ip_str, size_t ip_str_len) {
    struct ifaddrs *ifaddrs_ptr, *ifa;
    int found = 0;
    
    if (getifaddrs(&ifaddrs_ptr) == -1) {
        perror("getifaddrs");
        return -1;
    }
    
    for (ifa = ifaddrs_ptr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        // Look for IPv4 addresses on non-loopback interfaces
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr_in = (struct sockaddr_in *)ifa->ifa_addr;
            char *addr_str = inet_ntoa(addr_in->sin_addr);
            
            // Skip loopback and zero addresses
            if (strcmp(addr_str, "127.0.0.1") != 0 && 
                strcmp(addr_str, "0.0.0.0") != 0 &&
                !(ifa->ifa_flags & IFF_LOOPBACK)) {
                
                strncpy(ip_str, addr_str, ip_str_len - 1);
                ip_str[ip_str_len - 1] = '\0';
                found = 1;
                printf("Using interface %s with IP %s\n", ifa->ifa_name, addr_str);
                break;
            }
        }
    }
    
    freeifaddrs(ifaddrs_ptr);
    return found ? 0 : -1;
}

// Function to validate IP routing compatibility
int validate_routing(const char *source_ip_str, const char *dest_ip_str) {
    struct in_addr src_addr, dst_addr;
    
    if (inet_pton(AF_INET, source_ip_str, &src_addr) <= 0 ||
        inet_pton(AF_INET, dest_ip_str, &dst_addr) <= 0) {
        return -1;
    }
    
    // Check if both are loopback
    if ((ntohl(src_addr.s_addr) >> 24) == 127 && 
        (ntohl(dst_addr.s_addr) >> 24) == 127) {
        printf("Both source and destination are loopback - OK\n");
        return 0;
    }
    
    // Check if source is loopback but destination is not
    if ((ntohl(src_addr.s_addr) >> 24) == 127 && 
        (ntohl(dst_addr.s_addr) >> 24) != 127) {
        printf("WARNING: Source is loopback but destination is external\n");
        printf("This may cause routing issues on macOS\n");
        return 1; // Warning, but continue
    }
    
    // Check if both are external
    if ((ntohl(src_addr.s_addr) >> 24) != 127 && 
        (ntohl(dst_addr.s_addr) >> 24) != 127) {
        printf("Both source and destination are external - OK\n");
        return 0;
    }
    
    return 0;
}

int create_and_send_tcp_packet(const char *source_ip_str, const char *dest_ip_str,
                               int source_port, int dest_port, const char *payload) {
    int sockfd;
    char datagram[4096];
    struct sockaddr_in sin_dest;
    struct pseudo_header psh;
    int payload_len = strlen(payload);
    char actual_source_ip[INET_ADDRSTRLEN];
    
    // Validate and potentially fix routing
    printf("Validating routing...\n");
    int routing_status = validate_routing(source_ip_str, dest_ip_str);
    
    // If we have a routing warning and source is loopback, try to use a real interface IP
    if (routing_status == 1) {
        printf("Attempting to use actual interface IP instead of loopback...\n");
        if (get_interface_ip(actual_source_ip, sizeof(actual_source_ip)) == 0) {
            printf("Switching source IP from %s to %s\n", source_ip_str, actual_source_ip);
            source_ip_str = actual_source_ip;
        } else {
            printf("Could not determine interface IP, continuing with %s\n", source_ip_str);
        }
    }
    
    // Calculate total packet size
    int ip_header_len = IP4_HDR_LEN_WORDS * 4;
    int tcp_header_len = sizeof(struct tcphdr);
    int total_len = ip_header_len + tcp_header_len + payload_len;
    
    printf("\nPacket size calculation:\n");
    printf("  IP header: %d bytes\n", ip_header_len);
    printf("  TCP header: %d bytes\n", tcp_header_len);
    printf("  Payload: %d bytes\n", payload_len);
    printf("  Total: %d bytes\n", total_len);

    // Create raw socket
    printf("\nCreating raw socket...\n");
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("socket() creation failed");
        return -1;
    }
    printf("Raw socket created successfully (fd: %d)\n", sockfd);

    // Set up destination address
    memset(&sin_dest, 0, sizeof(sin_dest));
    sin_dest.sin_family = AF_INET;
    if (inet_pton(AF_INET, dest_ip_str, &sin_dest.sin_addr) <= 0) {
        perror("inet_pton() for destination IP failed");
        close(sockfd);
        return -1;
    }
    printf("Destination address set: %s\n", dest_ip_str);

    // Zero out the packet buffer
    memset(datagram, 0, sizeof(datagram));

    // Get pointers to headers
    struct ip *iph = (struct ip *)datagram;
    struct tcphdr *tcph = (struct tcphdr *)(datagram + ip_header_len);

    // Fill in the IP Header
    printf("Filling IP header...\n");
    iph->ip_v = IP4_VERSION;
    iph->ip_hl = IP4_HDR_LEN_WORDS;
    iph->ip_tos = 0;
    iph->ip_len = htons(total_len);
    iph->ip_id = htons(getpid() & 0xFFFF);
    iph->ip_off = 0;
    iph->ip_ttl = 64;
    iph->ip_p = IPPROTO_TCP;
    iph->ip_sum = 0;

    if (inet_pton(AF_INET, source_ip_str, &iph->ip_src) <= 0) {
        perror("inet_pton() for source IP failed");
        close(sockfd);
        return -1;
    }
    iph->ip_dst = sin_dest.sin_addr;

    // Fill in the TCP Header
    printf("Filling TCP header...\n");
    tcph->th_sport = htons(source_port);
    tcph->th_dport = htons(dest_port);
    tcph->th_seq = htonl(12345);
    tcph->th_ack = 0;
    tcph->th_off = tcp_header_len / 4;
    tcph->th_flags = TH_SYN;
    tcph->th_win = htons(5840);
    tcph->th_sum = 0;
    tcph->th_urp = 0;

    // Copy payload
    printf("Copying payload...\n");
    char *tcp_payload_ptr = (char *)tcph + tcp_header_len;
    memcpy(tcp_payload_ptr, payload, payload_len);

    // Calculate IP checksum
    printf("Calculating IP checksum...\n");
    unsigned short ip_checksum = calculate_checksum(iph, ip_header_len);
    iph->ip_sum = htons(ip_checksum);

    // Calculate TCP checksum using pseudo header
    printf("Calculating TCP checksum...\n");
    psh.source_address = iph->ip_src.s_addr;
    psh.dest_address = iph->ip_dst.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(tcp_header_len + payload_len);

    int tcp_segment_len = tcp_header_len + payload_len;
    int psh_len = sizeof(struct pseudo_header) + tcp_segment_len;
    char *pseudogram = malloc(psh_len);
    if (!pseudogram) {
        perror("malloc for pseudogram failed");
        close(sockfd);
        return -1;
    }

    memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), tcph, tcp_segment_len);

    unsigned short tcp_checksum = calculate_checksum(pseudogram, psh_len);
    tcph->th_sum = htons(tcp_checksum);
    free(pseudogram);

    // Tell kernel we're providing the IP header
    printf("Setting IP_HDRINCL socket option...\n");
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt(IP_HDRINCL) failed");
        close(sockfd);
        return -1;
    }

    // Send the packet
    printf("\nSending TCP SYN packet to %s:%d from %s:%d with payload \"%s\"...\n",
           dest_ip_str, dest_port, source_ip_str, source_port, payload);

    ssize_t bytes_sent = sendto(sockfd, datagram, total_len, 0,
                                (struct sockaddr *)&sin_dest, sizeof(sin_dest));

    if (bytes_sent < 0) {
        printf("sendto() failed: %s (errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    } else {
        printf("Packet sent successfully! Total length: %ld bytes.\n", bytes_sent);
    }

    close(sockfd);
    return 0;
}