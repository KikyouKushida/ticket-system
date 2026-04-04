#define main ticket_system_main
#include "..\main.cpp"
#undef main
#include <iostream>
#include <fstream>
#include <string>

int main() {
    set_up();
    std::ifstream fin("..\\testcases\\2.in");
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        std::cout << "RUN " << line << "\n";
        std::cout.flush();
        execute(line);
    }
    return 0;
}
