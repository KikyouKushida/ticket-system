#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include "info.h"
#include "file.h"
#include "user.h"
#include "train.h"

std::vector<std::string> separateBy(std::string &str, char c) {
    std::vector<std::string> processed_list_of_strings;
    for (int i = 0, j; i < str.size(); i = j + 1) {
        j = i;
        if (str[i] == c) {
            continue;
        }
        while (j + 1 < str.size() && str[j + 1] != c) {
            ++j;
        }
        processed_list_of_strings.push_back(str.substr(i, j - i + 1));
    }
    return processed_list_of_strings;
}

bool execute(std::string &instruction) {
    std::vector<std::string> parts = separateBy(instruction, ' ');
    if (parts.size() < 1) {
        throw std::runtime_error("Invalid instruction!"); 
    }
    if (parts.size() == 1 && parts[0] == "exit") {
        // TO DO: Logout everyone
        return false;
    } else if (parts.size() == 1 && parts[0] == "clean") {
        // TO DO: Clean all the data
    }
    return true;
}

int main() {
    std::cout << "Welcome to the ticket system!\n";
    while (true) {
        std::string instruction;
        std::getline(std::cin, instruction);
        if (execute(instruction) == false) {
            break;
        }
    }
    return 0;
}