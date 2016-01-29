/*
client.cpp
Author: Michael Lamb
Date: 28.1.2016
Description: This program is a client which
sends a file to a server. The server determines
a random port on which to receive the file.
*/

//includes for general use
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//includes for networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

void error(const char *msg) //display messages on sys call errors
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, 
        n_port, 
        r_port, 
        n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    FILE *filep;

    //c-string for messages, assumes [4] will be null terminator
    char buffer[5];

    if (argc < 4) //check command line args, need hostname, n_port, filename
    {
       fprintf(stderr,"usage %s hostname, negotiation port, filename\n", argv[0]);
       exit(0);
    }

    /*************************** 
    Begin negotation stage (TCP)
    After setting up the sockfd,
    send r_port for transfer
    ****************************/

    n_port = atoi(argv[2]); 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("Error opening socket\n");

    server = gethostbyname(argv[1]);
    if (server == NULL) 
    {
        fprintf(stderr,"Error, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(n_port); //negotiation port
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("Error connecting\n");

    //request r_port by sending shared secret '117'
    char secret[] = "117";
    n = write(sockfd,secret,sizeof(secret));
    if(n < 0)
        error("Unable to establish trust with server");

    n = read(sockfd,&r_port,sizeof(r_port)); //get the r_port
    if(n < 0)
        error("Error getting transfer port\n");
    
    printf("Communicating with server. Transferring file on port %d\n",ntohs(r_port));
   
    //close open socket per PA1 instructions
    close(sockfd);

    /*******************************
    Begin file transfer stage (UDP)
    After setting up the socket,
    iterate through the given file
    and send every 4 bytes to server
    ********************************/

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); //SOCK_DGRRAM for UDP
    if (sockfd < 0) 
        error("Error opening socket\n");

    server = gethostbyname(argv[1]);
    if (server == NULL) 
    {
        fprintf(stderr,"Error, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); //clear variables
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(r_port); //negotiation port

    filep = fopen(argv[3],"r");
    if(filep == NULL)
        error("Error opening file\n");

    size_t length = strlen(buffer);
    bzero(buffer, length); //just to be safe, y'know

    while(fgets(buffer,5,filep)) //give until there's nothing left...
    {
        length = strlen(buffer); //update size of string

        n = sendto(sockfd, buffer, length, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if(n < 0)
            error("Error sending payload\n");

        //ack should be same message, all caps
        n = recvfrom(sockfd, buffer, length, 0, NULL, NULL);
        if(n < 0)
            error("Error receiving payload\n");
        printf("%s\n",buffer);
    }

    //once the file's a goner, send EOF
    char eof[] = "\0";
    n = sendto(sockfd, eof, sizeof(eof), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    //cleanup
    fclose(filep);
    printf("File transfer complete\n");

    return 0;
}