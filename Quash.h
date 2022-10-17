#ifndef QUASH

#include <queue>
#include <string>
#include <unordered_map>
#include <iostream>
#include <stdlib.h>

class Quash {
    private:
    std::queue<std::string> input;
    std::unordered_map<std::string, std::string> commands;

    public:
    Quash();
    //~Quash();
    void run();
    std::string take_input();
    void setup();
    
};

#endif