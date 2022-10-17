#ifndef QUASH

#include <queue>
#include <unordered_map>
#include <iostream>
using namespace std;

class Quash {
    private:
    queue<string> input;
    //unordered_map commands;

    public:
    Quash();
    //~Quash();
    void run();
    string take_input();
    
};

#endif