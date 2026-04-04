#pragma once
#include <memory.h>
#include <string> 
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include "info.h"
#include "bplustree.h"
#include "user.h"
#include "station.h"
#include "seat.h"
#include "train.h"

struct OrderRecord {
    static int order_count;
    int user_no, train_no, order_no, depart_station_no, arrive_station_no;
    int depart_time, arrive_time, depart_date, arrive_date, seat, start_date;
    long long price;
    int status; // status = 0: success; status = 1: pending; status = 2: refunded;
    OrderRecord();
    void create_order_no();
};

struct OrderManager {
    const char* order_file_name = "data/order_file.txt";
    std::fstream file;
    Bplustree<int, int> data1;
    Bplustree<pii, int> data2;
    // data1: (user_no, order_no)
    // data2: ((train_no, date), order_no), only storing incomplete order (which must be -q = true)
    OrderManager();
    void open_file();
    void close_file();
    void write_record(const OrderRecord& order_record);
    OrderRecord read_record(const int &order_no);
    int add_order(OrderRecord &order_record);
    std::vector<OrderRecord> query_order(const std::string &username);
    std::string buy_ticket(const std::string &username, const std::string &train_id, const pii &date, const int &tickets, 
        const std::string &depart_station_name, const std::string &arrive_station_name, const bool &accept_pending);
    int refund_ticket(const std::string &username, const int &index);
};