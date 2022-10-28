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
        trimString(input);
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
    string envVar = input.substr(start+1, end-start-1);
    const char* varStr = envVar.c_str();
    string env = getenv(varStr);
    input.replace(start, end-start, env);
}

void clean(string &input, string &fullInput, bool &isBackground) {
    trimString(input);
    parseComment(input);
    fullInput = input;
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

vector<string> parseSingleDelim(string input, string delimiter) {
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
        vector<string> params = parseSingleDelim(token, " ");
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
    int strsSize = strs.size();
    char** chars = new char*[strs.size() + 1];
    for (int i = 0; i < strsSize; i++) {
        chars[i] = new char[strs[i].length()+1];
        strcpy(chars[i], strs[i].c_str());
    }
    chars[strs.size()+1] = NULL;
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
        if (process.keyWord  == "echo") {
            string s = process.original.substr(5);
            trimString(s);
            s.erase(remove( s.begin(), s.end(), '\"' ), s.end());
            s.erase(remove( s.begin(), s.end(), '\'' ), s.end());
            cout << s << "\n";
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
            int numBackgroundJobs = backgroundJobs.size();
            for (int i = 0; i < numBackgroundJobs; i++){
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
    } else if (access(process.keyWord.c_str(), F_OK) != -1 && access(process.keyWord.c_str(), X_OK) == -1) {
        char catCommand[] = "cat";
        char* params[] = {catCommand, NULL};
        
        execvp("cat", params);
        
        return false;
    } else {
        //Separate command and params
        const char* charCommand = process.keyWord.c_str();
        std::vector<char*> charParam;
        // const_cast is needed because execvp prototype wants an
        // array of char*, not const char*.
        for (auto const& a : process.params)
            charParam.emplace_back(const_cast<char*>(a.c_str()));
        // NULL terminate
        charParam.push_back(nullptr);
        // The first argument to execvp should be the same as the
        // first element in charParam, but we'll assume the caller knew
        // what they were doing, and that program is a std::string. 
        execvp(charCommand, charParam.data());
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
            if (commands[i].delimiter == "<") {
                int fileDes = open(commands[i+1].keyWord.c_str(), O_RDONLY);
                dup2(fileDes, STDIN_FILENO);
                dup2(pipes[i][1], STDOUT_FILENO);

            } else if (commands[i].delimiter == ">") {
                int fileDes = open(commands[i+1].keyWord.c_str(), O_WRONLY | O_CREAT, 0644);
                dup2(pipes[i-1][0], STDIN_FILENO);
                dup2(fileDes, STDOUT_FILENO);
            } else if (commands[i].delimiter == ">>") {
                int fileDes = open(commands[i+1].keyWord.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
                dup2(pipes[i-1][0], STDIN_FILENO);
                dup2(fileDes, STDOUT_FILENO);
            } else {
                if (i > 0) {
                    //From pipes to stdin
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                if (i < numCommands-1) {
                    //From stdout to pipes
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
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

// void handle_sigchld(int sig) {
//   int saved_errno = errno;
//   while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
//   errno = saved_errno;
//   cout << getpid() << "\n";
// }

void Quash::run() {
    // struct sigaction sa;
    // sa.sa_handler = &handle_sigchld;
    // sigemptyset(&sa.sa_mask);
    // sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    // if (sigaction(SIGCHLD, &sa, 0) == -1) {
    //     perror(0);
    //     exit(1);
    // }
    string input;
    while (input != "exit" && input != "quit") {
        cout << "[Quash]$ ";
        input = takeInput();
        if (input.empty()) continue;
        //Save original input
        string fullInput = input;

        //Clean for &, $, and #
        bool isBackground = false;
        clean(input, fullInput, isBackground);
        if (input.length() == 0) continue;

        //Divide input to commands
        vector<string> delimiters {"", "|", ">>", ">", "<"};
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
        if (commands[0].keyWord == "export") {
            string param = commands[0].original;
            int idx = param.find("=");
            string key = param.substr(7, idx-7);
            string val = param.substr(idx+1);
            setenv(key.c_str(), val.c_str(), 1);
        }

        if(!isBackground) {
            int status;
            waitpid(pid, &status, 0);
        }
        int numBackgroundJobs = backgroundJobs.size();
        for (int i = 0; i < numBackgroundJobs; i++) {
            pid_t childPid = backgroundJobs[i].pid;
            if (waitpid(childPid, 0, WNOHANG) == childPid) {
                handleAtExit(backgroundJobs[i].pid, i);
            }
        }
    }
}

void Quash::handleAtExit(pid_t pid, int idx) {
    cout << "Completed: ";
    printf("[%d]    %d     ", idx, backgroundJobs[idx].pid);
    cout << backgroundJobs[idx].fullCommand << "\n";

    auto iter = std::find_if(backgroundJobs.begin(), backgroundJobs.end(),
                             [&](const Job& job){return job.pid == pid;});

    // if found, erase it
    if ( iter != backgroundJobs.end())
       backgroundJobs.erase(iter);
}
