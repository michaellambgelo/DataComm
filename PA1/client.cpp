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

    FILE *filep;

    char buffer[5];

    if (argc < 4) //check command line args, need hostname, n_port, filename
    {
       fprintf(stderr,"usage %s hostname, negotiation port, filename\n", argv[0]);
       exit(0);
    }

    //begin negotation stage----------------------------------------------------
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
   
    close(sockfd);

    //begin UDP transfer stage---------------------------------------------------
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
    //if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    //    error("Error: unable to connect()\n");

/*    n = sendto(sockfd, secret, sizeof(secret), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (n < 0)
        error("Error sending UDP message");
    printf("Secret sent\n");
    fflush(stdout);*/

/*    n = recvfrom(sockfd, buffer, strlen(buffer), 0, NULL, NULL);
    if(n < 0)
        error("Error receiving ack");
*/
    filep = fopen(argv[3],"r");
    if(filep == NULL)
        error("Error opening file\n");

    bzero(buffer, strlen(buffer));
    while(fgets(buffer,5,filep))
    {
        size_t length = strlen(buffer);
        n = sendto(sockfd, buffer, length, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if(n < 0)
            error("Error sending payload\n");
        n = recvfrom(sockfd, buffer, length, 0, NULL, NULL);
        if(n < 0)
            error("Error receiving payload\n");
        printf("%s\n",buffer);
    }
    char eof[] = "\0";
    n = sendto(sockfd, eof, sizeof(eof), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    fclose(filep);
    printf("File transfer complete\n");

    return 0;
}