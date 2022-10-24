
#include "Quash.h"
using namespace std;

void trimString(string &str) {
    while (isspace(str.at(0))) {
        str = str.substr(1);
    }
    while (isspace(str.at(str.size()-1))) {
        str = str.substr(0, str.size()-1);
    }
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

        parsed.push_back(currProcess);
        input.erase(0, index + delimiter.length());
        delete[] pos;
    }

    return parsed;
}

 
int main() {
    vector<string> delimiters {"", "|", ">", "<"};

    string input = "cat file.txt > file2.txt";

    vector<Process> processes = parseInput(input, delimiters);
    
    for (int i = 0; i < processes.size(); i++) {
        for (int j = 0; j < processes[i].params.size(); j++) {
            cout << processes[i].params[j] << " ";
        }

        cout << processes[i].delimiter << " " << processes[i].builtIn << "\n";
    }

    return 0;
}