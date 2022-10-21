#include "Quash.h"


Quash::Quash() {

}


string Quash::take_input() {
    string input;
    std::getline (std::cin,input);
    return input;
}

void Quash::run() {
    string input;
    while (input != "exit") {
        std::cout << "[Quash]$ ";
        input = take_input();
        std::string firstWord = input.substr(0, input.find(" "));
        if (firstWord == "echo") {
            std::cout << input.substr(4) << "\n";
        }
    }
}