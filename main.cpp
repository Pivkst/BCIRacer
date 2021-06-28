#include <stdio.h>
#include <network.h>
#include <thread>
#include <graphics.h>
#include <stdlib.h> //srand, rand

int main()
{
    //SDL setup
    std::string title = "BCIGAME";
    initLog();
    int SCREEN_WIDTH = 1280;
    int SCREEN_HEIGHT = 720;
    initEnvironment(title, SCREEN_WIDTH, SCREEN_HEIGHT, false);
    initTTF();
    loadTextures();
    //loadSounds();
    SDL_Event e;

    //Network setup
    int buttonState = NONE;
    std::thread listenerThread (listener, &buttonState);

    //Game setup
    bool running = true;
    bool paused = true;
    bool gameOver = false;
    int frame = 0;
    static int TOP_Y = 50000;
    static int BOTTOM_Y = -2000;
    static int CAR_WIDTH = 240; //This is slightly slimmer than the car texture on purpose
    SDL_Point playerCar = {SCREEN_WIDTH/2, 0};
    std::vector<SDL_Point> cars;
    SDL_Point fatalCar;
    srand(888888);
    std::string debugString = "";

    while(running){
        //Check for events
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type){
                case SDL_QUIT: running = false; break;
            }
        }
        if(buttonState == STOP) running = false;
        else if(buttonState == START) paused = false;

        if(!paused)
        {
            //Game
            if(buttonState == LEFT && playerCar.x >= CAR_WIDTH/2)
                playerCar.x-=10;
            else if(buttonState == RIGHT && playerCar.x <= SCREEN_WIDTH-CAR_WIDTH/2)
                playerCar.x+=10;
            if(rand()%50 == 0){
                SDL_Point newCar = {200 + rand()%(SCREEN_WIDTH-400), TOP_Y};
                cars.push_back(newCar);
            }
            for(auto i = cars.begin(); i<cars.end(); i++){
                i->y -= 50;
                //Check for collision
                if(i->y<0 && i->y>-100 && i->x>(playerCar.x-240) && i->x<(playerCar.x+240)){
                    running = false;
                    gameOver = true;
                    i->y += 50; //Adjust car position back to the player's front
                    fatalCar = *i;
                    break;
                }
            }
            //Clean up offscreen cars
            if(cars.size() > 100){
                auto firstVisibleCar = cars.begin();
                while(firstVisibleCar->y < BOTTOM_Y)
                    firstVisibleCar++;
                cars.erase(cars.begin(), firstVisibleCar-1);
            }
            debugString = std::to_string(cars.size());
            frame++;
        }
        drawGame(frame, playerCar, cars, debugString);
    }
    if(gameOver){
        drawGameOver(playerCar, fatalCar);
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
