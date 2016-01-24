#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

using namespace std;

int getaddrinfo(const char *node,
				const char *service,
				const struct addrinfo *hints,
				struct addrinfo **results);

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		cerr << "Usage: " << argv[0] << " requires DOMAIN and PORT arguments" << endl ;
		return 1;
	}
	int status;
	struct addrinfo host_info;
	struct addrinfo *host_info_list;

	memset(&host_info, 0, sizeof(host_info));



	return 0;
}