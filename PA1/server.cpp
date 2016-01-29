/*
server.cpp
Author: Michael Lamb
Date: 28.1.2016
Description: This program is a server which
can receive and store a file from a client on 
a random port.
*/

//includes for general use
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    while(val == n_port) //ensure r_ is different from n_
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
    int sockfd, 
        newsockfd, 
        tcpsockfd, 
        n_port, 
        r_port;
    socklen_t clilen;

    //c-string for messages, assumes [4] will be null terminator
    char buffer[5];

    struct sockaddr_in  serv_addr, 
                        cli_addr;
    int n, 
        o;
    FILE *filep;

    if (argc < 2) //check command line args, need n_port
    {
        fprintf(stderr,"Error, no negotiation port provided\n");
        exit(1);
    }
    
    /*************************** 
    Begin negotation stage (TCP)
    After setting up the sockfd,
    send r_port for transfer
    ****************************/

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("Error opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables

    n_port = atoi(argv[1]); //convert char*[] arg to int
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(n_port); //host to network short

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("Error: unable to bind()");

    printf("Waiting for connections...\n");

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

    //close open sockets per PA1 instructions
    close(newsockfd);
    close(sockfd);
    
    /*******************************
    Begin file transfer stage (UDP)
    After setting up the socket,
    write the payload received to
    the output file
    ********************************/

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("Error: unable to open socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(r_port); //host to network short

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("Error: unable to bind()");

    size_t length = strlen(buffer); //used to keep track of string

    filep = fopen("received.txt","a");

    do    
    {
        n = recvfrom(sockfd, buffer, 5, 0, (struct sockaddr*)&cli_addr, &clilen);
        if(n < 0)
            error("Error receiving message\n");
        fprintf(filep,buffer);
        length = strlen(buffer); //update string length

        if(strcmp(buffer,"\0") == 0) //is payload EOF?
            n = 0;

        //ack is capitalized version of payload
        for(int i = 0; i < length; i++)
        {
            buffer[i] = toupper(buffer[i]);
        }

        o = sendto(sockfd, buffer, length, 0, (struct sockaddr*)&cli_addr, clilen);
        if(o < 0)
            error("Error sending ack\n");
        bzero(buffer,length);

    }while(n > 0);

    //cleanup
    fclose(filep);
    close(sockfd);
    return 0; 
}