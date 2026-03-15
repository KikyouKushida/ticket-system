#include "file.h"
#include <fstream>

template<class T>
void write_obj(std::fstream& file, long long offset, const T& obj) {
    file.seekp(offset);
    file.write(reinterpret_cast<const char*>(&obj), sizeof(T));
    file.flush();
}

template<class T>
void read_obj(std::fstream& file, long long offset, T& obj) {
    file.seekg(offset);
    file.read(reinterpret_cast<char*>(&obj), sizeof(T));
}