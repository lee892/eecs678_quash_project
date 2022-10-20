#include "Quash.h"


Quash::Quash() {
    //char* arg_list[] = {"ls",  NULL};
    //execvp("ls", arg_list);
}

void echo() {

}

void Quash::setup() {
    

}


string Quash::take_input() {
    string input;
    getline(cin, input);

    return input;
}

string trim_string(string str) {
    while (isspace(str.at(0))) {
        str = str.substr(1);
    }
    while (isspace(str.at(str.size()-1))) {
        str = str.substr(0, str.size()-1);
    }
    return str;
}

vector<string> parse_input(string input, string delimiter) {
    size_t pos = 0;
    string token;
    vector<string> parsed;

    while ((pos = input.find(delimiter)) != string::npos) {
        token = input.substr(0, pos);
        token = trim_string(token);
        parsed.push_back(token);
        input.erase(0, pos + delimiter.length());
    }
    input = trim_string(input);
    parsed.push_back(input);
    return parsed;
}

char** strings_to_chars(vector<string> strs) {
    char** chars = new char*[strs.size()];
    for (int i = 0; i < strs.size(); i++) {
        chars[i] = new char[strs[i].length()+1];
        strcpy(chars[i], strs[i].c_str());
    }
    return chars;
}

void close_pipes(int pipes[][2], int numPipes, int pipe) {
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

void Quash::pipe_commands(string input) {
    commands = parse_input(input, "|");
    int numCommands = commands.size();
    int status;
    //If only one command
    if (numCommands == 1) {
        pid_t pid = fork();

        if (pid == 0) {
            //Split string on " "
            vector<string> p = parse_input(commands[0], " ");
            //Separate command and params
            const char* command = p[0].c_str();
            char** params = strings_to_chars(p);

            execvp(command, params);
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
            if (i < commands.size()) {
                //From stdout to pipes
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            //Close other pipes
            close_pipes(pipes, numCommands-1, i);

            //Split string on " ", call exec
            vector<string> p = parse_input(commands[i], " ");
            for (int j = 0; j < p.size(); j++) {
                cout << p[i] << " ";
            }
            const char* command = p[0].c_str();
            char** params = strings_to_chars(p);
            
            execvp(command, params);
            //Delete params
            for (int i = 0; i < p.size(); i++) {
                delete params[i];
            }
            delete params;
            exit(0);
        }
    }
    close_pipes(pipes, numCommands-1, -1);
    for (int i = 0; i < numCommands; i++) {
        waitpid(pids[i], &status, 0);
    }
}

void Quash::io_redirects() {

}

void Quash::run() {
    string input;
    while (input != "exit" && input != "quit") {
        cout << "[Quash]$ ";
        input = take_input();

        pipe_commands(input);
        
    }
}