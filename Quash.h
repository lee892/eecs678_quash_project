#ifndef QUASH

#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
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
    vector<string> parse_input(string input, string delimiter);
    void pipe_commands(string input);
    void io_redirects();
    void setup();
};

#endif