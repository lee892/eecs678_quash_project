#include "Quash.h"

struct Process {
    char delimiter;
    string command;
    vector<string> params;
    bool builtIn;
};


Quash::Quash() {
    //char* arg_list[] = {"ls",  NULL};
    //execvp("ls", arg_list);
}

void echo() {

}

void Quash::setup() {
    

}


string takeInput() {
    string input;
    getline(cin, input);

    return input;
}

void parseComment(string &input) {
    size_t pos = input.find("#");
    input = input.substr(0, pos);
}

void trimString(string &str) {
    while (isspace(str.at(0))) {
        str = str.substr(1);
    }
    while (isspace(str.at(str.size()-1))) {
        str = str.substr(0, str.size()-1);
    }
}

bool parseAmpersand(string &input) {
    size_t pos = input.find("&");
    if (pos != string::npos) {
        input = input.substr(0, pos);
        return true;
    }
    return false;
}

void parseEnv(string &input) {
    /*size_t start;
    string temp = input;
    int prev = 0;
    while ((start = temp.find("$")) != string::npos) {
        int end = start;
        while (temp[end] >= 'A' && temp[end] <= 'Z') {
            end++;
        }
        int i = start + prev;
        int j = end + prev;
        string varStr = input.substr(i, j)
        string env = getenv(varStr.substr(1));
        input.replace(i, j, )
        prev = start;
    }*/
    size_t start = input.find("$");
    int end = start + 1;
    while (input[end] >= 'A' && input[end] <= 'Z') {
        end++;
    }
    const char* varStr = input.substr(start+1, end).c_str();
    string env = getenv(varStr);
    input.replace(start, end, env);
}

void clean(string &input, bool &isBackground) {
    trimString(input);
    parseComment(input);
    parseEnv(input);
    isBackground = parseAmpersand(input);

}

vector<string> parseInput(string input, string delimiter) {
    size_t pos = 0;
    string token;
    vector<string> parsed;

    while ((pos = input.find(delimiter)) != string::npos) {
        token = input.substr(0, pos);
        trimString(token);
        parsed.push_back(token);
        input.erase(0, pos + delimiter.length());
    }
    trimString(input);
    parsed.push_back(input);
    return parsed;
}

char** stringsToChars(vector<string> strs) {
    char** chars = new char*[strs.size()];
    for (int i = 0; i < strs.size(); i++) {
        chars[i] = new char[strs[i].length()+1];
        strcpy(chars[i], strs[i].c_str());
    }
    return chars;
}

void closePipes(int pipes[][2], int numPipes, int pipe) {
    for (int i = 0; i < numPipes; i++) {
        if (i == pipe-1) {
            close(pipes[i][1]);
        } else if (i == pipe) {
            close(pipes[i][0]);
        } else {
            close(pipes[i][1]);
            close(pipes[i][0]);
        }
    }
}

// includes command in parameters
void executeCommand(string command, vector<string> parameters, string stringParameter) {
    if (command == "echo") {
        std::cout << stringParameter.substr(4) << "\n";
    } else {
        const char* charCommand = parameters[0].c_str();
        char** params = stringsToChars(parameters);
        execvp(charCommand, params);
    }
}

void Quash::pipeCommands(string input) {
    commands = parseInput(input, "|");
    int numCommands = commands.size();
    int status;
    //If only one command
    if (numCommands == 1) {
        pid_t pid = fork();

        if (pid == 0) {
            //Split string on " "
            vector<string> p = parseInput(commands[0], " ");
            //Separate command and params
            const char* command = p[0].c_str();
            char** params = stringsToChars(p);

            executeCommand(p[0], p, commands[0]);
            //Delete params
            for (int i = 0; i < p.size(); i++) {
                delete params[i];
            }
            delete params;
            exit(0);
        } else {
            //Parent process wait
            waitpid(pid, &status, 0);
        }
        return;
    }
    //Piping multiple processes
    pid_t pids[numCommands];
    int pipes[numCommands-1][2];

    for (int i = 0; i < numCommands-1; i++) {
        pipe(pipes[i]);
    }

    for (int i = 0; i < numCommands; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            //Redirect IO
            if (i > 0) {
                //From pipes to stdin
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if (i < numCommands-1) {
                //From stdout to pipes
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            //Close other pipes
            closePipes(pipes, numCommands-1, i);

            //Split string on " ", call exec
            vector<string> p = parseInput(commands[i], " ");
            const char* command = p[0].c_str();
            char** params = stringsToChars(p);
            
            execvp(command, params);
            //Delete params
            for (int i = 0; i < p.size(); i++) {
                delete params[i];
            }
            delete params;
            exit(0);
        }
        
    }
    closePipes(pipes, numCommands-1, -1);
    for (int i = 0; i < numCommands; i++) {
        waitpid(pids[i], &status, 0);
    }
}

void Quash::redirectIO() {

}

void Quash::run() {
    string input;
    while (input != "exit" && input != "quit") {
        cout << "[Quash]$ ";
        input = takeInput();
        bool isBackground = false;
        clean(input, isBackground);
        if (input.length() == 0) continue;
        pipeCommands(input);
        
    }
}