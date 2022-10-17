#include "Quash.h"


Quash::Quash() {

}


string Quash::take_input() {
    string input;
    cin >> input;
    return input;
}

void Quash::run() {
    string input;
    while (input != "exit") {
        std::cout << "[Quash]$ ";
        input = take_input();
    }

}