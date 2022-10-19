#ifndef QUASH

#include <queue>
#include <string>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
//#include <boost/algorithm/string.hpp>

using namespace std;

class Quash {
    private:
    queue<string> inputs;
    vector<string> commands;
    

    public:
    Quash();
    //~Quash();
    void run();
    string take_input();
    void pipe_commands(string input);
    void io_redirects();
    void setup();
};

#endif