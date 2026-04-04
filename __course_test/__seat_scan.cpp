#define main ticket_system_main
#include "..\\main.cpp"
#undef main
#include <fstream>

int main() {
    set_up();
    std::ifstream in("..\\testcases\\2.in");
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("[8194]", 0) == 0) break;
        if (!line.empty()) execute(line);
    }
    std::vector<int> station_candidates = train_manager.data2.find_all_values(station_manager.query_station_no("江苏省淮阴市"));
    std::cout << "station_candidates=" << station_candidates.size() << "\n";
    for (int idx = 0; idx < station_candidates.size(); ++idx) {
        int train_no = station_candidates[idx];
        TrainRecord tr = train_manager.read_record(train_no);
        bool has_station = false;
        for (int j = 0; j < tr.station_num; ++j) {
            if (station_manager.query_station_no("江苏省淮阴市") == tr.station_no[j]) has_station = true;
        }
        if (!has_station) continue;
        std::cout << "train=" << tr.train_id << " train_no=" << train_no << " sale=[" << tr.sale_begin_date << "," << tr.sale_end_date << "]\n";
        for (int d = tr.sale_begin_date; d <= tr.sale_end_date; ++d) {
            int seat_no = seat_manager.query_seat_no(train_no, d);
            if (seat_no <= 0) {
                std::cout << "  missing date=" << d << "\n";
                break;
            }
        }
    }
}
