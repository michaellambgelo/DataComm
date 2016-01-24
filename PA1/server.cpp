#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

int getaddrinfo(const char *node,
				const char *service,
				const struct addrinfo *hints,
				struct addrinfo **results);

int main()
{
	return 0;
}