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
    while(val == n_port) //ensure r_ is different from n_s
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
    int sockfd,     //initial file descriptor
        newsockfd,  //client sock file descriptor 
        n_port,     //negotiation port, from argv
        r_port;     //random port
    socklen_t clilen;

    //c-string for messages, assumes [4] will be null terminator
    char buffer[5];

    struct sockaddr_in serv_addr, cli_addr;
    int n, o;       //used for write(), read(), sendto(), recvfrom()
    FILE *filep;    //file pointer to "received.txt"

    if (argc < 2) //check command line args, need n_port
    {
        fprintf(stderr,"ERROR, no negotiation port provided\n");
        exit(1);
    }
    
    /*************************** 
    Begin negotation stage (TCP)
    After setting up the sockfd,
    send r_port for transfer
    ****************************/

    //initial socket file descriptor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables

    //sockaddr_in struct arguments
    n_port = atoi(argv[1]); //convert char*[] arg to int
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(n_port); //host to network short

    //bind socket file descriptor to sockaddr_in struct
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("Error: unable to bind()");

    // listen() and accept()
    listen(sockfd,5); //listen with a queue of 5

    printf("Listening for connections.\n");
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

    //close all open sockets, per Programming Assignment 1 instructions
    close(newsockfd); 
    close(sockfd);
    
    /*******************************
    Begin file transfer stage (UDP)
    After setting up the socket,
    write the payload received to
    the output file
    ********************************/

    //datagram socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("Error: unable to open socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables

    //sockaddr_in struct arguments
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(r_port); //host to network short

    //bind the socket and port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("Error: unable to bind()");

    size_t length = strlen(buffer); //get the number of chars in the buffer

    filep = fopen("received.txt","a"); //append mode

    do
    {
        //get payload from client
        n = recvfrom(sockfd, buffer, 5, 0, (struct sockaddr*)&cli_addr, &clilen);
        if(n < 0)
            error("Error receiving message\n");

        fprintf(filep,buffer); //output to file
        
        if(strcmp(buffer,"\0") == 0) //check if eof was sent
            n = 0;

        length = strlen(buffer); //update c-string length
        for(int i = 0; i < length; i++)
        {
            buffer[i] = toupper(buffer[i]); //capitalize message for ack
        }

        //send ack to client
        o = sendto(sockfd, buffer, length, 0, (struct sockaddr*)&cli_addr, clilen);
        if(o < 0)
            error("Error sending ack\n");
        
        bzero(buffer,length); //clear buffer, just to be safe

    }while(n > 0); //as long as there are bytes received...

    fclose(filep);
    close(sockfd);

    return 0; 
}