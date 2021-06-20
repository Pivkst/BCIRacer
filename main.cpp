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

static int SCREEN_WIDTH = 1280, SCREEN_HEIGHT = 720;
static int VANISH_Y = 200;
static int ZERO_Y = 620;
int mapXposition(int x, double scale){
    return SCREEN_WIDTH/2 + static_cast<int>(static_cast<double>(x - SCREEN_WIDTH/2) * scale);
}
int mapYposition(int y){
    return std::min(ZERO_Y - y * (ZERO_Y-VANISH_Y) / 10000, 10000);
}
double mapScale(int y){
    return std::max(1.0 - (static_cast<double>(y) / 10000), 0.0);
}

int main()
{
    //SDL setup
    std::string title = "BCIGAME";
    initLog();
    initEnvironment(title, SCREEN_WIDTH, SCREEN_HEIGHT, false);
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
    int carx = SCREEN_WIDTH/2;
    int cary = 0;

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
            cary-=100;
        else if(buttonState == RIGHT)
            cary+=100;

        //Render
        SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
        SDL_RenderClear(renderer);
        textures[TEXTURE_BACKGROUND]->render();
        double scale = mapScale(cary);
        textures[TEXTURE_CAR]->renderScaled(mapXposition(carx, scale), mapYposition(cary), scale);
        SDL_RenderPresent(renderer);
    }
    closeLog();
    //Force exit to stop blocked listener thread
    exit(1); //TODO close listener thread with a signal
}
