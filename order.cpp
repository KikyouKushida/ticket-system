#include "order.h"
#include "file.h"

int OrderRecord::order_count = 0;

OrderRecord::OrderRecord() {

}

void OrderRecord::create_order_no() {
    order_count += 1;
    order_no = order_count;
    meta_manager.sync_from_static();
}

OrderManager::OrderManager() : data1("order_manager_data1"), data2("order_manager_data2") {
}

void OrderManager::open_file() {
    file.open(order_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(order_file_name, std::ios::out | std::ios::binary); // create a new file
        file.close();
        file.open(order_file_name, std::ios::in | std::ios::out | std::ios::binary);
    }
}

void OrderManager::close_file() {
    file.close();
}

void OrderManager::write_record(const OrderRecord &order_record) {
    file.seekp(order_record.order_no * sizeof(OrderRecord));
    file.write(reinterpret_cast<const char*>(&order_record), sizeof(OrderRecord));
}

OrderRecord OrderManager::read_record(const int &order_no) {
    file.seekg(order_no * sizeof(OrderRecord));
    OrderRecord order_record = OrderRecord();
    file.read(reinterpret_cast<char*>(&order_record), sizeof(OrderRecord));
    return order_record;
}

int OrderManager::add_order(OrderRecord &order_record) {
    order_record.create_order_no();
    write_record(order_record);
    data1.insert_value(order_record.user_no, order_record.order_no);
    if (order_record.status == 1) {
        data2.insert_value(pii(order_record.train_no, order_record.start_date), order_record.order_no);
    }
    return 0;
}

std::vector<OrderRecord> OrderManager::query_order(const std::string &username) {
    std::vector<OrderRecord> query_order_return;
    int this_user_no = user_manager.query_user_no(username);
    if (this_user_no <= 0 || logined_user.find(username) == logined_user.end()) {
        OrderRecord order_record;
        order_record.order_no = -1;
        query_order_return.push_back(order_record);
        return query_order_return;
    }
    std::vector<int> all_order_nos = data1.find_all_values(this_user_no);
    for (int i = 0; i < all_order_nos.size(); ++i) {
        query_order_return.push_back(read_record(all_order_nos[i]));
    }
    std::reverse(query_order_return.begin(), query_order_return.end());
    return query_order_return;
}

std::string OrderManager::buy_ticket(const std::string &username, const std::string &train_id, const pii &date, const int &tickets, 
    const std::string &depart_station_name, const std::string &arrive_station_name, const bool &accept_pending) {
        int this_user_no = user_manager.query_user_no(username);
        if (this_user_no <= 0) {
            return "-1";
        }
        if (logined_user.find(username) == logined_user.end()) {
            return "-1";
        }
        int this_train_no = train_manager.query_train_no(train_id);
        if (this_train_no <= 0) {
            return "-1";
        }
        int depart_station_no = station_manager.query_station_no(depart_station_name);
        int arrive_station_no = station_manager.query_station_no(arrive_station_name);
        if (depart_station_no <= 0 || arrive_station_no <= 0) {
            return "-1";
        }
        TrainRecord this_train_record = train_manager.read_record(this_train_no);
        if (this_train_record.released == false) {
            return "-1";
        }
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
            return "-1";
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
                return "-1";
            }
        now_date = utils::date_to_int(date);
        OrderRecord this_order_record;
        this_order_record.user_no = this_user_no;
        this_order_record.train_no = this_train_no;
        this_order_record.depart_station_no = depart_station_no;
        this_order_record.arrive_station_no = arrive_station_no;
        this_order_record.depart_date = now_date;
        this_order_record.depart_time = now_time;
        this_order_record.price = 0;
        this_order_record.seat = tickets;
        for (int j = depart_index; j < arrive_index; ++j) {
            now_time += this_train_record.travel_times[j];
            this_order_record.price += this_train_record.prices[j];
            if (j > depart_index) {
                now_time += this_train_record.stop_times[j - 1];
            }
            while (now_time >= 1440) {
                now_time -= 1440;
                now_date += 1;
            }
        }
        this_order_record.arrive_date = now_date;
        this_order_record.arrive_time = now_time;
        this_order_record.price = 1ll * this_order_record.price * tickets;
        this_order_record.start_date = utils::date_to_int(date) - delta_date;
        int available_tickets = seat_manager.get_seats(this_train_no, utils::date_to_int(date) - delta_date, depart_index, arrive_index);
        if (tickets > available_tickets) {
            if (accept_pending == true) {
                this_order_record.status = 1;
                add_order(this_order_record);
                return "queue";
            } else {
                return "-1";
            }
        }
        this_order_record.status = 0;
        add_order(this_order_record);
        SeatRecord this_seat_record = seat_manager.read_record(seat_manager.query_seat_no(this_train_no, utils::date_to_int(date) - delta_date));
        for (int j = depart_index; j < arrive_index; ++j) {
            this_seat_record.seats[j] -= tickets;
        }
        seat_manager.write_record(this_seat_record);
        return utils::int_to_string(this_order_record.price);
    }
        
int OrderManager::refund_ticket(const std::string &username, const int &index) {
    int this_user_no = user_manager.query_user_no(username);
    if (this_user_no <= 0) {
        return -1;
    }
    if (logined_user.find(username) == logined_user.end()) {
        return -1;
    }
    std::vector<int> all_order_records = data1.find_all_values(this_user_no);
    if (all_order_records.size() < index) {
        return -1;
    }
    int this_order_no = all_order_records[all_order_records.size() - index];
    OrderRecord this_order_record = read_record(this_order_no);
    if (this_order_record.status == 2) {
        return -1;
    } else if (this_order_record.status == 1) {
        this_order_record.status = 2;
        write_record(this_order_record);
        data2.erase_value(pii(this_order_record.train_no, this_order_record.start_date), this_order_record.order_no);
        return 0;
    }
    this_order_record.status = 2;
    write_record(this_order_record);
    SeatRecord this_seat_record = seat_manager.read_record(seat_manager.query_seat_no(this_order_record.train_no, this_order_record.start_date));
    TrainRecord this_train_record = train_manager.read_record(this_order_record.train_no);
    int depart_index = -1, arrive_index = -1;
    for (int j = 0; j < this_train_record.station_num; ++j) {
        if (this_train_record.station_no[j] == this_order_record.depart_station_no) {
            depart_index = j;
        } else if (this_train_record.station_no[j] == this_order_record.arrive_station_no) {
            arrive_index = j;
        }
    }
    for (int j = depart_index; j < arrive_index; ++j) {
        this_seat_record.seats[j] += this_order_record.seat;
    }
    std::vector<int> all_order_nos_to_try = data2.find_all_values(pii(this_order_record.train_no, this_order_record.start_date));
    for (int i = 0; i < all_order_nos_to_try.size(); ++i) {
        OrderRecord try_order_record = read_record(all_order_nos_to_try[i]);
        TrainRecord try_train_record = train_manager.read_record(try_order_record.train_no);
        int depart_index = -1, arrive_index = -1;
        for (int j = 0; j < try_train_record.station_num; ++j) {
            if (try_train_record.station_no[j] == try_order_record.depart_station_no) {
                depart_index = j;
            } else if (try_train_record.station_no[j] == try_order_record.arrive_station_no) {
                arrive_index = j;
            }
        }
        int available_seats = this_seat_record.seats[depart_index];
        for (int j = depart_index + 1; j < arrive_index; ++j) {
            if (available_seats < try_order_record.seat) {
                break;
            }
            available_seats = std::min(available_seats, this_seat_record.seats[j]);
        }
        if (available_seats < try_order_record.seat) {
            continue;
        }
        for (int j = depart_index; j < arrive_index; ++j) {
            this_seat_record.seats[j] -= try_order_record.seat;
        }
        try_order_record.status = 0;
        write_record(try_order_record);
        data2.erase_value(pii(try_order_record.train_no, try_order_record.start_date), try_order_record.order_no);
    }
    seat_manager.write_record(this_seat_record);
    return 0;
}

