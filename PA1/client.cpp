#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

using namespace std;

int getaddrinfo(const char *node, 		//domain or IP
				const char *service, 	//"http" or port number
				const struct addrinfo *hints,
				struct addrinfo **results);

int main(int argc, char* argv[])
{
	if(argc != 4)
	{
		cerr << "Usage: " << argv[0] << " requires DOMAIN, PORT, FILENAME arguments" << endl ;
		return 1;
	}

	int status;
	struct addrinfo host_info;
	struct addrinfo *host_info_list;

	memset(&host_info, 0, sizeof(host_info)); //clear memory
	hints.ai_family =  AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	host_info = argv[1];
	int n_port = argv[2];

	return 0;
}