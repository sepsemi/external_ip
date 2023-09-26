#include <unistd.h>

#define IPv4_ADDRESS_FAMILY 0x01
#define XOR_MAPPED_ADDRESS_TYPE 0x0020

#define NUM_SERVERS 6

struct stun_server {
    uint8_t *address;
    uint16_t port;
};

struct stun_message_header {
    uint16_t type;
    uint16_t length;
    uint32_t cookie;
    uint32_t identifier[3];
};

struct stun_attribute {
    uint16_t type;
    uint16_t length;
    union {
        struct {
            uint8_t reserved;
            uint8_t family;
            uint16_t port;
            uint32_t address;
        } xor_mapped_address;
    } value;
};

// Stun server ip addresses NOT domains
struct stun_server servers[] = {
    {"74.125.24.127", 3478},
    {"142.93.228.31", 3478},
    {"159.69.191.124", 443},
    {"208.83.246.100", 3478},
    {"65.108.75.112", 3478},
    {"142.251.125.127", 3478},
};

