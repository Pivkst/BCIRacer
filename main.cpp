#include <stdio.h>
#include <network.h>
#include <thread>
#include <graphics.h>
#include <stdlib.h> //srand, rand
#include <settings.h>

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
    bool running = true;
    bool paused = getSettingsBool("start paused");
    bool gameOver = false;
    static int TOP_Y = 50000;
    static int BOTTOM_Y = -2000;
    static int CAR_WIDTH = 240; //This is slightly slimmer than the car texture on purpose
    bool aligned = getSettingsBool("aligned");
    int playerCarLane = static_cast<int>(getSettingsInt("start lane")-1); //Used in aligned mode
    if(playerCarLane<0) playerCarLane=0;
    if(playerCarLane>3) playerCarLane=3;
    SDL_Point playerCar = {SCREEN_WIDTH/8 + SCREEN_WIDTH/4*playerCarLane, 0};
    std::vector<SDL_Point> cars;
    SDL_Point fatalCar;
    srand(getSettingsInt("random seed"));
    std::string debugString = "";
    Uint32 spawnDelay = 200*15;
    bool noCrash = getSettingsBool("no crash");
    bool crashed = false;
    Uint32 timeDelta = 10;
    Uint32 lastFrameTime = 0;
    Uint32 lastSpawnTime = 0;
    Uint32 frameRateCap = getSettingsInt("framerate cap");
    Uint32 frameTimeCap = 1000/frameRateCap;
    double speedFactor = getSettingsDouble("speed factor");

    getSettingsFromArguments(argc, argv, aligned);

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

        //Move player car
        double moveFactor = speedFactor*timeDelta;
        if(!paused)
        {
            if(aligned){
                int targetX = SCREEN_WIDTH/8 + playerCarLane*SCREEN_WIDTH/4;
                if(targetX > playerCar.x+20)
                    playerCar.x += static_cast<int>(1.5*moveFactor);
                else if(targetX < playerCar.x-20)
                    playerCar.x -= static_cast<int>(1.5*moveFactor);
            }
            else{
                if(buttonState == LEFT && playerCar.x >= CAR_WIDTH/2)
                    playerCar.x-=static_cast<int>(0.75*moveFactor);
                else if(buttonState == RIGHT && playerCar.x <= SCREEN_WIDTH-CAR_WIDTH/2)
                    playerCar.x+=static_cast<int>(0.75*moveFactor);
            }
            //Place new cars
            if(!crashed){
                if(SDL_GetTicks() - lastSpawnTime >= spawnDelay /*rand()%50 == 0*/){
                    SDL_Point newCar;
                    if(aligned)
                        newCar = {SCREEN_WIDTH/8 + (rand()%4)*SCREEN_WIDTH/4, TOP_Y};
                    else
                        newCar = {200 + rand()%(SCREEN_WIDTH-400), TOP_Y};
                    cars.push_back(newCar);
                    lastSpawnTime = SDL_GetTicks();
                    if(spawnDelay > 1500)
                        spawnDelay -= 80;
                    else if(spawnDelay > 500)
                        spawnDelay -= 30;
                }
            }
            else{
                lastSpawnTime += timeDelta;
            }
            //Move cars
            if(!crashed){
                for(auto i = cars.begin(); i<cars.end(); i++){
                    if(!crashed) i->y -= static_cast<int>(5*moveFactor);
                }
            }
            //Check for collision
            crashed = false;
            for(auto i = cars.begin(); i<cars.end(); i++){
                if(i->y<200 && i->y>-50 && i->x>(playerCar.x-240) && i->x<(playerCar.x+240)){
                    fatalCar = *i;
                    crashed = true;
                }
            }
            if(!noCrash && crashed){
                running = false;
                gameOver = true;
            }
            //Clean up offscreen cars
            if(cars.size() > 100){
                auto firstVisibleCar = cars.begin();
                while(firstVisibleCar->y < BOTTOM_Y)
                    firstVisibleCar++;
                cars.erase(cars.begin(), firstVisibleCar-1);
            }
        }
        //Wait to cap FPS
        Uint32 frameTime = SDL_GetTicks();
        timeDelta = frameTime - lastFrameTime;
        if(timeDelta < frameTimeCap) SDL_Delay(frameTimeCap - timeDelta);
        //Calculate time delta and FPS
        frameTime = SDL_GetTicks();
        timeDelta = frameTime - lastFrameTime;
        if(timeDelta>0)
            debugString = "FPS:" + std::to_string(1000/(timeDelta));
        lastFrameTime = frameTime;
        //Draw everything
        drawGame(speedFactor, playerCar, cars, debugString);
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
