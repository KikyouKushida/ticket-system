#include "file.h"

#include "order.h"
#include "seat.h"
#include "station.h"
#include "train.h"
#include "user.h"

MetaManager meta_manager;

namespace {

int scan_count_from_file(const char *file_name, const int record_size) {
    std::ifstream input(file_name, std::ios::binary);
    if (!input) {
        return 0;
    }
    input.seekg(0, std::ios::end);
    std::streamoff file_size = input.tellg();
    if (file_size < record_size) {
        return 0;
    }
    int count = static_cast<int>(file_size / record_size) - 1;
    return count > 0 ? count : 0;
}

MetaData scan_existing_meta() {
    MetaData meta_data{};
    meta_data.user_count = scan_count_from_file("data/user_file.txt", sizeof(UserRecord));
    meta_data.station_count = scan_count_from_file("data/station_file.txt", sizeof(StationRecord));
    meta_data.seat_count = scan_count_from_file("data/seat_file.txt", sizeof(SeatRecord));
    meta_data.train_count = scan_count_from_file("data/train_file.txt", sizeof(TrainRecord));
    meta_data.order_count = scan_count_from_file("data/order_file.txt", sizeof(OrderRecord));
    return meta_data;
}

}  // namespace

MetaManager::MetaManager() {
}

void MetaManager::open_file() {
    file.open(meta_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(meta_file_name, std::ios::out | std::ios::binary);
        MetaData meta_data = scan_existing_meta();
        file.write(reinterpret_cast<const char *>(&meta_data), sizeof(MetaData));
        file.close();
        file.open(meta_file_name, std::ios::in | std::ios::out | std::ios::binary);
    }
}

void MetaManager::close_file() {
    file.close();
}

MetaData MetaManager::read_meta() {
    MetaData meta_data{};
    file.seekg(0);
    file.read(reinterpret_cast<char *>(&meta_data), sizeof(MetaData));
    return meta_data;
}

void MetaManager::write_meta(const MetaData &meta_data) {
    file.seekp(0);
    file.write(reinterpret_cast<const char *>(&meta_data), sizeof(MetaData));
    file.flush();
}

void MetaManager::load() {
    MetaData meta_data = read_meta();
    UserRecord::user_count = meta_data.user_count;
    StationRecord::station_count = meta_data.station_count;
    SeatRecord::seat_count = meta_data.seat_count;
    TrainRecord::train_count = meta_data.train_count;
    OrderRecord::order_count = meta_data.order_count;
}

void MetaManager::sync_from_static() {
    MetaData meta_data;
    meta_data.user_count = UserRecord::user_count;
    meta_data.station_count = StationRecord::station_count;
    meta_data.seat_count = SeatRecord::seat_count;
    meta_data.train_count = TrainRecord::train_count;
    meta_data.order_count = OrderRecord::order_count;
    write_meta(meta_data);
}
