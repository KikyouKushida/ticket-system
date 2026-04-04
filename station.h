#pragma once
#include <set>
#include <string>
#include <fstream>
#include "info.h"
#include "bplustree.h"

struct StationRecord {
    static int station_count;
    int station_no;
    char station_name[STATION_NAME_LEN];
    StationRecord();
    void create_station_no();
};

struct StationManager {
    const char* station_file_name = "data/station_file.txt";
    std::fstream file;
    Bplustree<std::string, int> data;
    StationManager();
    void open_file();
    void close_file();
    void write_record(const StationRecord& station_record);
    StationRecord read_record(const int &station_no);
    int add_station(StationRecord &station_record);
    int query_station_no(const std::string& station_name);
    std::string query_station_name(const int &station_no);
};

extern StationManager station_manager;