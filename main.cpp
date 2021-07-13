#include <network.h>
#include <thread>
#include <graphics.h>
#include <stdlib.h> //srand, rand
#include <settings.h>

template<class T>
T clamp(T value, T min, T max){
    value = value > max ? max : value;
    value = value < min ? min : value;
    return value;
}

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
    socketData socket;
    int buttonState = NONE;
    std::thread listenerThread (listener, &socket, &buttonState);

    //Game setup
    bool running = true;
    bool paused = getSettingsBool("start paused");
    bool gameOver = false;
    static int TOP_Y = 50000;
    static int BOTTOM_Y = -2000;
    static int CAR_WIDTH = 240; //This is slightly slimmer than the car texture on purpose
    bool aligned = getSettingsBool("aligned");
    int playerCarLane = clamp(static_cast<int>(getSettingsInt("start lane")-1), 0, 3); //Used in aligned mode
    SDL_Point playerCar = {SCREEN_WIDTH/8 + SCREEN_WIDTH/4*playerCarLane, 0};
    std::vector<SDL_Point> cars;
    SDL_Point fatalCar;
    srand(getSettingsInt("random seed"));
    std::string debugString = "";
    Uint32 spawnDelay = 200*15;
    bool noCrashMode = getSettingsBool("no crash");
    bool crashed = false;
    Uint32 timeDelta = 10;
    Uint32 lastFrameTime = 0;
    Uint32 lastSpawnTime = 0;
    Uint32 frameRateCap = getSettingsInt("framerate cap");
    Uint32 frameTimeCap = 1000/frameRateCap;
    double speedFactor = getSettingsDouble("speed factor");
    std::queue<SDL_Point> carOrder;
    bool usingCustomCarOrder = getSettingsBool("use custom car order");

    getSettingsFromArguments(argc, argv, aligned);

    if(usingCustomCarOrder)
        usingCustomCarOrder = loadCustomCarOrder(&carOrder);
    debugString = std::to_string(carOrder.size());

    while(running){
        //Check for events
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type){
                case SDL_QUIT: running = false; break;
            }
            //Check for custom events
            if(e.type == CUSTOM_EVENT_TYPE){
                switch (e.user.code) {
                case START:
                    paused = false;
                    send(&socket, "started");
                    break;
                case PAUSE:
                    paused = true;
                    send(&socket, "paused");
                    break;
                case END: running = false; break;
                case LEFT:
                    if(!paused && aligned){
                        playerCarLane--;
                        playerCarLane = clamp(playerCarLane, 0, 3);
                        send(&socket, "atlane-"+std::to_string(playerCarLane+1));
                    }
                    break;
                case RIGHT:
                    if(!paused && aligned){
                        playerCarLane++;
                        playerCarLane = clamp(playerCarLane, 0, 3);
                        send(&socket, "atlane-"+std::to_string(playerCarLane+1));
                    }
                    break;
                case MOVETO:
                    int* valuePointer = reinterpret_cast<int*>(e.user.data1);
                    if(aligned){
                        playerCarLane = (*valuePointer)-1;
                        playerCarLane = clamp(playerCarLane, 0, 3);
                        send(&socket, "atlane-"+std::to_string(playerCarLane+1));
                    } else {
                        playerCar.x = CAR_WIDTH/2 + clamp(*valuePointer, 0, 100) * (SCREEN_WIDTH-CAR_WIDTH)/100;
                    }
                    delete valuePointer;
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
                if(usingCustomCarOrder && !carOrder.empty()){
                    unsigned int customDelay = static_cast<unsigned int>(carOrder.front().y);
                    while(SDL_GetTicks() - lastSpawnTime >= customDelay && !carOrder.empty()){
                        SDL_Point newCar;
                        if(aligned){
                            int lane = carOrder.front().x-1;
                            newCar = {SCREEN_WIDTH/8 + lane*SCREEN_WIDTH/4, TOP_Y};
                            send(&socket, "spawnatlane-"+std::to_string(lane+1));
                        }
                        else{
                            newCar = {CAR_WIDTH/2 + clamp(carOrder.front().x, 0, 100)*(SCREEN_WIDTH-CAR_WIDTH)/100, TOP_Y};
                            send(&socket, "spawnat-"+std::to_string(newCar.x));
                        }
                        carOrder.pop();
                        cars.push_back(newCar);
                        lastSpawnTime = SDL_GetTicks();
                        customDelay = static_cast<unsigned int>(carOrder.front().y);
                    }
                }
                else
                    if(SDL_GetTicks() - lastSpawnTime >= spawnDelay){
                        SDL_Point newCar;
                        if(aligned){
                            int lane = rand()%4;
                            newCar = {SCREEN_WIDTH/8 + lane*SCREEN_WIDTH/4, TOP_Y};
                            send(&socket, "spawnatlane-"+std::to_string(lane+1));
                        }
                        else{
                            newCar = {200 + rand()%(SCREEN_WIDTH-400), TOP_Y};
                            send(&socket, "spawnat-"+std::to_string(newCar.x));
                        }
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
            bool collision = false;
            for(auto i = cars.begin(); i<cars.end(); i++){
                if(i->y<200 && i->y>-50 && i->x>(playerCar.x-240) && i->x<(playerCar.x+240)){
                    fatalCar = *i;
                    collision = true;
                }
            }
            if(!crashed && collision) // ensure the code is only sent once per crash
                send(&socket, "crash");
            crashed = collision;
            if(!noCrashMode && crashed){
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
        int FPS;
        if(timeDelta>0)
            FPS = 1000/(timeDelta);
        else
            FPS = 1000;
        //debugString = "FPS:" + std::to_string(FPS);
        lastFrameTime = frameTime;
        //Draw everything
        drawGame(speedFactor, playerCar, cars, debugString);
    }
    if(gameOver){
        drawGameOver(playerCar, fatalCar);
        send(&socket, "gameover");
        while(gameOver){
            //Check for events
            while(SDL_PollEvent(&e) != 0) {
                if(e.type == SDL_QUIT or (e.type == CUSTOM_EVENT_TYPE and e.user.code == END)){
                    gameOver = false;
                }
            }
            if(buttonState == END) gameOver = false;
        }
    }
    send(&socket, "end");
    closeLog();
    //Force exit to stop blocked listener thread
    exit(EXIT_SUCCESS); //TODO close listener thread with a signal
}
