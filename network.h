#ifndef NETWORK_H
#define NETWORK_H

#include <winsock2.h>
#include <logging.h>

#define BUFLEN 512
#define PORT 1234

struct socketData{
    SOCKET socket_in = INVALID_SOCKET, socket_out = INVALID_SOCKET;
    struct sockaddr_in server, si_other;
    int slen = 0, recv_len = 0;
};

int initSocket(socketData* mySocket){
    WSADATA wsa;
    mySocket->slen = sizeof (mySocket->si_other);
    //Initialise winsock
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        writeToLog("Failed. Error Code : "+std::to_string(WSAGetLastError()));
        exit(EXIT_FAILURE);
    }

    //Create the sockets
    if((mySocket->socket_in = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
    {
        writeToLog("Could not create socket : "+std::to_string(WSAGetLastError()));
    }
    if((mySocket->socket_out = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
    {
        writeToLog("Could not create socket : "+std::to_string(WSAGetLastError()));
    }

    //Prepare the sockaddr_in structure
    mySocket->server.sin_family = AF_INET;
    mySocket->server.sin_addr.s_addr = INADDR_ANY;
    mySocket->server.sin_port = htons( PORT );

    //Bind
    if( bind(mySocket->socket_in ,(struct sockaddr *)&mySocket->server , sizeof(mySocket->server)) == SOCKET_ERROR)
    {
        writeToLog("Bind failed with error code : "+std::to_string(WSAGetLastError()));
        exit(EXIT_FAILURE);
    }
    writeToLog("Sockets bound");
    return 0;
}

void closeSocket(socketData* s){
    closesocket(s->socket_in);
    closesocket(s->socket_out);
    WSACleanup();
}

void recieve(socketData* mySocket, char* output){
    //fflush(stdout);

    //clear the buffer by filling null, it might have previously received data
    memset(output,'\0', BUFLEN);

    //try to receive some data, this is a blocking call
    if ((mySocket->recv_len = recvfrom(mySocket->socket_in, output, BUFLEN, 0, (struct sockaddr *) &mySocket->si_other, &mySocket->slen)) == SOCKET_ERROR)
    {
        writeToLog("recvfrom() failed with error code : "+std::to_string(WSAGetLastError()));
        exit(EXIT_FAILURE);
    }

    //print details of the client/peer and the data received
    //printf("\nReceived packet from %s:%d\n", inet_ntoa(mySocket->si_other.sin_addr), ntohs(mySocket->si_other.sin_port));

    //now reply the client with the same data
    if (sendto(mySocket->socket_out, output, mySocket->recv_len, 0, (struct sockaddr*) &mySocket->si_other, mySocket->slen) == SOCKET_ERROR)
    {
        writeToLog("sendto() failed with error code : "+std::to_string(WSAGetLastError()));
        exit(EXIT_FAILURE);
    }
    return;
}

#endif // NETWORK_H
