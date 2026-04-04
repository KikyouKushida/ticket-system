#pragma once
#include <fstream>

struct MetaData {
    int user_count;
    int station_count;
    int seat_count;
    int train_count;
    int order_count;
};

struct MetaManager {
    const char *meta_file_name = "data/meta_data.bin";
    std::fstream file;
    MetaManager();
    void open_file();
    void close_file();
    MetaData read_meta();
    void write_meta(const MetaData &meta_data);
    void load();
    void sync_from_static();
    void clear();
};

extern MetaManager meta_manager;
