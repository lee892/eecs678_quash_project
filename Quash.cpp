#include "Quash.h"


Quash::Quash() {

}

void echo() {

}

void Quash::setup() {

    //commands["echo"] = echo;
}

std::string Quash::take_input() {
    std::string input;
    std::cin >> input;

    return input;
}


void Quash::run() {
    std::string input;
    while (input != "exit") {
        std::cout << "[Quash]$ ";
        input = take_input();
    }
    std::cout << getenv("PATH");
}