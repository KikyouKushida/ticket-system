#pragma once

#include <memory.h>
#include <string> 
#include <algorithm>
#include <vector>

constexpr int PAGE_SIZE = 4096;
constexpr int USERNAME_LEN = 21;
constexpr int PASSWORD_LEN = 31;
constexpr int NAME_LEN = 16;
constexpr int MAILADDR_LEN = 31;
constexpr int TRAIN_ID_LEN = 21;
constexpr int STATION_NAME_LEN = 31;
constexpr int STATION_NUM = 100;
const std::string DATA_DIR = "data/";

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

inline std::string int_to_string(const int &src) {
    std::string result;
    if (src == 0) {
        return "0";
    }
    int x = abs(src);
    while (x) {
        result += (x % 10 + '0');
        x = x / 10;
    }
    if (src < 0) {
        result += '-';
    }
    std::reverse(result.begin(), result.end());
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

inline std::vector<int> intersect(std::vector<int> &v1, std::vector<int> &v2) {
    std::vector<int> ans;
    if (v1.size() == 0 || v2.size() == 0) {
        return ans;
    }
    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());
    for (int i = 0, j = 0; i < v1.size(); ++i) {
        while (j + 1 < v2.size() && v2[j] < v1[i]) {
            ++j;
        }
        if (v1[i] == v2[j]) {
            ans.push_back(v1[i]);
        }
    }
    return ans;
}

}