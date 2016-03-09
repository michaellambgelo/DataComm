/*
server.cpp
Author: Michael Lamb
Date: 28.1.2016
Description: This program is a server which
can receive and store a file from a client on 
a random port.

This client-server distribution is based on example
code from http://www.linuxhowtos.org/C_C++/socket.htm

Code sections obtained from this page are noted below.
*/

//includes for general use
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

//includes for networking
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

//other includes
#include "packet.h"

//packet type constants
#define PACKET_ACK 0
#define PACKET_DATA 1
#define PACKET_EOT_SERV2CLI 2
#define PACKET_EOT_CLI2SERV 3

using namespace std;

int randomPort(int n_port) //pick a random port
{
    int val = n_port;
    srand(time(NULL));
    while(val == n_port) //ensure r_ is different from n_
        val = rand() % (65535 - 1024) + 1024;

    return val;
}

/*
error(msg) function obtained from 
http://www.linuxhowtos.org/C_C++/socket.htm
*/
void error(const char *msg) //display messages on sys call errors
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 5) //check command line args
    {
        fprintf(stderr,"USAGE: %s, ",argv[0]);  //0
        printf("emulator name, ");              //1
        printf("emulator receive port, ");      //2
        printf("emulator send port, ");         //3
        printf("filename\n");                   //4
        exit(0);
    }

    /*      set up variables      */
    int sendSock, recvSock, n;
    socklen_t recvlen;

    struct sockaddr_in  send_cli_addr, 
                        recv_cli_addr;
    char buffer[32];

    FILE *filep;        //command line argument, write received payloads
    FILE *arrival_log;  //"arrival.log" - log sequence number of packets

    /*          packet variables             */
    char serialPacket[256];
    int type;
    int packet_seqnum;
    int expected_seqnum;
    int last_seqnum;
    int length;
    char data[32];
    packet *pack = new packet(0,0,0,data);


    /*      parse command line arguments     */
    char *emulatorName = argv[1];
    int recvFromEmulatorPort = atoi(argv[2]);
    int sendToEmulatorPort = atoi(argv[3]);
    
    /*          setup file logging          */
    filep = fopen(argv[4],"w");
    arrival_log = fopen("arrival.log","w");

    /*      set up sockets and structs       */

    //sending socket
    sendSock = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sendSock < 0) 
        error("Error opening sendSock\n");

    //receiving socket
    recvSock = socket(AF_INET,SOCK_DGRAM, 0);
    if (recvSock < 0)
        error("Error opening recvSock");

    //set up sockaddr_in for SENDING
    bzero((char *) &send_cli_addr, sizeof(send_cli_addr)); //clear variables
    send_cli_addr.sin_family = AF_INET;
    send_cli_addr.sin_addr.s_addr = INADDR_ANY;
    send_cli_addr.sin_port = htons(sendToEmulatorPort); //send port

    //set up sockaddr_in for RECEIVING
    bzero((char *) &recv_cli_addr, sizeof(recv_cli_addr)); //clear variables
    recv_cli_addr.sin_family = AF_INET;
    recv_cli_addr.sin_addr.s_addr = INADDR_ANY;
    recv_cli_addr.sin_port = htons(recvFromEmulatorPort); //recv port

    //bind socks to ports
    if (bind(recvSock, (struct sockaddr *) &recv_cli_addr, sizeof(recv_cli_addr)) < 0)
        error("Error: unable to bind() recvSock");
        printf("Ready to receive on port %d\n",recvFromEmulatorPort);

    //clear c-strings and setup packet variables
    bzero(buffer, sizeof(buffer));
    bzero(data, sizeof(data));
    bzero(serialPacket, sizeof(serialPacket));
    packet_seqnum = 0;
    expected_seqnum = 0;
    last_seqnum = 0;


    do
    {
        n = recvfrom(recvSock, serialPacket, sizeof(serialPacket), 0, NULL, NULL);
        if(n < 0) 
            error("Error receiving packet"); 
        else
            cout << "Packet received:" << endl;

        pack->deserialize(serialPacket);

        type = pack->getType();
        packet_seqnum = pack->getSeqNum();

        if(type == PACKET_DATA)
        {
            //Hannah started typing here
            //If the seqnum is right, send an ack with that seqnum. 
            //If the expected seqnum reaches 8, reset to 0.
            if (packet_seqnum == expected_seqnum)
            {
                cout << "Received: " << packet_seqnum<<endl;
                last_seqnum = packet_seqnum;
                pack->printContents();
                fprintf(filep,data);
                delete pack;
                pack = new packet(PACKET_ACK, expected_seqnum++, 0, NULL);
                pack->serialize(serialPacket);
                //If it get's to 8, make it expect 0
                if (expected_seqnum == 8)
                {
                    cout << "Expecting 8...now expecting 0."<< endl;
                    expected_seqnum = 0;
                }
            }
            else
            {   
                cout << "Unexpected sequence number. Resending ACK for: " << last_seqnum << endl;
                cout << "Expecting packet: " << expected_seqnum << endl; 
                delete pack;
                pack = new packet(PACKET_ACK, last_seqnum, 0, NULL);
                pack->serialize(serialPacket);
        }
        else if(type == PACKET_EOT_CLI2SERV)
        {
            delete pack;
            pack = new packet(PACKET_EOT_SERV2CLI, packet_seqnum, 0, NULL);
            pack->serialize(serialPacket);
        }

        n = sendto(sendSock, serialPacket, strlen(serialPacket), 0, (struct sockaddr*)&send_cli_addr, sizeof(send_cli_addr));
        if(n < 0)
            error("Error sending packet");
        else
            cout << "Packet sent:" << endl;
        pack->printContents();

        bzero(serialPacket, sizeof(serialPacket));
        delete pack;
        pack = new packet(0,0,0,data);

    }while(type != PACKET_EOT_CLI2SERV);

    cout << "End of transmission" << endl;
    //cleanup
    fclose(filep);
    fclose(arrival_log);
    close(sendSock);
    close(recvSock);
    return 0; 
}
