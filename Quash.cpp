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

vector<string> Quash::parse_input(string input, string delimiter) {
    size_t pos = 0;
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

void Quash::io_redirects() {

}

void Quash::pipe_commands(string input) {
    commands = parse_input(input, "|");


    for (int i = 0; i < commands.size(); i++) {
        
    }


}

void Quash::run() {
    string input;
    while (input != "exit" && input != "quit") {
        cout << "[Quash]$ ";
        input = take_input();

        pipe_commands(input);
        
    }
}