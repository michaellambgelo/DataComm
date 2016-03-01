//includes for general use
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//includes for networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, n_port, r_port, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    if (argc < 4) //check command line args, need hostname, n_port, filename
    {
       fprintf(stderr,"usage %s hostname, negotiation port, filename\n", argv[0]);
       exit(0);
    }

    //begin negotation stage----------------------------------------------------
    n_port = atoi(argv[2]); 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM for TCP
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(n_port); //negotiation port
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    n = read(sockfd,&r_port,sizeof(r_port) + 1); //get the r_port
    
    printf("Server says random port is %d",r_port);
/*    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);*/
    close(sockfd);
    return 0;
}