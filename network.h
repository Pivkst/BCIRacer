#ifndef NETWORK_H
#define NETWORK_H
#define SDL_MAIN_HANDLED

#include <winsock2.h>
#include <logging.h>
#include <SDL.h>

#define BUFLEN 512
#define PORT 1234

static const Uint32 CUSTOM_EVENT_TYPE = SDL_RegisterEvents(1);

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

    //Check if the custom SDL event was registered
    if(CUSTOM_EVENT_TYPE == (Uint32)-1){
        writeToLog("Failed to register custom SDL event");
        exit(EXIT_FAILURE);
    }
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

enum buttonStates{
    NONE,
    LEFT,
    RIGHT,
    BOTH,
    STOP,
    START
};

void listener(int* state){
    char message[BUFLEN];
    const char* noneCode = "none";
    const char* leftCode = "left";
    const char* rightCode = "right";
    const char* bothCode = "both";
    const char* exitCode = "end";
    const char* startCode = "start";
    socketData socket;
    initSocket(&socket);
    printf("Waiting for data...\n");
    while(true){
        recieve(&socket, message);
        if(strcmp(message, noneCode) == 0)
            *state = NONE;
        if(strcmp(message, leftCode) == 0){
            *state = LEFT;
            SDL_Event sdlEvent;
            SDL_memset(&sdlEvent, 0, sizeof(sdlEvent));
            sdlEvent.type = CUSTOM_EVENT_TYPE;
            sdlEvent.user.code = LEFT;
            SDL_PushEvent(&sdlEvent);
        }
        if(strcmp(message, rightCode) == 0){
            *state = RIGHT;
            SDL_Event sdlEvent;
            SDL_memset(&sdlEvent, 0, sizeof(sdlEvent));
            sdlEvent.type = CUSTOM_EVENT_TYPE;
            sdlEvent.user.code = RIGHT;
            SDL_PushEvent(&sdlEvent);
        }
        if(strcmp(message, bothCode) == 0)
            *state = BOTH;
        if(strcmp(message, exitCode) == 0){
            *state = STOP;
            SDL_Event sdlEvent;
            SDL_memset(&sdlEvent, 0, sizeof(sdlEvent));
            sdlEvent.type = CUSTOM_EVENT_TYPE;
            sdlEvent.user.code = STOP;
            SDL_PushEvent(&sdlEvent);
        }
        if(strcmp(message, startCode) == 0){
            *state = START;
            SDL_Event sdlEvent;
            SDL_memset(&sdlEvent, 0, sizeof(sdlEvent));
            sdlEvent.type = CUSTOM_EVENT_TYPE;
            sdlEvent.user.code = START;
            SDL_PushEvent(&sdlEvent);
        }
        //printf(" recieved: %s\n" , message);
        //fflush(stdout);
    }
}

#endif // NETWORK_H
