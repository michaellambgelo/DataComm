/*
client.cpp
LOOK AT ALL THIS RANDOM SHIT
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
#include <time.h> //for timeouts
#include <errno.h>

//includes for networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

//other includes
#include "packet.h"

//packet type constants
#define PACKET_ACK 0
#define PACKET_DATA 1
#define PACKET_EOT_SERV2CLI 2
#define PACKET_EOT_CLI2SERV 3


using namespace std;

void error(const char *msg) //display messages on sys call errors
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 5) //check command line args
    {
        fprintf(stderr,"USAGE: %s, ",argv[0]);  //0
        printf("emulator name, ");              //1
        printf("emulator send port, ");         //2
        printf("emulator receive port, ");      //3
        printf("filename\n");                   //4
        exit(0);
    }

    /*      set up variables      */
    int sendSock, recvSock, n;
    struct sockaddr_in send_serv_addr, recv_serv_addr;
    struct hostent *server;
    socklen_t recvlen;
    char buffer[32];

    FILE *filep;        //given by command line argument
    FILE *seqnum_log;   //"seqnum.log" - log the packet sequence number
    FILE *ack_log;      //"ack.log" - log the received ACK sequence number
    
    //clock_t timer;
    /* 
        To get time, set timer
            timer = clock();
        Then, get the difference
            timer = clock() - timer;
        Note on converting timer to ms:
            int msec = timer * 1000 / CLOCKS_PER_SEC; 
    */

    /*          packet variables             */
    packet *pack = new packet(0,0,0,buffer);
    char serialPacket[48];
    int seqnum;
    int length;

    /*      parse command line arguments     */
    char *emulatorName = argv[1];
    int sendToEmulatorPort = atoi(argv[2]);
    int recvFromEmulatorPort = atoi(argv[3]);
    
    /*          GBN functionality vars      */
    int window_size = 8;
    char packArr[window_size+1][256];
    int max_seqnum;
    int base_seqnum;

    /*          setup file logging          */
    filep = fopen(argv[4],"r");
    seqnum_log = fopen("seqnum.log","w");
    ack_log = fopen("ack.log","w");

    /*      set up sockets and structs       */

    //sending socket
    sendSock = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sendSock < 0) 
        error("Error opening sendSock\n");

    //set recvSock options
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 800000; //800 ms
    
    //receiving socket
    recvSock = socket(AF_INET,SOCK_DGRAM, 0);
    if (recvSock < 0)
        error("Error opening recvSock");
    
    if (setsockopt(recvSock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        error("Error setting socket option");
    //get server (emulator) address
    server = gethostbyname(emulatorName);
    if (server == NULL) 
    {
        fprintf(stderr,"Error, no such host\n");
        exit(0);
    }

    //set up sockaddr_in for SENDING
    bzero((char *) &send_serv_addr, sizeof(send_serv_addr));
    send_serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&send_serv_addr.sin_addr.s_addr, server->h_length);
    send_serv_addr.sin_port = htons(sendToEmulatorPort); 

    //set up sockaddr_in for RECEIVING
    bzero((char *) &recv_serv_addr, sizeof(recv_serv_addr));
    recv_serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&recv_serv_addr.sin_addr.s_addr, server->h_length);
    recv_serv_addr.sin_port = htons(recvFromEmulatorPort);

    
    // if (bind(sendSock, (struct sockaddr *) &send_serv_addr, sizeof(send_serv_addr)) < 0)
    //     error("Error: unable to bind() sendSock()");
    // printf("Ready to send on port %d\n",sendToEmulatorPort);

    if (bind(recvSock, (struct sockaddr *) &recv_serv_addr, sizeof(recv_serv_addr)) < 0)
        error("Error: unable to bind() recvSock");
    printf("Ready to receive on port %d\n",recvFromEmulatorPort);
    recvlen = sizeof(recv_serv_addr);

    //clear c-strings and set up packet variables
    bzero(buffer, sizeof(buffer));
    bzero(serialPacket, sizeof(serialPacket));
    seqnum = 0;
    base_seqnum = 0;
    window_size = 8;
    max_seqnum = window_size - 1;

    printf("Preparing packets to send...\n");

    while(pack->getType() != PACKET_EOT_SERV2CLI)
    {
        while (fgets(buffer, 31, filep) && (seqnum%8) <= window_size){
   
            //prepare packet if there's data
            delete pack;
            if(strlen(buffer) > 0)
            {
                pack = new packet(PACKET_DATA, seqnum%8, sizeof(buffer), buffer);

            }
            else
            {
                pack = new packet(PACKET_EOT_CLI2SERV, seqnum%8, 0, NULL);
            }
            cout<<"Populating packArr..."<<endl;
            bzero(packArr[seqnum%8], sizeof(packArr[seqnum%8]));
            pack->serialize(packArr[seqnum%8]);
            //send packet
            n = sendto(sendSock, packArr[seqnum%8], sizeof(packArr[seqnum%8]), 0, (struct sockaddr*)&send_serv_addr, sizeof(send_serv_addr));
            if(n < 0)
                error("Error sending packet");
            else
                cout << "Packet sent:" << endl;
            pack->printContents();
            fprintf(seqnum_log,"%d\n", seqnum%8);
            seqnum++;
        }

        bzero(serialPacket, sizeof(serialPacket));

        //wait for Ack
        cout<<"Waiting to receive..."<<endl<<endl; 
        n = recvfrom(recvSock, serialPacket, sizeof(serialPacket), 0, NULL, NULL);
        if (errno == EAGAIN){
            for (int i = base_seqnum; i != seqnum%8; ((i+1)%8))
            {
                cout<<"Resending packet: "<< i << endl;
                n = sendto(sendSock, packArr[i], sizeof(packArr[i]), 0, (struct sockaddr*)&send_serv_addr, sizeof(send_serv_addr));
                if(n < 0)
                    error("Error sending packet");
                else
                    cout << "Packet resent:" << endl;
                pack->printContents();
                fprintf(seqnum_log,"%d\n", seqnum%8);
            }
        }

        if(n < 0)
            error("Error receiving packet");
        else
            cout << "Packet received:" << endl;

        pack->deserialize(serialPacket);
        
        //Hannah started shit right here
        if (pack->getSeqNum() == base_seqnum)
        {
            base_seqnum++;
        } 
        else if (pack->getSeqNum() < base_seqnum)
        {
            for (int i = base_seqnum; i != seqnum%8; i++)
            {
                cout<<"Resending packet: "<< i << endl;
                n = sendto(sendSock, packArr[i], sizeof(packArr[i]), 0, (struct sockaddr*)&send_serv_addr, sizeof(send_serv_addr));
                if(n < 0)
                    error("Error sending packet");
                else
                    cout << "Packet resent:" << endl;
                pack->printContents();
                fprintf(seqnum_log,"%d\n", seqnum%8);
                seqnum = (seqnum + 1) % 8;

            }
        }
        fprintf(ack_log, "%d\n", pack->getSeqNum());
        pack->printContents();


        bzero(buffer, sizeof(buffer));
        bzero(serialPacket, sizeof(serialPacket));
    }

    //cleanup
    fclose(filep);
    fclose(seqnum_log);
    fclose(ack_log);
    close(sendSock);
    close(recvSock);
    return 0;
}