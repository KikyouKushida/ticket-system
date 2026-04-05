#include "info.h"
#include "train.h"
#include "file.h"

StationManager station_manager;

int TrainRecord::train_count = 0;

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
        utils::write_string(this->train_id, train_id, TRAIN_ID_LEN);
        this -> station_num = station_num;
        for (int i = 0; i < station_num; ++i) {
            StationRecord station_record;
            for (int j = 0; j < station_name[i].size(); ++j) {
                station_record.station_name[j] = station_name[i][j];
            }
            station_manager.add_station(station_record);
            this -> station_no[i] = station_manager.query_station_no(station_name[i]);
        }
        this -> seat_num = seat_num;
        this -> start_time = utils::time_to_int(pii((start_time[0] - '0') * 10 
            + start_time[1] - '0', (start_time[3] - '0') * 10 + start_time[4] - '0'));
        for (int i = 0; i < station_num - 1; ++i) {
            this -> prices[i] = prices[i];
            this -> travel_times[i] = travel_times[i];
        }
        for (int i = 0; i < station_num - 2; ++i) {
            this -> stop_times[i] = stop_times[i];
        }
        this -> sale_begin_date = utils::date_to_int(pii((sale_date[0] - '0') * 10 + sale_date[1] - '0', 
            (sale_date[3] - '0') * 10 + sale_date[4] - '0'));
        this -> sale_end_date = utils::date_to_int(pii((sale_date[6] - '0') * 10 + sale_date[7] - '0', 
            (sale_date[9] - '0') * 10 + sale_date[10] - '0'));
        this -> released = false;
    }

void TrainRecord::create_train_no() {
    train_count += 1;
    this -> train_no = train_count;
    meta_manager.sync_from_static();
}

int TrainManager::query_train_no(const std::string& train_id) {
    return data1.find_value(train_id);
}

std::string TrainManager::query_train_id(const int &train_no) {
    TrainRecord this_train_record = train_manager.read_record(train_no);
    return this_train_record.train_id;
}

TrainManager::TrainManager() : data1("train_manager_data1"), data2("train_manager_data2") {
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
    data1.insert_value(train_record.train_id, train_record.train_no);
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
    for (int i = this_train_record.sale_begin_date; i <= this_train_record.sale_end_date; ++i) {
        seat_manager.add_seats(this_train_record.train_no, i, this_train_record.seat_num);
    } 
    return 0;
}

std::vector<QueryTrainReturn> TrainManager::query_train(const std::string &train_id, const pii &depart_date) {
    int this_train_no = data1.find_value(train_id);
    std::vector<QueryTrainReturn> query_train_return;
    if (this_train_no <= 0) {
        return query_train_return;
    }
    TrainRecord this_train_record = read_record(this_train_no);
    if (utils::date_to_int(depart_date) < this_train_record.sale_begin_date || 
        utils::date_to_int(depart_date) > this_train_record.sale_end_date) {
            return query_train_return;
        }
    int now_date = utils::date_to_int(depart_date), now_time = this_train_record.start_time, now_price = 0;
    std::vector<int> this_seat_record;
    if (this_train_record.released == true) {
        this_seat_record = seat_manager.get_all_seats(this_train_no, now_date);
    } else {
        for (int i = 0; i < this_train_record.station_num - 1; ++i) {
            this_seat_record.push_back(this_train_record.seat_num);
        }
    }
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

std::vector<QueryTicketReturn> TrainManager::query_ticket(const std::string &depart_station, const std::string &arrive_station, const pii &depart_date, 
    const pii &time, const int &exclude_train_no, const int &sorting) {
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
            if (exclude_train_no == this_train_no) {
                continue;
            }
            pii date = depart_date;
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
                now_time += this_train_record.stop_times[j];
                while (now_time >= 1440) {
                    now_time -= 1440;
                    now_date += 1;
                }
            }
            if (now_time < utils::time_to_int(time)) {
                date = utils::int_to_date(utils::date_to_int(date) + 1);
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
                if (j > depart_index) {
                    now_time += this_train_record.stop_times[j - 1];
                }
                while (now_time >= 1440) {
                    now_time -= 1440;
                    now_date += 1;
                } 
            }
            tmp_query_ticket_return.arrive_date = utils::int_to_date(now_date);
            tmp_query_ticket_return.arrive_time = utils::int_to_time(now_time);
            tmp_query_ticket_return.seat = seat_manager.get_seats(this_train_no, utils::date_to_int(date) - delta_date, depart_index, arrive_index);
            tmp_query_ticket_return.time = (utils::date_to_int(tmp_query_ticket_return.arrive_date) - 
                utils::date_to_int(tmp_query_ticket_return.depart_date)) * 1440 + utils::time_to_int(tmp_query_ticket_return.arrive_time)
                - utils::time_to_int(tmp_query_ticket_return.depart_time);
            query_ticket_return.push_back(tmp_query_ticket_return);
        }
        if (sorting == 1) {
            // sort by time and train_id
            std::sort(query_ticket_return.begin(), query_ticket_return.end(), [&](QueryTicketReturn &a, QueryTicketReturn &b) {
                if (a.time != b.time) {
                    return a.time < b.time;
                } else {
                    return a.train_id < b.train_id;
                }
            });
        } else {
            // sort by price and train_id
            std::sort(query_ticket_return.begin(), query_ticket_return.end(), [&](QueryTicketReturn &a, QueryTicketReturn &b) {
                if (a.price != b.price) {
                    return a.price < b.price;
                } else {
                    return a.train_id < b.train_id;
                }
            });
        }
        return query_ticket_return;
    }

std::vector<QueryTicketReturn> TrainManager::query_transfer(const std::string &depart_station, const std::string &arrive_station, 
    const pii &date, const int &sorting) {
        std::vector<QueryTicketReturn> query_transfer_return;
        int min_total_time = -1, min_total_price = -1;
        int depart_station_no = station_manager.query_station_no(depart_station);
        int arrive_station_no = station_manager.query_station_no(arrive_station);
        std::vector<int> possible_train_no_2 = data2.find_all_values(arrive_station_no);
        if (depart_station_no <= 0 || arrive_station_no <= 0 || depart_station_no == arrive_station_no) {
            return query_transfer_return;
        }
        std::vector<int> possible_train_no_1 = data2.find_all_values(station_manager.query_station_no(depart_station));
        for (int i = 0; i < possible_train_no_1.size(); ++i) {
            int this_train_no = possible_train_no_1[i];
            TrainRecord this_train_record = read_record(this_train_no);
            int depart_index = -1;
            for (int j = 0; j < this_train_record.station_num; ++j) {
                if (depart_station_no == this_train_record.station_no[j]) {
                    depart_index = j;
                }
            }
            if (depart_index == -1) {
                continue;
            }
            int now_date = this_train_record.sale_begin_date, now_time = this_train_record.start_time;
            for (int j = 0; j < depart_index; ++j) {
                now_time += this_train_record.travel_times[j];
                now_time += this_train_record.stop_times[j];
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
            now_date = utils::date_to_int(date);
            QueryTicketReturn tmp1, tmp2;
            std::vector<int> this_seat_record = seat_manager.get_all_seats(this_train_no, now_date - delta_date);
            tmp1.train_id = this_train_record.train_id;
            tmp1.price = tmp1.time = 0;
            tmp1.seat = 1e9;
            tmp1.depart_station_name = depart_station;
            tmp1.depart_date = date;
            tmp1.depart_time = utils::int_to_time(now_time);
            for (int j = depart_index; j < this_train_record.station_num - 1; ++j) {
                now_time += this_train_record.travel_times[j];
                if (j > depart_index) {
                    now_time += this_train_record.stop_times[j - 1];
                }
                while (now_time >= 1440) {
                    now_time -= 1440;
                    now_date += 1;
                }
                tmp1.arrive_station_name = station_manager.query_station_name(this_train_record.station_no[j + 1]);
                tmp1.arrive_date = utils::int_to_date(now_date);
                tmp1.arrive_time = utils::int_to_time(now_time);
                tmp1.price += this_train_record.prices[j];
                tmp1.time += this_train_record.travel_times[j];
                if (j > depart_index) {
                    tmp1.time += this_train_record.stop_times[j - 1];
                }
                tmp1.seat = std::min(tmp1.seat, this_seat_record[j]);
                int transfer_station_no = this_train_record.station_no[j + 1];
                std::vector<int> possible_train_no_3 = data2.find_all_values(transfer_station_no);
                std::vector<int> possible_transfer_train_no = utils::intersect(possible_train_no_2, possible_train_no_3);
                int max_next_date = -1;
                for (int k = 0; k < possible_transfer_train_no.size(); ++k) {
                    int next_train_no = possible_transfer_train_no[k];
                    if (next_train_no == this_train_no) {
                        continue;
                    }
                    TrainRecord next_train_record = read_record(next_train_no);
                    int transfer_index = -1, arrive_index = -1;
                    for (int l = 0; l < next_train_record.station_num; ++l) {
                        if (next_train_record.station_no[l] == transfer_station_no) {
                            transfer_index = l;
                        }
                        if (next_train_record.station_no[l] == arrive_station_no) {
                            arrive_index = l;
                        }
                    }
                    if (transfer_index == -1 || transfer_index >= arrive_index) {
                        continue;
                    }
                    int next_date = next_train_record.sale_begin_date, next_time = next_train_record.start_time;
                    for (int l = 0; l < transfer_index; ++l) {
                        next_time += next_train_record.travel_times[l];
                        next_time += next_train_record.stop_times[l];
                        while (next_time >= 1440) {
                            next_time -= 1440;
                            next_date += 1;
                        }
                    }
                    max_next_date = std::max(max_next_date, next_train_record.sale_end_date + next_date - next_train_record.sale_begin_date);
                }
                int begin_date = utils::date_to_int(tmp1.arrive_date);
                for (int next_date = begin_date; next_date <= max_next_date; ++next_date) {
                    pii next_depart_date = utils::int_to_date(next_date);
                    pii next_depart_time = pii(0, 0);
                    if (next_date == begin_date) {
                        next_depart_time = tmp1.arrive_time;
                    }
                    std::vector<QueryTicketReturn> tmp3 = query_ticket(tmp1.arrive_station_name, 
                        arrive_station, next_depart_date, next_depart_time, this_train_no, sorting);
                    for (int k = 0; k < tmp3.size(); ++k) {
                        tmp2 = tmp3[k];
                        int total_price = tmp1.price + tmp2.price;
                        int total_time = utils::time_to_int(tmp2.arrive_time) - utils::time_to_int(tmp1.depart_time)
                            + (utils::date_to_int(tmp2.arrive_date) - utils::date_to_int(tmp1.depart_date)) * 1440;
                        if (query_transfer_return.size() == 0) {
                            query_transfer_return.push_back(tmp1);
                            query_transfer_return.push_back(tmp2);
                            min_total_price = total_price;
                            min_total_time = total_time;
                        } else {
                            if (sorting == 1) {
                                if (total_time < min_total_time) {
                                    query_transfer_return[0] = tmp1;
                                    query_transfer_return[1] = tmp2;
                                    min_total_price = total_price;
                                    min_total_time = total_time;
                                } else if (total_time == min_total_time && total_price < min_total_price) {
                                    query_transfer_return[0] = tmp1;
                                    query_transfer_return[1] = tmp2;
                                    min_total_price = total_price;
                                    min_total_time = total_time;
                                } else if (total_time == min_total_time && total_price == min_total_price
                                    && tmp1.train_id < query_transfer_return[0].train_id) {
                                        query_transfer_return[0] = tmp1;
                                        query_transfer_return[1] = tmp2;
                                        min_total_price = total_price;
                                        min_total_time = total_time;
                                } else if (total_time == min_total_time && total_price == min_total_price
                                    && tmp1.train_id == query_transfer_return[0].train_id
                                    && tmp2.train_id < query_transfer_return[1].train_id) {
                                        query_transfer_return[0] = tmp1;
                                        query_transfer_return[1] = tmp2;
                                        min_total_price = total_price;
                                        min_total_time = total_time;
                                }
                            } else {
                                if (total_price < min_total_price) {
                                    query_transfer_return[0] = tmp1;
                                    query_transfer_return[1] = tmp2;
                                    min_total_price = total_price;
                                    min_total_time = total_time;
                                } else if (total_price == min_total_price && total_time < min_total_time) {
                                    query_transfer_return[0] = tmp1;
                                    query_transfer_return[1] = tmp2;
                                    min_total_price = total_price;
                                    min_total_time = total_time;
                                } else if (total_time == min_total_time && total_price == min_total_price
                                    && tmp1.train_id < query_transfer_return[0].train_id) {
                                        query_transfer_return[0] = tmp1;
                                        query_transfer_return[1] = tmp2;
                                        min_total_price = total_price;
                                        min_total_time = total_time;
                                } else if (total_time == min_total_time && total_price == min_total_price
                                    && tmp1.train_id == query_transfer_return[0].train_id
                                    && tmp2.train_id < query_transfer_return[1].train_id) {
                                        query_transfer_return[0] = tmp1;
                                        query_transfer_return[1] = tmp2;
                                        min_total_price = total_price;
                                        min_total_time = total_time;
                                }
                            }
                        }
                    }
                }
            }
        }
        return query_transfer_return;
    }
