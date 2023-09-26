#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "stun.h"

#define BUFFER_SIZE 1024
#define REQUEST_LENGTH 0x0000
#define MAGIC_COOKIE 0x2112A442
#define BIND_REQUEST_TYPE 0x0001

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif


struct ctx_client {
    uint16_t port;
    const char *address;
    int fd;
};

bool create_socket(struct ctx_client *ctx) {
    
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) <0) {
        perror("Socket creation error: ");
    }

    // Make the socket reusable
    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

    // Set a timeout on the socket (500 ms)
    struct timeval timeout = {
        .tv_sec = 0,
        .tv_usec = 500000
    };

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    
    ctx->fd = fd;
}

struct stun_message_header create_bind_request_packet() {
    struct stun_message_header header;

    header.length = htons(REQUEST_LENGTH);
    header.cookie = htonl(MAGIC_COOKIE);
    header.type = htons(BIND_REQUEST_TYPE);

    for (size_t i =0; i < 3; i++) {
        srand((uint32_t) time(0));
        header.identifier[i] = rand();
    }

    return header;
}

bool send_packet_to_remote(struct ctx_client *client, struct stun_message_header packet) {

    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_port = htons(client->port);
    
    // set the address of the remote

    inet_pton(AF_INET, client->address, &address.sin_addr);

    // connect to the remote
    if (connect(client->fd, (struct sockaddr *)&address, sizeof(struct sockaddr)) <0) {
        perror("Connect:");
    }
    
    DEBUG_PRINT("[DEBUG]: Connected to %s:%d\n", client->address, client->port);

    // send the packet
    if (sendto(client->fd, &packet, sizeof(struct stun_message_header), 0, (struct sockaddr *)&address, sizeof(struct sockaddr)) == -1) {
        perror("Failed to send:");
        return false;
    }

    DEBUG_PRINT("[DEBUG]: Successfully send bind packet\n");
    return true;
}

bool check_response_identifier(struct stun_message_header packet, struct stun_message_header response) {
    if (response.type != htons(0x0101)) {
        DEBUG_PRINT("[DEBUG]: Invalid response type\n");
        return 0;
    }

    for (size_t i = 0; i < 3; i++) {
        if (packet.identifier[i] != response.identifier[i]) {
            return false;
        }
    }

    return true;
}

bool parse_stun_response(struct stun_message_header *response, size_t length) {
    // Calculate the end of the buffer
    char *end = (char *)response + ntohs(response->length) + 20; // 20 bytes for STUN header

    // Skip the STUN header
    char *attr = (char *)response + 20;

    while (attr < end) {
        struct stun_attribute *attribute = (struct stun_attribute *)attr;

        if (!ntohs(attribute->type) == XOR_MAPPED_ADDRESS_TYPE) {
            return false;
        }
        
        attr += ntohs(attribute->length) + 4; // Move to next attribute

        uint16_t xor_port = ntohs(attribute->value.xor_mapped_address.port);
        uint32_t xor_addr = ntohl(attribute->value.xor_mapped_address.address);

        uint16_t port = xor_port ^ (MAGIC_COOKIE >> 16);
        uint32_t addr = xor_addr ^ MAGIC_COOKIE;

        struct in_addr inaddr;
        inaddr.s_addr = htonl(addr);

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &inaddr, ip_str, sizeof(ip_str));

        printf("%s:%d\n", ip_str, port);

        // We don't care about other results, the first one is the right one.
        return true;
    }

    return false;
}

bool get_external_ip_address(struct stun_server server) {
    // Create a client context
    struct ctx_client client = {
        .port = server.port,
        .address = server.address
    };

    bool success = create_socket(&client);
    
    // Create a bind request packet
    struct stun_message_header packet =  create_bind_request_packet();
    bool result = send_packet_to_remote(&client, packet);
    
    char buffer[BUFFER_SIZE] = {0};
    size_t nbytes = read(client.fd, buffer, sizeof(char[BUFFER_SIZE]));

    if (nbytes <0) {
        DEBUG_PRINT("[DEBUG]: Recieved nbytes=%d, faild to parse response\n", nbytes);
        return false;
    }   

    // Parse the response from remote 
    struct stun_message_header *response = (struct stun_message_header *)buffer;
 
    if (!check_response_identifier(packet, *response)) {
        DEBUG_PRINT("[DEBUG]: Invalid identifier\n");
        return false;
    }

    if(!parse_stun_response(response, nbytes)) {
        DEBUG_PRINT("[DEBUG]: Failed to parse responsei\n");
        return false;
    }

    return true;
}
    
int main(int argc, char **argv[]) {

    for (size_t i =0; i < NUM_SERVERS; i++) {
        DEBUG_PRINT("[DEBUG]: Connecting using: index=%zu, address=%s, port=%d\n", i, servers[i].address, servers[i].port);
    
        if(get_external_ip_address(servers[i])) {
            return 0;
        }

    }

    printf("Failed to get ip using %zu servers\n", NUM_SERVERS);

    return 1;
}
