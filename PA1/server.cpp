//includes for general use
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <ctype.h> //for toupper()

//includes for networking
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

int randomPort(int n_port) //pick a random port
{
    int val = n_port;
    srand(time(NULL));
    while(val == n_port)
        val = rand() % 65535 + 1024;

    return val;
}

void error(const char *msg) //display messages on sys call errors
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, tcpsockfd, n_port, r_port;
    socklen_t clilen;
    char buffer[4];
    struct sockaddr_in serv_addr, cli_addr;
    int n, o;
    FILE *filep;

    if (argc < 2) //check command line args, need n_port
    {
        fprintf(stderr,"ERROR, no negotiation port provided\n");
        exit(1);
    }
    
    //begin negotation stage----------------------------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables

    n_port = atoi(argv[1]); //convert char*[] arg to int
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(n_port); //host to network short

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("Error: unable to bind()");

    // listen() and accept()
    listen(sockfd,5); //listen with a queue of 5
    clilen = sizeof(cli_addr);
    //new file descriptor for connection to client
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
        error("Error: unable to accept()");
    
    //wait for client to request r_port
    n = read(newsockfd,buffer,sizeof(buffer)); 
    if(n < 0)
        error("Error receiving message from client");
    if(strcmp(buffer, "117") != 0)
    {
        printf("Invalid request ID: %s",buffer);
        exit(4);
    }

    r_port = randomPort(n_port); 
    //send r_port
    printf("Negotiation detected. Selected random port %d\n",ntohs(r_port));
    n = write(newsockfd,&r_port,sizeof(r_port));

    close(newsockfd);
    close(sockfd);
    
    //begin UDP transfer stage--------------------------------------------------
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("Error: unable to open socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(r_port); //host to network short

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("Error: unable to bind()");

/*    // listen() and accept()
    listen(sockfd,5); //listen with a queue of 5
    clilen = sizeof(cli_addr);
    //new file descriptor for connection to client
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) 
        error("Error: unable to accept()\n");

    printf("Accepted file transfer on port %d\n",r_port);*/

    size_t length = strlen(buffer);

    filep = fopen("received.txt","a");
/*    do
    {
        n = recvfrom(sockfd, buffer, 4, 0, NULL, NULL);
        if (n < 0)
            error("Error receiving");

    }while(strcmp(buffer,"117") != 0);
    printf("%s receieved. Accepting file transfer.\n",buffer);
    fflush(stdout);
    bzero(buffer, length);
*/
    n = 1;
    while(n > 0)
    {
        n = recvfrom(sockfd, buffer, 4, 0, (struct sockaddr*)&cli_addr, &clilen);
        if(n < 0)
            error("Error receiving message\n");
        fprintf(filep,buffer);
        length = strlen(buffer);

        if(strcmp(buffer,"\0") == 0)
            n = 0;


        for(int i = 0; i < length; i++)
        {
            buffer[i] = toupper(buffer[i]);
        }
        if(buffer[length - 1] == '\0')
            continue;
        o = sendto(sockfd, buffer, length, 0, (struct sockaddr*)&cli_addr, clilen);
        if(o < 0)
            error("Error sending ack\n");
        bzero(buffer,length);
    }
    
    fclose(filep);
    close(sockfd);
    return 0; 
}