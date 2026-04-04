#pragma once
#include <set>
#include <string>
#include <fstream>
#include "info.h"
#include "bplustree.h"

extern std::set<std::string> logined_user;

struct UserRecord {
    static int user_count;
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    char name[NAME_LEN];
    char mailAddr[MAILADDR_LEN];
    int privilege;
    int user_no;
    UserRecord();
    UserRecord(const std::string &username, const std::string &password, const std::string &name, const std::string &mailAddr, const int &privilege);
    void create_user_no();
};

struct UserManager {
    static bool first;
    const char* user_file_name = "data/user_file.txt";;
    std::fstream file;
    Bplustree<std::string, int> data;
    UserManager();
    void open_file();
    void close_file();
    void write_record(const UserRecord& user_record);
    UserRecord read_record(const int &user_no);
    int query_user_no(const std::string& username);
    int add_user(const std::string &cur_username, UserRecord &user_record);
    int login_user(const std::string &username, const std::string &password);
    int logout_user(const std::string &username);
    void logout_all_user();
    std::string query_profile(const std::string &cur_username, const std::string &username);
    std::string modify_profile(const std::string &cur_username, const UserRecord &user_record);
};

extern UserManager user_manager;