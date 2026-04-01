#include "info.h"
#include "train.h"

StationManager station_manager;

TrainRecord::TrainRecord() {
    memset(train_id, 0, sizeof(train_id));
    station_num = seat_num = 0;
    memset(station_no, 0, sizeof(station_no));
    memset(prices, 0, sizeof(prices));
    memset(travel_times, 0, sizeof(travel_times));
    memset(stop_times, 0, sizeof(stop_times));
    type = 0;
    start_time = -1;
    sale_begin_date = sale_end_date = -1;
    train_no = 0;
    released = false;
}

TrainRecord::TrainRecord(const std::string& train_id, const int station_num, const std::string *station_name, const int seat_num, 
    const int *prices, const std::string &start_time, const int *travel_times, const int *stop_times, 
    const std::string &sale_date) {
        utils::write_string(this->train_id, train_id);
        this -> station_num = station_num;
        for (int i = 0; i < station_num; ++i) {
            this -> station_no[i] = station_manager.query_station_no(station_name[i]);
        }
        this -> seat_num = seat_num;
        this -> start_time = utils::time_to_int(pii((start_time[0] - '0') * 10 
            + start_time[1], (start_time[3] - '0') * 10 + start_time[4]));
        for (int i = 0; i < station_num - 1; ++i) {
            this -> prices[i] = prices[i];
            this -> travel_times[i] = travel_times[i];
        }
        for (int i = 0; i < station_num - 2; ++i) {
            this -> stop_times[i] = stop_times[i];
        }
        this -> sale_begin_date = utils::date_to_int(pii((sale_date[0] - '0') * 10 + sale_date[1], 
            (sale_date[3] - '0') * 10 + sale_date[4]));
        this -> sale_end_date = utils::date_to_int(pii((sale_date[6] - '0') * 10 + sale_date[7], 
            (sale_date[9] - '0') * 10 + sale_date[10]));
        this -> released = false;
    }

void TrainRecord::create_train_no() {
    train_count += 1;
    this -> train_no = train_count;
}

TrainManager::TrainManager() {

}

void TrainManager::open_file() {
    file.open(train_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(train_file_name, std::ios::out | std::ios::binary); // create a new file
        file.close();
        file.open(train_file_name, std::ios::in | std::ios::out | std::ios::binary);
    }
}

void TrainManager::close_file() {
    file.close();
}

void TrainManager::write_record(const TrainRecord& train_record) {
    file.seekp(train_record.train_no * sizeof(TrainRecord));
    file.write(reinterpret_cast<const char*>(&train_record), sizeof(TrainRecord));
}

TrainRecord TrainManager::read_record(const int &train_no) {
    file.seekg(train_no * sizeof(TrainRecord));
    TrainRecord train_record = TrainRecord();
    file.read(reinterpret_cast<char*>(&train_record), sizeof(TrainRecord));
    return train_record;
}

int TrainManager::add_train(TrainRecord &train_record) {
    int this_train_no = data1.find_value(train_record.train_id);
    if (this_train_no > 0) {
        return -1;
    }
    train_record.create_train_no();
    write_record(train_record);
    return 0;
}

int TrainManager::delete_train(const std::string &train_id) {
    int this_train_no = data1.find_value(train_id);
    if (this_train_no == 0) {
        return -1;
    }
    TrainRecord this_train_record = read_record(this_train_no);
    if (this_train_record.released == true) {
        return -1;
    }
    data1.erase_value(train_id);
    return 0;
}

int TrainManager::release_train(const std::string &train_id) {
    int this_train_no = data1.find_value(train_id);
    if (this_train_no == 0) {
        return -1;
    }
    TrainRecord this_train_record = read_record(this_train_no);
    if (this_train_record.released == true) {
        return -1;
    }
    this_train_record.released = true;
    write_record(this_train_record);
    for (int i = 0; i < this_train_record.station_num; ++i) {
        data2.insert_value(this_train_record.station_no[i], this_train_no);
    }
    return 0;
}

std::vector<QueryTrainReturn> TrainManager::query_train(const std::string &train_id, const pii &depart_date) {
    int this_train_no = data1.find_value(train_id);
    std::vector<QueryTrainReturn> query_train_return;
    if (this_train_no == 0) {
        return query_train_return;
    }
    TrainRecord this_train_record = read_record(this_train_no);
    if (this_train_record.released == false) {
        return query_train_return;
    }
    if (utils::date_to_int(depart_date) < this_train_record.sale_begin_date || 
        utils::date_to_int(depart_date) > this_train_record.sale_end_date) {
            return query_train_return;
        }
    int now_date = utils::date_to_int(depart_date), now_time = this_train_record.start_time, now_price = 0;
    std::vector<int> this_seat_record = seat_manager.get_all_seats(this_train_no, now_date);
    QueryTrainReturn tmp_query_train_return;
    tmp_query_train_return.station_name = station_manager.query_station_name(this_train_record.station_no[0]);
    tmp_query_train_return.depart_date = utils::int_to_date(now_date);
    tmp_query_train_return.depart_time = utils::int_to_time(now_time);
    tmp_query_train_return.price = now_price;
    tmp_query_train_return.seat = this_seat_record[0];
    query_train_return.push_back(tmp_query_train_return);
    for (int i = 0; i < this_train_record.station_num - 2; ++i) {
        tmp_query_train_return.station_name = station_manager.query_station_name(this_train_record.station_no[i + 1]);
        now_time += this_train_record.travel_times[i];
        while (now_time >= 1440) {
            now_time -= 1440;
            now_date += 1;
        }
        now_price += this_train_record.prices[i];
        tmp_query_train_return.arrive_date = utils::int_to_date(now_date);
        tmp_query_train_return.arrive_time = utils::int_to_time(now_time);
        now_time += this_train_record.stop_times[i];
        while (now_time >= 1440) {
            now_time -= 1440;
            now_date += 1;
        }
        tmp_query_train_return.depart_date = utils::int_to_date(now_date);
        tmp_query_train_return.depart_time = utils::int_to_time(now_time);
        tmp_query_train_return.price = now_price;
        tmp_query_train_return.seat = this_seat_record[i + 1];
        query_train_return.push_back(tmp_query_train_return);
    }
    tmp_query_train_return.station_name = station_manager.query_station_name(this_train_record.station_no[this_train_record.station_num - 1]);
    now_time += this_train_record.travel_times[this_train_record.station_num - 2];
    while (now_time >= 1440) {
        now_time -= 1440;
        now_date += 1;
    }
    now_price += this_train_record.prices[this_train_record.station_num - 2];
    tmp_query_train_return.arrive_date = utils::int_to_date(now_date);
    tmp_query_train_return.arrive_time = utils::int_to_time(now_time);
    tmp_query_train_return.price = now_price;
    query_train_return.push_back(tmp_query_train_return);
    return query_train_return;
}

std::vector<QueryTicketReturn> TrainManager::query_ticket(const std::string &depart_station, const std::string &arrive_station, const pii &date, 
    int sorting) {
        std::vector<QueryTicketReturn> query_ticket_return;
        int depart_station_no = station_manager.query_station_no(depart_station);
        int arrive_station_no = station_manager.query_station_no(arrive_station);
        if (depart_station_no <= 0 || arrive_station_no <= 0 || depart_station_no == arrive_station_no) {
            return query_ticket_return;
        }
        std::vector<int> possible_train_no_1 = data2.find_all_values(station_manager.query_station_no(depart_station));
        std::vector<int> possible_train_no_2 = data2.find_all_values(station_manager.query_station_no(arrive_station));
        std::vector<int> possible_train_no = utils::intersect(possible_train_no_1, possible_train_no_2);
        for (int i = 0; i < possible_train_no.size(); ++i) {
            int this_train_no = possible_train_no[i];
            TrainRecord this_train_record = read_record(this_train_no);
            int depart_index = -1, arrive_index = -1;
            for (int j = 0; j < this_train_record.station_num; ++j) {
                if (depart_station_no == this_train_record.station_no[j]) {
                    depart_index = j;
                } 
                if (arrive_station_no == this_train_record.station_no[j]) {
                    arrive_index = j;
                }
            }
            if (depart_index == -1 || depart_index >= arrive_index) {
                continue;
            }
            int now_date = this_train_record.sale_begin_date, now_time = this_train_record.start_time;
            for (int j = 0; j < depart_index; ++j) {
                now_time += this_train_record.travel_times[j];
                while (now_time >= 1440) {
                    now_time -= 1440;
                    now_date += 1;
                }
            }
            int delta_date = now_date - this_train_record.sale_begin_date;
            if (utils::date_to_int(date) - delta_date < this_train_record.sale_begin_date || 
                utils::date_to_int(date) - delta_date > this_train_record.sale_end_date) {
                continue;
            }
            QueryTicketReturn tmp_query_ticket_return;
            tmp_query_ticket_return.train_id = this_train_record.train_id;
            tmp_query_ticket_return.depart_station_name = depart_station;
            tmp_query_ticket_return.arrive_station_name = arrive_station;
            tmp_query_ticket_return.price = 0;
            tmp_query_ticket_return.depart_date = date;
            tmp_query_ticket_return.depart_time = utils::int_to_time(now_time);
            now_date = utils::date_to_int(date);
            for (int j = depart_index; j < arrive_index; ++j) {
                tmp_query_ticket_return.price += this_train_record.prices[j];
                now_time += this_train_record.travel_times[j];
                while (now_time >= 1440) {
                    now_time -= 1440;
                    now_date += 1;
                } 
            }
            tmp_query_ticket_return.arrive_date = utils::int_to_date(now_date);
            tmp_query_ticket_return.arrive_time = utils::int_to_time(now_time);
            tmp_query_ticket_return.seat = seat_manager.get_seats(this_train_no, utils::date_to_int(date) - delta_date, depart_index, arrive_index);
            query_ticket_return.push_back(tmp_query_ticket_return);
        }
        return query_ticket_return;
    }