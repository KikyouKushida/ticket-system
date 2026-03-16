#pragma once

#include <memory.h>
#include <string> 

constexpr int PAGE_SIZE = 4096;
constexpr int USERNAME_LEN = 21;
constexpr int PASSWORD_LEN = 31;
constexpr int NAME_LEN = 16;
constexpr int MAILADDR_LEN = 31;

namespace utils {

inline void write_string(char* dest, const std::string &src) {
    memcpy(dest, src.data(), src.size());
    dest[src.size()] = '\0';
}

inline int string_to_int(const std::string &src) {
    int result = 0;
    for (int i = 0; i < src.size(); ++i) {
        result = result * 10 + src[i] - '0';
    }
    return result;
}

}