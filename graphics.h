#ifndef GRAPHICS_H
#define GRAPHICS_H

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>
#include <logging.h>
#include <vector>
#include <math.h>

void logError(){
    std::string error = SDL_GetError();
    //You don't need to call TTF_GetError() etc. because they're defined as SDL_GetError() anyway
    if(error == ""){
        error = "Unknown error";
    }
    logFile << "Error: " << error << std::endl;
}

//Graphics environment
static SDL_Window* window = nullptr;
static SDL_Surface* windowSurface = nullptr;
static SDL_Renderer* renderer = nullptr;
static std::string basePath;
bool initEnvironment(std::string windowTitle, int w, int h, bool fullscreen = false){
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ){
        logError();
        return false;
    }
    basePath = std::string(SDL_GetBasePath());

    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags)){
        logError();
        return false;
    }

    if( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0 ){
        logError();
        return false;
    }

    writeToLog("SDL initialized");

    Uint32 flags = SDL_WINDOW_SHOWN;
    if(fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags);
    if(window == nullptr){
        logError();
        return false;
    }
    windowSurface = SDL_GetWindowSurface(window); //Don't need to free this
    writeToLog("Window initialized");

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr){
        logError();
        return false;
    }
    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0xff, 0xff);
    writeToLog("Renderer initialized");

    return true;
}

void toggleFullscreen(){
    if(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
        SDL_SetWindowFullscreen(window, 0);
    else
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    windowSurface = SDL_GetWindowSurface(window);
}

SDL_Surface* loadOptimizedSurface(std::string path){
    SDL_Surface* surface = IMG_Load((basePath+path).c_str());
    if(surface == nullptr){
        printf("Unable to load image: %s\n", SDL_GetError());
        return nullptr;
    }
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* optimizedSurface = SDL_ConvertSurface(surface, format, 0);
    //SDL_SetColorKey(optimizedSurface, SDL_TRUE, SDL_MapRGB(optimizedSurface->format, 0, 0xFF, 0xFF));
    SDL_FreeSurface(surface);
    return optimizedSurface;
}

class LTexture {
public:
    int w, h, clipw, cliph; //Image and clip dimensions
    LTexture();
    ~LTexture();
    bool loadFromFile(std::string);
    bool loadFromRenderedText(std::string, TTF_Font*, SDL_Color);
    void setClipSize(int, int);
    int getClipAmount(); //Return number of clips based on clip size
    void setColor(Uint8, Uint8, Uint8); //Set color modulation
    void setAlpha(Uint8); //Set transparency modulation
    void render(int, int, double); //Renders texture at given point
    void renderScaled(int, int, double);
    void render(int, int, int, double); //Renders texture clip n at given point
    void free(); //Deallocates texture
private:
    //The actual hardware texture
    SDL_Texture* hardwareTexture;
};
LTexture::LTexture(){
    hardwareTexture = nullptr;
    w = 0;
    h = 0;
}
LTexture::~LTexture(){
    free();
}
bool LTexture::loadFromFile(std::string path){
    free();
    SDL_Surface* surface = loadOptimizedSurface(path);
    if(surface == nullptr){
        logError();
        return false;
    }
    hardwareTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if(hardwareTexture == nullptr) logError();
    w = surface->w;
    h = surface->h;
    clipw = w;
    cliph = h;
    SDL_FreeSurface(surface);
    return true;
}
bool LTexture::loadFromRenderedText(std::string text, TTF_Font* font, SDL_Color color){
    free();
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if(surface == nullptr){
        logError();
        return false;
    }
    hardwareTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if(hardwareTexture == nullptr){
        logError();
        return false;
    }
    w = surface->w;
    h = surface->h;
    clipw = w;
    cliph = h;
    SDL_FreeSurface(surface);
    return true;
}
void LTexture::setClipSize(int w, int h){
    clipw = w;
    cliph = h;
}
int LTexture::getClipAmount(){
    return (w / clipw) * (h / cliph);
}
void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue){
    SDL_SetTextureColorMod(hardwareTexture, red, green, blue);
}
void LTexture::setAlpha(Uint8 alpha){
    SDL_SetTextureAlphaMod(hardwareTexture, alpha);
}
void LTexture::free(){
    if(hardwareTexture != nullptr){
        SDL_DestroyTexture(hardwareTexture);
        hardwareTexture = nullptr;
        w = 0;
        h = 0;
    }
}
void LTexture::render(int x=0, int y=0, double angle=0){
    SDL_Rect renderQuad = {x, y, w, h};
    SDL_RenderCopyEx(renderer, hardwareTexture, nullptr, &renderQuad, angle*180/M_PI, nullptr, SDL_FLIP_NONE);
}
void LTexture::renderScaled(int x, int y, double scale)
{
    int scaledWidth = static_cast<int>(w * scale);
    int scaledHeight = static_cast<int>(h * scale);
    SDL_Rect scaleRect = {
        x - scaledWidth/2,
        y - scaledHeight,
        scaledWidth,
        scaledHeight};
    SDL_RenderCopyEx(renderer, hardwareTexture, nullptr, &scaleRect, 0, nullptr, SDL_FLIP_NONE);
}
void LTexture::render(int x, int y, int n, double angle=0){
    SDL_Rect targetQuad = {x, y, clipw, cliph};
    //This needs to find the top-left pixel of the frame of animation being drawn
    int framesPerRow = (w / clipw); //First it calculates how many frames are in each row of the sprite sheet
    if(framesPerRow == 0) framesPerRow = 1; //If the result is 0, it increases it to 1 to avoid a division by zero
    SDL_Rect clipQuad = {
        (n % framesPerRow) * clipw,
        (n / framesPerRow) * cliph,
        clipw, cliph
    };
    SDL_RenderCopyEx(renderer, hardwareTexture, &clipQuad, &targetQuad, angle*180/M_PI, nullptr, SDL_FLIP_NONE);
}

//True Type Fonts
const int NUMBER_OF_FONTS = 2;
const int NUMBER_OF_COLORS = 7;
static TTF_Font* fonts[NUMBER_OF_FONTS];
enum fonts{
    CONSOLA, CONSOLA_BIG
};
static SDL_Color colors[NUMBER_OF_COLORS];
enum colors{
    BLACK,
    WHITE,
    RED,
    GREEN,
    BLUE,
    GRAY,
    LIGHTGRAY
};
bool initTTF(){
    if(TTF_Init() == -1){
        logError();
        return false;
    }
    colors[BLACK]= {0, 0, 0, 255};
    colors[WHITE] = {255, 255, 255, 255};
    colors[RED] = {255, 0, 0, 255};
    colors[GREEN] = {0, 255, 0, 255};
    colors[BLUE] = {0, 0, 255, 255};
    colors[GRAY] = {100, 100, 100, 255};
    colors[LIGHTGRAY] = {200, 200, 200, 255};
    fonts[CONSOLA] = TTF_OpenFont((basePath+"fonts\\consola.ttf").c_str(), 28);
    fonts[CONSOLA_BIG] = TTF_OpenFont((basePath+"fonts\\consola.ttf").c_str(), 200);
    if(fonts[CONSOLA] == nullptr){
        logError();
        return false;
    }
    writeToLog("Fonts loaded");
    return true;
}
enum alignments{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};
static LTexture generatedText;

void drawText(int x, int y, std::string text, int fontID = CONSOLA, int colorID=GREEN, int alignment=ALIGN_LEFT){
    generatedText.loadFromRenderedText(text, fonts[fontID], colors[colorID]);
    switch (alignment) {
    case ALIGN_LEFT:
        generatedText.render(x, y);
        break;
    case ALIGN_RIGHT:
        generatedText.render(x - generatedText.w, y);
        break;
    case ALIGN_CENTER:
        generatedText.render(x - generatedText.w/2, y - generatedText.h/2);
        break;
    }
};

//Load textures
static std::vector<LTexture*> textures; //Remember to "delete" textures before removing them from this vector
enum textureNames{
    TEXTURE_LINE1,
    TEXTURE_LINE2,
    TEXTURE_LINE3,
    TEXTURE_BACKGROUND,
    TEXTURE_CAR
};

bool loadTextures(){
    std::string texturePathsPath = basePath+"texturePaths.txt";
    std::ifstream file(texturePathsPath);
    if(!file.is_open()){
        writeToLog("Couldn't open "+texturePathsPath);
        return false;
    }
    bool success = true;
    std::string line;
    while(std::getline(file, line)){
        LTexture* texture = new LTexture;
        if(!texture->loadFromFile(line)){
            texture->loadFromRenderedText("ERROR", fonts[CONSOLA], colors[RED]);
            success = false;
        }
        textures.push_back(texture);
    }
    if(success) writeToLog(std::to_string(textures.size()) + " textures loaded");
    else writeToLog("Textures not loaded properly");
    return success;
}

double reScaleY(int y){
    double newY = static_cast<double>(y)/1000;
    if(newY > 0)
        return atan(newY) * 2 / M_PI;
    else
        return newY;
}
int mapXposition(int x, double scale){
    return windowSurface->w/2 + static_cast<int>(static_cast<double>(x - windowSurface->w/2) * scale);
}
int mapYposition(int y){
    int ZERO_Y = windowSurface->h - 100;
    int VANISH_Y = 200;
    return ZERO_Y - static_cast<int>(reScaleY(y) * (ZERO_Y-VANISH_Y));
}
double mapScale(int y){
    return std::max(1.0 - (reScaleY(y)), 0.0);
}

void drawGame(double speed, SDL_Point playerCar, std::vector<SDL_Point> cars, std::string debug = ""){
    int lineRate = static_cast<int>(speed*7);
    int liney = static_cast<int>(SDL_GetTicks())%(1000/(lineRate));
    double scale = 0;
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    SDL_RenderClear(renderer);
    //Draw background
    textures[TEXTURE_BACKGROUND]->render();
    //Draw road lines
    for(int y = 0 -liney*lineRate; y<15000; y+=1000){
        scale = mapScale(y);
        textures[TEXTURE_LINE1]->renderScaled(mapXposition(1*windowSurface->w/4 +20, scale), mapYposition(y), scale*0.5);
        textures[TEXTURE_LINE2]->renderScaled(mapXposition(2*windowSurface->w/4    , scale), mapYposition(y), scale*0.5);
        textures[TEXTURE_LINE3]->renderScaled(mapXposition(3*windowSurface->w/4 -20, scale), mapYposition(y), scale*0.5);
    }

    //Draw cars
    bool playerCarDrawn = false;
    for(auto i = cars.rbegin(); i<cars.rend(); i++){
        if(i->y < 0 && playerCarDrawn == false){
            //Draw player car
            scale = mapScale(playerCar.y);
            textures[TEXTURE_CAR]->renderScaled(mapXposition(playerCar.x, scale), mapYposition(playerCar.y), scale);
            playerCarDrawn = true;
        }
        scale = mapScale(i->y);
        textures[TEXTURE_CAR]->renderScaled(mapXposition(i->x, scale), mapYposition(i->y), scale);
    }
    //Draw player car if it hasn't been drawn yet
    if(!playerCarDrawn){
        scale = mapScale(playerCar.y);
        textures[TEXTURE_CAR]->renderScaled(mapXposition(playerCar.x, scale), mapYposition(playerCar.y), scale);
        playerCarDrawn = true;
    }
    if(debug != "")
        drawText(10, 10, debug);
    SDL_RenderPresent(renderer);
}

void drawGameOver(SDL_Point playerCar, SDL_Point fatalCar){
    SDL_SetRenderDrawColor(renderer, 0xcc, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);
    double scale = mapScale(fatalCar.y);
    textures[TEXTURE_CAR]->renderScaled(mapXposition(fatalCar.x, scale), mapYposition(fatalCar.y), scale);
    scale = mapScale(playerCar.y);
    textures[TEXTURE_CAR]->renderScaled(mapXposition(playerCar.x, scale), mapYposition(playerCar.y), scale);
    drawText(windowSurface->w/2, 200, "BAM", CONSOLA_BIG, BLACK, ALIGN_CENTER);
    SDL_RenderPresent(renderer);
}

#endif // GRAPHICS_H
