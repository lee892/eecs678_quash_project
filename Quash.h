#ifndef QUASH

#include <queue>
#include <string>
#include <string.h>
#include <signal.h>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <algorithm>
//#include <boost/algorithm/string.hpp>

using namespace std;


struct Process {
    string delimiter;
    string keyWord;
    string original;
    vector<string> params;
    bool builtIn;
};

struct Job {
    string fullCommand;
    pid_t pid;
};

class Quash {
    private:
    vector<Process> commands;
    vector<Job> backgroundJobs;
    unordered_set<string> builtIns {"echo", "export", "cd", "pwd", "jobs", "kill"};
    void handleAtExit(pid_t pid, int idx);
    public:
    Quash();
    //~Quash();
    void run();
    bool executeCommand(Process process);
    void executeCommands();
    void redirectIO();
};

#endif