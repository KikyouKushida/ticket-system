#include "seat.h"

SeatRecord::SeatRecord() {

}

void SeatRecord::create_seat_no() {
    seat_count += 1;
    seat_no = seat_count;
}

SeatManager::SeatManager() {

}

void SeatManager::open_file() {
    file.open(seat_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(seat_file_name, std::ios::out | std::ios::binary); // create a new file
        file.close();
        file.open(seat_file_name, std::ios::in | std::ios::out | std::ios::binary);
    }
}

void SeatManager::close_file() {
    file.close();
}

void SeatManager::write_record(const SeatRecord& seat_record) {
    file.seekp(seat_record.seat_no * sizeof(SeatRecord));
    file.write(reinterpret_cast<const char*>(&seat_record), sizeof(SeatRecord));
}

SeatRecord SeatManager::read_record(const int &seat_no) {
    file.seekg(seat_no * sizeof(SeatRecord));
    SeatRecord seat_record = SeatRecord();
    file.read(reinterpret_cast<char*>(&seat_record), sizeof(SeatRecord));
    return seat_record;
}

void SeatManager::add_seats(const int &train_no, const int &date, const int &seat_num) {
    SeatRecord seat_record;
    seat_record.train_no = train_no;
    seat_record.date = date;
    for (int i = 0; i < STATION_NUM - 1; ++i) {
        seat_record.seats[i] = seat_num;
    }
    seat_record.create_seat_no();
    write_record(seat_record);
    data.insert_value(pii(train_no, date), seat_record.seat_no);
}

int SeatManager::get_seats(const int &train_no, const int &date, const int &l, const int &r) {
    int seat_no = data.find_value(pii(train_no, date));
    if (seat_no <= 0) {
        assert(0);
    }
    SeatRecord seat_record = read_record(seat_no);
    int ans = 1e9;
    for (int i = l; i < r; ++i) {
        ans = std::min(ans, seat_record.seats[i]);
    }
    return ans;
}

std::vector<int> SeatManager::get_all_seats(const int &train_no, const int &date) {
    std::vector<int> ans;
    int seat_no = data.find_value(pii(train_no, date));
    if (seat_no <= 0) {
        assert(0);
    }
    SeatRecord seat_record = read_record(seat_no);
    for (int i = 0; i < STATION_NUM - 1; ++i) {
        ans.push_back(seat_record.seats[i]);
    }
    return ans;
}

int SeatManager::buy_seats(const int &train_no, const int &date, const int &l, const int &r, const int &tickets) {
    if (tickets < 1 || l >= r) {
        return -1;
    }
    int seat_no = data.find_value(pii(train_no, date));
    if (seat_no <= 0) {
        return -1;
    }
    SeatRecord seat_record = read_record(seat_no);
    int min = 1e9;
    for (int i = l; i < r; ++i) {
        min = std::min(min, seat_record.seats[i]);
    }
    if (min < tickets) {
        return -1;
    }
    for (int i = l; i < r; ++i) {
        seat_record.seats[i] -= tickets;
    }
    write_record(seat_record);
    return 0;
}

int SeatManager::refund_seats(const int &train_no, const int &date, const int &l, const int &r, const int &tickets) {
    if (tickets < 1 || l >= r) {
        return -1;
    }
    int seat_no = data.find_value(pii(train_no, date));
    if (seat_no <= 0) {
        return -1;
    }
    SeatRecord seat_record = read_record(seat_no);
    for (int i = l; i < r; ++i) {
        seat_record.seats[i] += tickets;
    }
    write_record(seat_record);
    return 0;
}