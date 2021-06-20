#include <stdio.h>
#include <network.h>
#include <thread>
#include <graphics.h>

enum buttonStates{
    NONE,
    LEFT,
    RIGHT,
    BOTH,
    STOP
};

void listener(int* state){
    char message[BUFLEN];
    const char* noneCode = "none";
    const char* leftCode = "left";
    const char* rightCode = "right";
    const char* bothCode = "both";
    const char* exitCode = "end";
    socketData socket;
    initSocket(&socket);
    printf("Waiting for data...\n");
    while(true){
        recieve(&socket, message);
        if(strcmp(message, noneCode) == 0)
            *state = NONE;
        if(strcmp(message, leftCode) == 0)
            *state = LEFT;
        if(strcmp(message, rightCode) == 0)
            *state = RIGHT;
        if(strcmp(message, bothCode) == 0)
            *state = BOTH;
        if(strcmp(message, exitCode) == 0)
            *state = STOP;
        //printf(" recieved: %s\n" , message);
        //fflush(stdout);
    }
}

int main()
{
    //SDL setup
    std::string title = "BCIGAME";
    initLog();
    int w = 1280, h = 720;
    initEnvironment(title, w, h, false);
    initTTF();
    loadTextures();
    //loadSounds();
    SDL_Event e;

    //Network setup
    int buttonState = NONE;
    std::thread listenerThread (listener, &buttonState);
    //listener(buttonState);

    //Game setup
    bool running = true;
    int carx = w/2;

    while(running){
        //Check for events
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type){
                case SDL_QUIT: running = false; break;
            }
        }
        if(buttonState == STOP) running = false;

        //Game
        if(buttonState == LEFT)
            carx-=10;
        else if(buttonState == RIGHT)
            carx+=10;

        //Render
        SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
        SDL_RenderClear(renderer);
        textures[TEXTURE_CAR]->renderScaled(carx, h -100, 1.0);
        SDL_RenderPresent(renderer);
    }
    closeLog();
    //Force exit to stop blocked listener thread
    exit(1); //TODO close listener thread with a signal
}
