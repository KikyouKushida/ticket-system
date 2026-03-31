#pragma once
#include <set>
#include <string>
#include <fstream>
#include <vector>
#include <cassert>
#include "info.h"
#include "bplustree.h"

struct SeatRecord {
    static int seat_count;
    int train_no, date;
    int seats[STATION_NUM - 1];
    int seat_no;
    SeatRecord();
    void create_seat_no();
};

struct SeatManager {
    const char* seat_file_name = "seat_file.txt";
    std::fstream file;
    Bplustree<pii, int> data;
    SeatManager();
    void open_file();
    void close_file();
    void write_record(const SeatRecord& seat_record);
    SeatRecord read_record(const int &seat_no);
    void add_seats(const int &train_no, const int &date, const int &seat_num);
    int get_seats(const int &train_no, const int &date, const int &l, const int &r);
    std::vector<int> get_all_seats(const int &train_no, const int &date);
    int buy_seats(const int &train_no, const int &date, const int &l, const int &r);
    int refund_seats(const int &train_no, const int &date, const int &l, const int &r);
};

SeatManager seat_manager;