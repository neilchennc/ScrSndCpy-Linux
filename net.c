#include "net.h"

#include <arpa/inet.h>

int is_valid_ip_address(char *address)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, address, &(sa.sin_addr));
    return result != 0;
}
