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

static void print_transfer(const std::vector<QueryTicketReturn> &v, const std::string &tag) {
    std::cout << tag << " size=" << v.size() << "\n";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << i << ": id=" << v[i].train_id
                  << " " << v[i].depart_station_name << ' ' << v[i].depart_date.first << '-' << v[i].depart_date.second
                  << ' ' << v[i].depart_time.first << ':' << v[i].depart_time.second
                  << " -> " << v[i].arrive_station_name << ' ' << v[i].arrive_date.first << '-' << v[i].arrive_date.second
                  << ' ' << v[i].arrive_time.first << ':' << v[i].arrive_time.second
                  << " price=" << v[i].price << " time=" << v[i].time << " seat=" << v[i].seat << "\n";
    }
}

static void add_and_release(TrainManager &tm, const std::string &id, int station_num, std::string *stations,
    int seat_num, int *prices, const std::string &start, int *travel, int *stop, const std::string &sale) {
    TrainRecord t(id, station_num, stations, seat_num, prices, start, travel, stop, sale);
    std::cout << id << " add=" << tm.add_train(t) << " release=" << tm.release_train(id) << "\n";
}

int main() {
    station_manager.open_file();
    seat_manager.open_file();
    TrainManager tm;
    tm.open_file();

    std::string s1[] = {"A", "X"};
    int p1[] = {10};
    int tr1[] = {60};
    int sp0[] = {};
    add_and_release(tm, "T1", 2, s1, 100, p1, "08:00", tr1, sp0, "06-01|06-05");

    std::string s2[] = {"X", "C"};
    int p2[] = {20};
    int tr2[] = {60};
    add_and_release(tm, "T2", 2, s2, 100, p2, "09:30", tr2, sp0, "06-01|06-05");

    std::string s3[] = {"A", "Y"};
    int p3[] = {5};
    int tr3[] = {50};
    add_and_release(tm, "T3", 2, s3, 100, p3, "08:00", tr3, sp0, "06-01|06-05");

    std::string s4[] = {"Y", "C"};
    int p4[] = {5};
    int tr4[] = {60};
    add_and_release(tm, "T4", 2, s4, 100, p4, "12:00", tr4, sp0, "06-01|06-05");

    std::string s5[] = {"A", "Z"};
    int p5[] = {8};
    int tr5[] = {20};
    add_and_release(tm, "T5", 2, s5, 100, p5, "23:30", tr5, sp0, "06-01|06-05");

    std::string s6[] = {"Z", "C"};
    int p6[] = {8};
    int tr6[] = {30};
    add_and_release(tm, "T6", 2, s6, 100, p6, "00:10", tr6, sp0, "06-01|06-05");

    print_transfer(tm.query_transfer("A", "C", pii(6,1), 1), "A->C sort=time 06-01");
    print_transfer(tm.query_transfer("A", "C", pii(6,1), 0), "A->C sort=price 06-01");
    print_transfer(tm.query_transfer("A", "C", pii(6,5), 1), "A->C sort=time 06-05");
    print_transfer(tm.query_transfer("A", "B", pii(6,1), 1), "A->B no route");
    return 0;
}
