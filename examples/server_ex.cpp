//includes for general use
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

//includes for networking
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int randomPort(int n_port) //pick a random port
{
    int val;
    srand(time(NULL));
    do{
    val = rand() % 65535 + 1024
    }while(val != n_port); //ensure new port

    return val;
}

void error(const char *msg) //display messages on sys call errors
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, filesockfd, n_port, r_port;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    if (argc < 2) //check command line args, need n_port
    {
        fprintf(stderr,"ERROR, no negotiation port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables

    n_port = atoi(argv[1]); //convert char*[] arg to int
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(n_port); //host to network short

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    // listen() and accept()
    listen(sockfd,5); //listen with a queue of 5
    clilen = sizeof(cli_addr);
    //new file descriptor for connection to client
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
        error("ERROR on accept");

    r_port = randomPort(n_port); 
    //send r_port
    n = write(newsockfd,&r_port,sizeof(r_port));

/*     bzero(buffer,256); //clear buffer for message
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");*/
    close(newsockfd);
    close(sockfd);
    return 0; 
}