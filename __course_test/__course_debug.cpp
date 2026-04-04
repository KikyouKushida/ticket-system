#define main ticket_system_main
#include "..\main.cpp"
#undef main
#include <fstream>

int main() {
    set_up();
    std::ifstream in("..\\testcases\\2.in");
    std::string line, target;
    while (std::getline(in, line)) {
        if (line.rfind("[8194]", 0) == 0) { target = line; break; }
        if (!line.empty()) execute(line);
    }
    std::cout << "TARGET=" << target << "\n";
    std::vector<std::string> parts = separateBy(target, ' ');
    std::string s, t, d, p = "time";
    for (int j = 2; j < parts.size(); j += 2) {
        if (parts[j] == "-s") s = parts[j + 1];
        else if (parts[j] == "-t") t = parts[j + 1];
        else if (parts[j] == "-d") d = parts[j + 1];
        else p = parts[j + 1];
    }
    pii depart_date = string_to_date(d);
    pii time = pii(0,0);
    int depart_station_no = station_manager.query_station_no(s);
    int arrive_station_no = station_manager.query_station_no(t);
    std::cout << "depart_no=" << depart_station_no << " arrive_no=" << arrive_station_no << "\n";
    std::vector<int> possible_train_no_1 = train_manager.data2.find_all_values(depart_station_no);
    std::vector<int> possible_train_no_2 = train_manager.data2.find_all_values(arrive_station_no);
    std::vector<int> possible_train_no = utils::intersect(possible_train_no_1, possible_train_no_2);
    std::cout << "cand=" << possible_train_no.size() << "\n";
    for (int idx = 0; idx < possible_train_no.size(); ++idx) {
        int this_train_no = possible_train_no[idx];
        TrainRecord tr = train_manager.read_record(this_train_no);
        int depart_index = -1, arrive_index = -1;
        for (int j = 0; j < tr.station_num; ++j) {
            if (depart_station_no == tr.station_no[j]) depart_index = j;
            if (arrive_station_no == tr.station_no[j]) arrive_index = j;
        }
        if (depart_index == -1 || depart_index >= arrive_index) continue;
        pii date = depart_date;
        int now_date = tr.sale_begin_date, now_time = tr.start_time;
        for (int j = 0; j < depart_index; ++j) {
            now_time += tr.travel_times[j];
            now_time += tr.stop_times[j];
            while (now_time >= 1440) { now_time -= 1440; now_date += 1; }
        }
        if (now_time < utils::time_to_int(time)) {
            date = utils::int_to_date(utils::date_to_int(date) + 1);
        }
        int delta_date = now_date - tr.sale_begin_date;
        int start_date = utils::date_to_int(date) - delta_date;
        int seat_no = seat_manager.query_seat_no(this_train_no, start_date);
        std::cout << tr.train_id << " train_no=" << this_train_no
                  << " sale=[" << tr.sale_begin_date << "," << tr.sale_end_date << "]"
                  << " depart_idx=" << depart_index << " arrive_idx=" << arrive_index
                  << " pass_date=" << now_date << " pass_time=" << now_time
                  << " delta=" << delta_date << " query_date=" << utils::date_to_int(date)
                  << " start_date=" << start_date << " seat_no=" << seat_no << "\n";
    }
}
