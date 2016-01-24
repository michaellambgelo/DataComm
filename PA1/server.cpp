#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		cerr << "Usage: " << argv[0] << " requires DOMAIN and PORT arguments" << endl ;
		return 1;
	}

	int r_port = srand(time(NULL)) % 65535 + 1023;

	int status;
	struct addrinfo host_info;
	struct addrinfo *servinfo;
	struct sockaddr_storage connect_ip; // connector's address information


	memset(&host_info, 0, sizeof(host_info)); //clear memory
	hints.ai_family =  AF_UNSPEC; //unspecified, IPv4/IPv6
	hints.ai_socktype = SOCK_STREAM; //TCP
	hints.ai_flags = AI_PASSIVE;

	host_info = argv[1]; //domain or IP
	int n_port = argv[2]; //negotiation port

	if ((status = getaddrinfo(NULL, n_port, &hints, &servinfo)) != 0) 
	{
	    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	    exit(1);
	}

	// make a socket:
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// bind it to the port we passed in to getaddrinfo():
	bind(sockfd, res->ai_addr, res->ai_addrlen);

	return 0;
}