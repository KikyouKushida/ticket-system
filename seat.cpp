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

void SeatManager::write_record(const SeatRecord& train_record) {
    file.seekp(train_record.train_no * sizeof(SeatRecord));
    file.write(reinterpret_cast<const char*>(&train_record), sizeof(SeatRecord));
}

SeatRecord SeatManager::read_record(const int &seat_no) {
    file.seekg(seat_no * sizeof(SeatRecord));
    SeatRecord seat_record = SeatRecord();
    file.read(reinterpret_cast<char*>(&seat_record), sizeof(SeatRecord));
    return seat_record;
}