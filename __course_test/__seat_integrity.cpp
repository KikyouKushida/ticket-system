#define main ticket_system_main
#include "..\main.cpp"
#undef main
#include <iostream>
#include <fstream>
#include <string>

int main() {
    set_up();
    std::ifstream fin("..\\testcases\\2.in");
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        if (line.rfind("[8194]", 0) == 0) break;
        execute(line);
    }
    std::cout << "train_count=" << TrainRecord::train_count << " seat_count=" << SeatRecord::seat_count << "\n";
    int missing = 0;
    for (int tn = 1; tn <= TrainRecord::train_count; ++tn) {
        TrainRecord tr = train_manager.read_record(tn);
        if (tr.train_no <= 0) continue;
        if (!tr.released) continue;
        for (int d = tr.sale_begin_date; d <= tr.sale_end_date; ++d) {
            int seat_no = seat_manager.query_seat_no(tn, d);
            if (seat_no <= 0) {
                ++missing;
                std::cout << "missing train_no=" << tn << " train_id=" << tr.train_id << " d=" << d << " sale=[" << tr.sale_begin_date << "," << tr.sale_end_date << "] station_num=" << tr.station_num << "\n";
                if (missing >= 100) {
                    std::cout << "too_many_missing\n";
                    return 0;
                }
            }
        }
    }
    std::cout << "missing_total=" << missing << "\n";
    return 0;
}
