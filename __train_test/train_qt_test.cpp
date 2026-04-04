#include <iostream>
#include <vector>
#include <string>
#include "../bplustree.cpp"
#include "../station.cpp"
#include "../seat.cpp"
#include "../train.cpp"

int StationRecord::station_count = 0;
int SeatRecord::seat_count = 0;
int TrainRecord::train_count = 0;

static void print_query(const std::vector<QueryTicketReturn> &v, const std::string &tag) {
    std::cout << tag << " size=" << v.size() << "\n";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << i << ": id=" << v[i].train_id
                  << " depart=" << v[i].depart_station_name << " " << v[i].depart_date.first << '-' << v[i].depart_date.second
                  << ' ' << v[i].depart_time.first << ':' << v[i].depart_time.second
                  << " arrive=" << v[i].arrive_station_name << " " << v[i].arrive_date.first << '-' << v[i].arrive_date.second
                  << ' ' << v[i].arrive_time.first << ':' << v[i].arrive_time.second
                  << " price=" << v[i].price << " seat=" << v[i].seat << " time=" << v[i].time << "\n";
    }
}

int main() {
    station_manager.open_file();
    seat_manager.open_file();
    TrainManager tm;
    tm.open_file();

    std::string st1[] = {"A", "X", "B"};
    int pr1[] = {10, 20};
    int tr1[] = {20, 10};
    int sp1[] = {10};
    TrainRecord t1("T1", 3, st1, 100, pr1, std::string("23:50"), tr1, sp1, std::string("06-01|06-05"));
    std::cout << "add1=" << tm.add_train(t1) << "\n";
    std::cout << "release1=" << tm.release_train("T1") << "\n";

    int no1 = tm.data1.find_value("T1");
    TrainRecord r1 = tm.read_record(no1);
    std::cout << "record sale=" << r1.sale_begin_date << ',' << r1.sale_end_date << " start=" << r1.start_time << " released=" << r1.released << " train_no=" << r1.train_no << "\n";

    print_query(tm.query_ticket("A", "B", pii(6,1), pii(0,0), 0, 1), "A->B 06-01 00:00");
    print_query(tm.query_ticket("X", "B", pii(6,1), pii(0,0), 0, 1), "X->B 06-01 00:00");
    print_query(tm.query_ticket("X", "B", pii(6,1), pii(23,55), 0, 1), "X->B 06-01 23:55");
    print_query(tm.query_ticket("X", "B", pii(6,2), pii(0,5), 0, 1), "X->B 06-02 00:05");
    print_query(tm.query_ticket("X", "B", pii(6,5), pii(23,55), 0, 1), "X->B 06-05 23:55");
    print_query(tm.query_ticket("X", "B", pii(6,6), pii(0,0), 0, 1), "X->B 06-06 00:00");
    return 0;
}
