#include <stdio.h>
#include <network.h>
#include <thread>
#include <graphics.h>
#include <math.h>
#include <vector>
#include <stdlib.h> //srand, rand

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
static int TOP_Y = 50000;
static int BOTTOM_Y = -2000;
static int CAR_WIDTH = 240; //This is slightly slimmer than the car texture on purpose
double reScaleY(int y){
    double newY = static_cast<double>(y)/1000;
    if(newY > 0)
        return atan(newY) * 2 / M_PI;
    else
        return newY;
}
int mapXposition(int x, double scale){
    return SCREEN_WIDTH/2 + static_cast<int>(static_cast<double>(x - SCREEN_WIDTH/2) * scale);
}
int mapYposition(int y){
    return ZERO_Y - static_cast<int>(reScaleY(y) * (ZERO_Y-VANISH_Y));
}
double mapScale(int y){
    return std::max(1.0 - (reScaleY(y)), 0.0);
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
    bool gameOver = false;
    double scale = 0;
    int carx = SCREEN_WIDTH/2;
    int cary = 0;
    int liney = 0;
    std::vector<SDL_Point> cars;
    SDL_Point fatalCar;
    srand(888888);

    while(running){
        //Check for events
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type){
                case SDL_QUIT: running = false; break;
            }
        }
        if(buttonState == STOP) running = false;

        //Game
        if(buttonState == LEFT && carx >= CAR_WIDTH/2)
            carx-=10;
        else if(buttonState == RIGHT && carx <= SCREEN_WIDTH-CAR_WIDTH/2)
            carx+=10;
        if(rand()%50 == 0){
            SDL_Point newCar = {200 + rand()%(SCREEN_WIDTH-400), TOP_Y};
            cars.push_back(newCar);
        }
        for(auto i = cars.begin(); i<cars.end(); i++){
            i->y -= 50;
            //Check for collision
            if(i->y<0 && i->y>-100 && i->x>(carx-240) && i->x<(carx+240)){
                running = false;
                gameOver = true;
                i->y += 50; //Adjust car position back to the player's front
                fatalCar = *i;
                break;
            }
        }
        //TODO clean up offscreen cars

        //Render
        SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
        SDL_RenderClear(renderer);
            //Draw background
        textures[TEXTURE_BACKGROUND]->render();
            //Draw road lines
        liney++;
        if(liney == 20)
            liney = 0;
            for(int y = -2000 -liney*16; y<40000; y+=static_cast<int>(1000/scale)){
                scale = mapScale(y);
                textures[TEXTURE_LINE]->renderScaled(mapXposition(640, scale), mapYposition(y), scale);
            }
            //Draw cars
        bool playerCarDrawn = false;
        int lastcary = 0;
        for(auto i = cars.rbegin(); i<cars.rend(); i++){
            if(i->y < 0 && playerCarDrawn == false){
                //Draw player car
                scale = mapScale(cary);
                textures[TEXTURE_CAR]->renderScaled(mapXposition(carx, scale), mapYposition(cary), scale);
                playerCarDrawn = true;
            }
            scale = mapScale(i->y);
            textures[TEXTURE_CAR]->renderScaled(mapXposition(i->x, scale), mapYposition(i->y), scale);
            lastcary = i->y;
        }
        if(!playerCarDrawn){
            //Draw player car if it hasn't been drawn yet
            scale = mapScale(cary);
            textures[TEXTURE_CAR]->renderScaled(mapXposition(carx, scale), mapYposition(cary), scale);
            playerCarDrawn = true;
        }


        drawText(10, 10, std::to_string(lastcary));
        SDL_RenderPresent(renderer);
    }
    if(gameOver){
        //Show game over screen
        SDL_SetRenderDrawColor(renderer, 0xcc, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);
        double scale = mapScale(fatalCar.y);
        textures[TEXTURE_CAR]->renderScaled(mapXposition(fatalCar.x, scale), mapYposition(fatalCar.y), scale);
        scale = mapScale(cary);
        textures[TEXTURE_CAR]->renderScaled(mapXposition(carx, scale), mapYposition(cary), scale);
        drawText(SCREEN_WIDTH/2, 200, "BAM", CONSOLA_BIG, BLACK, ALIGN_CENTER);
        SDL_RenderPresent(renderer);
        while(gameOver){
            //Check for events
            while(SDL_PollEvent(&e) != 0) {
                switch(e.type){
                    case SDL_QUIT: gameOver = false; break;
                }
            }
            if(buttonState == STOP) gameOver = false;
        }
    }
    closeLog();
    //Force exit to stop blocked listener thread
    exit(1); //TODO close listener thread with a signal
}
