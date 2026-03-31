#include "station.h"

StationRecord::StationRecord() {
    memset(station_name, 0, sizeof(station_name));
    station_no = -1;
}

void StationRecord::create_station_no() {
    station_count += 1;
    this -> station_no = station_count;
}

StationManager::StationManager() {

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

int StationManager::query_station_no(const std::string& station_name) {
    return data.find_value(station_name);
}

std::string StationManager::query_station_name(const int &station_no) {
    return read_record(station_no).station_name;
}