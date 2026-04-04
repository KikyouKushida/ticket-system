#pragma once
#include <set>
#include <string>
#include <fstream>
#include <vector>
#include "info.h"
#include "station.h"
#include "seat.h"

struct SeatRecord;
struct SeatManager;

struct TrainRecord {
    static int train_count;
    char train_id[TRAIN_ID_LEN];
    int station_num;
    int station_no[STATION_NUM];
    int seat_num;
    int start_time;
    int prices[STATION_NUM - 1];
    int travel_times[STATION_NUM - 1];
    int stop_times[STATION_NUM - 2];
    int sale_begin_date, sale_end_date;
    char type;
    bool released;
    int train_no;
    TrainRecord();
    TrainRecord(const std::string& train_id, const int station_num, const std::string *station_name, const int seat_num, 
        const int *prices, const std::string &start_time, const int *travel_times, const int *stop_times, 
        const std::string &sale_date);
    void create_train_no();
};

struct QueryTicketReturn {
    std::string train_id, depart_station_name, arrive_station_name;
    pii depart_time, arrive_time, depart_date, arrive_date;
    int price, time, seat;
};

struct QueryTrainReturn {
    std::string station_name;
    pii depart_time, arrive_time, depart_date, arrive_date;
    int price, seat;
};

struct TrainManager {
    const char* train_file_name = "data/train_file.txt";
    std::fstream file;
    Bplustree<std::string, int> data1;
    Bplustree<int, int> data2;
    TrainManager();
    void open_file();
    void close_file();
    void write_record(const TrainRecord& train_record);
    TrainRecord read_record(const int &train_no);
    int query_train_no(const std::string& train_id);
    std::string query_train_id(const int &train_no);
    int add_train(TrainRecord &train_record);
    int delete_train(const std::string &train_id);
    int release_train(const std::string &train_id);
    std::vector<QueryTrainReturn> query_train(const std::string &train_id, const pii &depart_date);
    std::vector<QueryTicketReturn> query_ticket(const std::string &depart_station, const std::string &arrive_station, 
        const pii &depart_date, const pii &time, const int &exclude_train_no, const int &sorting);
    std::vector<QueryTicketReturn> query_transfer(const std::string &depart_station, const std::string &arrive_station, 
        const pii &depart_date, const int &sorting);
};

extern TrainManager train_manager;