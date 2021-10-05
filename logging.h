//Copyright 2021 Jan Tomšič Pivk

#ifndef LOGGING_H
#define LOGGING_H

#include <sstream>
#include <fstream>

static std::ofstream logFile;
bool initLog(){
    logFile.open ("log.txt");
    return logFile.is_open();
}
void closeLog(){
    logFile.close();
}

void writeToLog(std::string text){
    logFile << text << std::endl;
}

#endif // LOGGING_H
