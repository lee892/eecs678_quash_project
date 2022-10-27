#include "Quash.h"

Quash::Quash() {
    //char* arg_list[] = {"ls",  NULL};
    //execvp("ls", arg_list);
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
    if (start == string::npos) return;

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

size_t* findNextDelimiter(string input, vector<string> delimiters) {
    size_t* res = new size_t[2];
    res[0] = string::npos;
    res[1] = 0;

    size_t nextPos;
    int n = delimiters.size();
    for (int i = 1; i < n; i++) {
        nextPos = input.find(delimiters[i]);
        if (nextPos != string::npos && nextPos < res[0]) {
            res[0] = nextPos;
            res[1] = i;
        }
    }
    return res;
}

vector<string> parseParams(string input, string delimiter) {
    size_t pos;
    string token;
    vector<string> parsed;

    while ((pos = input.find(delimiter)) != string::npos) {
        token = input.substr(0, pos);
        parsed.push_back(token);
        input.erase(0, pos + delimiter.length());

    }
    parsed.push_back(input);
    return parsed;
}

vector<Process> parseInput(string input, vector<string> delimiters) {
    size_t* pos;
    int index;
    string token;
    vector<Process> parsed;

    while (!input.empty()) {
        pos = findNextDelimiter(input, delimiters);
        
        index = (pos[0] == string::npos) ? input.length() : pos[0];
        token = (pos[0] == string::npos) ? input : input.substr(0, index);

        trimString(token);
        //Separate by spaces
        vector<string> params = parseParams(token, " ");
        //The delimiter found
        string delimiter = delimiters[pos[1]];
        //Create Process struct
        Process currProcess;
        currProcess.delimiter = delimiter;
        currProcess.keyWord = params[0];
        currProcess.params = params;
        currProcess.original = token;

        parsed.push_back(currProcess);
        input.erase(0, index + delimiter.length());
        delete[] pos;
    }

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

bool Quash::executeCommand(Process process) {
    if (builtIns.find(process.keyWord) != builtIns.end()) {
        cout << "Running built in " << process.keyWord << "\n";
        if (process.keyWord  == "echo") {
            cout << process.original.substr(5) << "\n";
        }
        if (process.keyWord == "pwd") {
            char path[256];
            memset(path, 0, sizeof(path));
            if (getcwd(path, sizeof(path)) == NULL) {
                perror("getcwd");
                abort();
            }
            string s;
            for (const auto &piece : path) s += piece;
            cout << s << "\n";
        }
        if (process.keyWord == "jobs") {
            for (int i = 0; i < backgroundJobs.size(); i++){
                printf("[%d]    %d     ", i, backgroundJobs[i].pid);
                cout << backgroundJobs[i].fullCommand << "\n";
            }
        }
        if (process.keyWord == "exit" || process.keyWord == "quit") {
            exit(0);
        }
        if (process.keyWord == "kill") {
            // kill(pid_t, int sig)
            int sig = stoi(process.params[1]);
            pid_t pid = stoi(process.params[2]);
            kill(pid, sig);
        }
        return true;
    } else {
        //Separate command and params
        const char* charCommand = process.keyWord.c_str();
        char** params = stringsToChars(process.params);
            execvp(charCommand, params);

        //Delete params
        for (int i = 0; i < process.params.size(); i++) {
            delete params[i];
        }
        delete[] params;
        return false;
    }
}

void Quash::executeCommands() {
    int numCommands = commands.size();
    int status;
    //If only one command
    if (numCommands == 1) {
        pid_t pid = fork();

        if (pid == 0) {
            executeCommand(commands[0]);
            exit(0);
        }

        //Parent process wait
        waitpid(pid, &status, 0);

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

            //Execute
            executeCommand(commands[i]);
            
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
        //Save original input
        string fullInput = input;

        //Clean for &, $, and #
        bool isBackground = false;
        clean(input, isBackground);
        if (input.length() == 0) continue;

        //Divide input to commands
        vector<string> delimiters {"", "|", "<", ">", ">>"};
        commands = parseInput(input, delimiters);

        //Create process for job
        pid_t pid = fork();
        if (pid == 0) {
            //In child process
            executeCommands();
            if (isBackground) {
            } 
            exit(0); // must do something here
        } else {
            // In parent process
            if (isBackground) {
                //Add to background jobs
                Job childProcess;
                childProcess.pid = pid;
                childProcess.fullCommand = fullInput;
                cout << "pushing background process\n";
                backgroundJobs.push_back(childProcess);
                int idx = backgroundJobs.size() - 1;
                printf("Background job started: [%d]    %d     ", idx, backgroundJobs[idx].pid);
                cout << backgroundJobs[idx].fullCommand << "\n";
            }
            if (commands[0].keyWord == "cd") {
                // if empty, go home
                string path;
                if(commands[0].original.size() > 3) {
                        path = commands[0].original.substr(3);
                } else {
                    string home = "HOME";
                    string str(getenv(home.c_str()));
                    path = str;
                }
                string key = "PWD";
                // set working directory and $PWD
                chdir(path.c_str());
                setenv(key.c_str(), path.c_str(), 1);
            }
            if (commands[0].keyWord == "export") {
                string param = commands[0].original;
                int idx = param.find("=");
                string key = param.substr(7, idx-7);
                string val = param.substr(idx+1);
                setenv(key.c_str(), val.c_str(), 1);
            }
        }
        int status;
        waitpid(pid, &status, 0);
        handleAtExit(pid);
    }
}

void Quash::handleAtExit(pid_t pid) {
    cout << "exiting\n";
    auto iter = std::find_if(backgroundJobs.begin(), backgroundJobs.end(),
                             [&](const Job& job){return job.pid == pid;});

    // if found, erase it
    if ( iter != backgroundJobs.end())
       backgroundJobs.erase(iter);
}