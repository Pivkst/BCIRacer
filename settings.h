#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <windows.h>
#include "logging.h"
#include "graphics.h"
#include <queue>

void getSettingsFromArguments(int argc, char** argv, bool& alignment){
    if(argc > 1)
        if(std::string(argv[1]) == "-aligned")
            alignment = true;
}

void checkForError(){
    if(GetLastError() == 0x2){
        writeToLog("settings.ini not found or invalid");
        exit(EXIT_FAILURE);
    }
}

std::string getSettingsString(LPCSTR name){
    char settingsString[100];
    GetPrivateProfileStringA("BCIGAME", name, "defaultString", settingsString, 100, ".//settings.ini");
    checkForError();
    return std::string(settingsString);
}

bool getSettingsBool(LPCSTR name){
    bool defaultValue = false;
    bool value = GetPrivateProfileIntA("BCIGAME", name, defaultValue, ".//settings.ini");
    checkForError();
    return value;
}

double getSettingsDouble(LPCSTR name){
    char settingsString[100];
    GetPrivateProfileStringA("BCIGAME", name, "12345", settingsString, 100, ".//settings.ini");
    checkForError();
    return std::atof(settingsString);
}

UINT getSettingsInt(LPCSTR name){
    return GetPrivateProfileIntA("BCIGAME", name, 12345, ".//settings.ini");
}

bool loadCustomCarOrder(std::queue<SDL_Point>* order){
    static std::ifstream orderFile;
    std::string fileName = getSettingsString("custom order file");
    orderFile.open(fileName);
    if(orderFile.is_open()){
        std::string line;
        while(std::getline(orderFile, line)){
            auto commaPosition = line.find(",");
            int lane = stoi(line.substr(0, commaPosition));
            int delay = stoi(line.substr(commaPosition+1, line.size()));
            SDL_Point car = {lane, delay};
            order->push(car);
        }
        return true;
    } else {
        writeToLog("Can't open custom order file "+fileName);
        return false;
    }
}

#endif // SETTINGS_H
