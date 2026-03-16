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

UserManager user_manager;

bool execute(std::string &instruction) {
    std::vector<std::string> parts = separateBy(instruction, ' ');
    if (parts.size() < 2) {
        throw std::runtime_error("Invalid instruction!"); 
    }
    std::cout << parts[0] << " ";
    if (parts.size() == 2 && parts[1] == "exit") {
        // TO DO: Logout everyone
        user_manager.logout_all_user();
        std::cout << "bye\n";
        return false;
    } else if (parts.size() == 2 && parts[1] == "clean") {
        // TO DO: Clean all the data
    } else if (parts[1] == "add_user") {
        std::string c, u, p, n, m; 
        int g;
        for (int i = 2; i < parts.size(); i += 2) {
            if (parts[i] == "-c") {
                c = parts[i + 1];
            } else if (parts[i] == "-u") {
                u = parts[i + 1];
            } else if (parts[i] == "-p") {
                p = parts[i + 1];
            } else if (parts[i] == "-n") {
                n = parts[i + 1];
            } else if (parts[i] == "-m") {
                m = parts[i + 1];
            } else {
                g = utils::string_to_int(parts[i + 1]);
            }
        }
        UserRecord user_record(u, p, n, m, g);
        std::cout << user_manager.add_user(c, user_record) << "\n";
    } else if (parts[1] == "login") {
        std::string u, p;
        for (int i = 2; i < parts.size(); i += 2) {
            if (parts[i] == "-u") {
                u = parts[i + 1];
            } else {
                p = parts[i + 1];
            }
        }
        std::cout << user_manager.login_user(u, p) << "\n";
    } else if (parts[1] == "logout") {
        std::string u = parts[3];
        std::cout << user_manager.logout_user(u) << "\n";
    } else if (parts[1] == "query_profile") {
        std::string c, u;
        for (int i = 2; i < parts.size(); i += 2) {
            if (parts[i] == "-c") {
                c = parts[i + 1];
            } else {
                u = parts[i + 1];
            }
        }
        std::cout << user_manager.query_profile(c, u) << "\n";
    } else if (parts[1] == "modify_profile") {
        std::string c, u, p, n, m; 
        int g;
        for (int i = 2; i < parts.size(); i += 2) {
            if (parts[i] == "-c") {
                c = parts[i + 1];
            } else if (parts[i] == "-u") {
                u = parts[i + 1];
            } else if (parts[i] == "-p") {
                p = parts[i + 1];
            } else if (parts[i] == "-n") {
                n = parts[i + 1];
            } else if (parts[i] == "-m") {
                m = parts[i + 1];
            } else {
                g = utils::string_to_int(parts[i + 1]);
            }
        }
        UserRecord user_record(u, p, n, m, g);
        std::cout << user_manager.modify_profile(c, user_record) << "\n";
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