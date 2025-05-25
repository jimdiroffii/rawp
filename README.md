# Raw Packet Crafter

## Overview

Craft and send custom TCP packets with manual IP/TCP headers and checksums.

## Requirements

* Root or `CAP_NET_RAW` privileges
* C99-compatible compiler
* POSIX-compliant OS (e.g., Linux or macOS)

## Build

```sh
make clean && make
```

## Usage

```sh
sudo ./packet_sender <source_ip> <dest_ip> <source_port> <dest_port> [payload]
```

## Files

* `main.c` — CLI entry point
* `packet_crafter.c` / `packet_crafter.h` — packet construction and send logic
* `network_structs.h` — IP/TCP header and pseudo-header definitions
* `checksum.c` / `checksum.h` — IP-style checksum calculation

## Example

```sh
sudo ./packet_sender 192.168.1.100 192.168.1.1 12345 80 "Hello"
```
