#ifndef SETTINGS_H
#define SETTINGS_H
#include <string>
#include <windows.h>

void getSettingsFromArguments(int argc, char** argv, bool& alignment){
    if(argc > 1)
        if(std::string(argv[1]) == "-aligned")
            alignment = true;
}

std::string getSettingsString(LPCSTR name){
    char settingsString[100];
    GetPrivateProfileStringA("BCIGAME", name, "defaultString", settingsString, 100, ".//settings.ini");
    return std::string(settingsString);
}

bool getSettingsBool(LPCSTR name){
    int value = 0;
    return GetPrivateProfileIntA("BCIGAME", name, value, ".//settings.ini");
}

double getSettingsDouble(LPCSTR name){
    char settingsString[100];
    GetPrivateProfileStringA("BCIGAME", name, "12345", settingsString, 100, ".//settings.ini");
    return std::atof(settingsString);
}

#endif // SETTINGS_H
