#include <stdio.h>
#include <network.h>
#include <thread>
#include <graphics.h>
#include <stdlib.h> //srand, rand

int main(int argc, char** argv)
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
    bool aligned = false;
    if(argc > 1)
        if(std::string(argv[1]) == "aligned")
            aligned = true;
    bool running = true;
    bool paused = true;
    bool gameOver = false;
    int frame = 0;
    static int TOP_Y = 50000;
    static int BOTTOM_Y = -2000;
    static int CAR_WIDTH = 240; //This is slightly slimmer than the car texture on purpose
    int playerCarLane = 2; //Used in aligned mode
    SDL_Point playerCar = {SCREEN_WIDTH/8 + SCREEN_WIDTH/4*playerCarLane, 0};
    std::vector<SDL_Point> cars;
    SDL_Point fatalCar;
    srand(888888);
    std::string debugString = "";
    int lastSpawnFrame = 0;
    int spawnDelay = 200;

    while(running){
        //Check for events
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type){
                case SDL_QUIT: running = false; break;
            }
            //Check for custom events
            if(e.type == CUSTOM_EVENT_TYPE){
                switch (e.user.code) {
                case START: paused = false; break;
                case STOP: running = false; break;
                case LEFT:
                    if(playerCarLane>0)
                        playerCarLane--;
                    break;
                case RIGHT:
                    if(playerCarLane<3)
                        playerCarLane++;
                    break;
                }
            }
        }

        if(!paused)
        {
            //Move player car
            if(aligned){
                int targetX = SCREEN_WIDTH/8 + playerCarLane*SCREEN_WIDTH/4;
                if(targetX > playerCar.x+20)
                    playerCar.x += 20;
                else if(targetX < playerCar.x-20)
                    playerCar.x -= 20;
            }
            else{
                if(buttonState == LEFT && playerCar.x >= CAR_WIDTH/2)
                    playerCar.x-=10;
                else if(buttonState == RIGHT && playerCar.x <= SCREEN_WIDTH-CAR_WIDTH/2)
                    playerCar.x+=10;
            }
            //Place new cars
            if(frame-lastSpawnFrame >= spawnDelay /*rand()%50 == 0*/){
                SDL_Point newCar;
                if(aligned)
                    newCar = {SCREEN_WIDTH/8 + (rand()%4)*SCREEN_WIDTH/4, TOP_Y};
                else
                    newCar = {200 + rand()%(SCREEN_WIDTH-400), TOP_Y};
                cars.push_back(newCar);
                lastSpawnFrame = frame;
                if(spawnDelay > 100)
                    spawnDelay -= 5;
                else if(spawnDelay > 50)
                    spawnDelay -= 2;
            }
            //Move cars
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
            frame++;
        }
        if(argc>1)debugString = std::string(SDL_GetPixelFormatName(SDL_GetWindowPixelFormat(window)));
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
