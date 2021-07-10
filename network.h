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
    MOVETO,
    END,
    PAUSE,
    START
};

void createSDLEvent (int code, void* dataPointer=nullptr){
    SDL_Event sdlEvent;
    SDL_memset(&sdlEvent, 0, sizeof(sdlEvent));
    sdlEvent.type = CUSTOM_EVENT_TYPE;
    sdlEvent.user.code = code;
    sdlEvent.user.data1 = dataPointer;
    SDL_PushEvent(&sdlEvent);
}

void listener(int* state){
    char message[BUFLEN];
    socketData socket;
    initSocket(&socket);
    std::string messageString;
    while(true){
        recieve(&socket, message);
        messageString = std::string(message);
        if(messageString == "none")
            *state = NONE;
        else if(messageString == "left"){
            *state = LEFT;
            createSDLEvent(LEFT);
        }
        else if(messageString == "right"){
            *state = RIGHT;
            createSDLEvent(RIGHT);
        }
        else if(messageString.size()>6){
            unsigned long long delimiterPosition = messageString.find("-");
            std::string code = messageString.substr(0, delimiterPosition);
            if(code == "moveto"){
                std::string valueString = messageString.substr(delimiterPosition+1, messageString.size());
                int value = stoi(valueString);
                int* valuePointer = new int(value);
                createSDLEvent(MOVETO, valuePointer);
            }
        }
        else if(messageString == "end"){
            createSDLEvent(END);
        }
        else if(messageString == "pause"){
            createSDLEvent(PAUSE);
        }
        else if(messageString == "start"){
            createSDLEvent(START);
        }
    }
}

#endif // NETWORK_H
