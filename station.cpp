#include "station.h"
#include "file.h"

int StationRecord::station_count = 0;

StationRecord::StationRecord() {
    memset(station_name, 0, sizeof(station_name));
    station_no = -1;
}

void StationRecord::create_station_no() {
    station_count += 1;
    this -> station_no = station_count;
    meta_manager.sync_from_static();
}

StationManager::StationManager() : data("station_manager_data") {
}

void StationManager::open_file() {
    file.open(station_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(station_file_name, std::ios::out | std::ios::binary); // create a new file
        file.close();
        file.open(station_file_name, std::ios::in | std::ios::out | std::ios::binary);
    }
}

void StationManager::close_file() {
    file.close();
}

void StationManager::write_record(const StationRecord &station_record) {
    file.seekp(station_record.station_no * sizeof(StationRecord));
    file.write(reinterpret_cast<const char*>(&station_record), sizeof(StationRecord));
}

StationRecord StationManager::read_record(const int &station_no) {
    file.seekg(station_no * sizeof(StationRecord));
    StationRecord station_record = StationRecord();
    file.read(reinterpret_cast<char*>(&station_record), sizeof(StationRecord));
    return station_record;
}

int StationManager::add_station(StationRecord &station_record) {
    int this_station_no = data.find_value(station_record.station_name);
    if (this_station_no > 0) {
        return -1;
    }
    station_record.create_station_no();
    write_record(station_record);
    data.insert_value(station_record.station_name, station_record.station_no);
    return 0;
}

int StationManager::query_station_no(const std::string& station_name) {
    return data.find_value(station_name);
}

std::string StationManager::query_station_name(const int &station_no) {
    return read_record(station_no).station_name;
}
