#pragma once

#include <memory.h>
#include <string> 

constexpr int PAGE_SIZE = 4096;
constexpr int USERNAME_LEN = 21;
constexpr int PASSWORD_LEN = 31;
constexpr int NAME_LEN = 16;
constexpr int MAILADDR_LEN = 31;
constexpr int TRAIN_ID_LEN = 21;
constexpr int STATION_NAME_LEN = 31;
constexpr int STATION_NUM = 100;

typedef std::pair<int, int> pii;

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

inline int date_to_int(const pii &date) {
    if (date.first == 6) {
        return date.second;
    } else if (date.first == 7) {
        return 30 + date.second;
    } else if (date.first == 8) {
        return 61 + date.second;
    } else {
        return 92 + date.second;
    }
}

inline pii int_to_date(const int &num) {
    if (num <= 30) {
        return pii(6, num);
    } else if (num <= 61) {
        return pii(7, num - 30);
    } else if (num <= 92) {
        return pii(8, num - 61);
    } else {
        return pii(9, num - 92);
    }
}

inline int time_to_int(const pii &time) {
    return time.first * 60 + time.second;
}

inline pii int_to_time(const int &num) {
    return pii(num / 60, num % 60);
}

}