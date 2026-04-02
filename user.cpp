#include "user.h"
#include <cstring>

int UserRecord::user_count = 0;
bool UserManager::first = true;

UserRecord::UserRecord(const std::string &username, const std::string &password, const std::string &name, const std::string &mailAddr, const int &privilege) {
    utils::write_string(this->username, username);
    utils::write_string(this->password, password);
    utils::write_string(this->name, name);
    utils::write_string(this->mailAddr, mailAddr);
    this -> privilege = privilege;
}

UserRecord::UserRecord() {
    memset(username, 0, USERNAME_LEN);
    memset(password, 0, PASSWORD_LEN);
    memset(name, 0, NAME_LEN);
    memset(mailAddr, 0, MAILADDR_LEN);
    privilege = -1;
}

void UserRecord::create_user_no() {
    user_count += 1;
    this -> user_no = user_count;
}

void UserManager::open_file() {
    file.open(user_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(user_file_name, std::ios::out | std::ios::binary); // create a new file
        file.close();
        file.open(user_file_name, std::ios::in | std::ios::out | std::ios::binary);
    }
}

UserManager::UserManager() {
    
}

void UserManager::close_file() {
    file.close();
}

void UserManager::write_record(const UserRecord &user_record) {
    file.seekp(user_record.user_no * sizeof(UserRecord));
    file.write(reinterpret_cast<const char*>(&user_record), sizeof(UserRecord));
}

UserRecord UserManager::read_record(const int &user_no) {
    file.seekg(user_no * sizeof(UserRecord));
    UserRecord user_record = UserRecord();
    file.read(reinterpret_cast<char*>(&user_record), sizeof(UserRecord));
    return user_record;
}

int UserManager::add_user(const std::string &cur_username, UserRecord &user_record) {
    if (first == true) {
        user_record.privilege = 10;
        user_record.create_user_no();
        write_record(user_record);
        data.insert_value(user_record.username, user_record.user_no);
        first = false;
        return 0;
    }
    int cur_user_no = data.find_value(cur_username);
    int this_user_no = data.find_value(user_record.username);
    if (this_user_no > 0) {
        return -1;
    }
    UserRecord cur_user_record = read_record(cur_user_no);
    if (cur_user_record.privilege <= user_record.privilege) {
        return -1;
    }
    user_record.create_user_no();
    write_record(user_record);
    data.insert_value(user_record.username, user_record.user_no);
    return 0;
}

int UserManager::login_user(const std::string &username, const std::string &password) {
    if (logined_user.find(username) != logined_user.end()) {
        return -1;
    }
    int this_user_no = data.find_value(username);
    if (this_user_no == 0) {
        return -1;
    }
    UserRecord this_user_record = read_record(this_user_no);
    if (this_user_record.password != password) {
        return -1;
    }
    logined_user.insert(username);
    return 0;
}

int UserManager::logout_user(const std::string &username) {
    if (logined_user.find(username) != logined_user.end()) {
        logined_user.erase(username);
        return 0;
    }
    return -1;
}

void UserManager::logout_all_user() {
    while (!logined_user.empty()) {
        logined_user.erase(logined_user.begin());
    }
}

std::string UserManager::query_profile(const std::string &cur_username, const std::string &username) {
    if (logined_user.find(cur_username) == logined_user.end()) {
        return "-1";
    }
    int cur_user_no = data.find_value(cur_username);
    if (cur_username == username) {
        UserRecord this_user_record = read_record(cur_user_no);
        std::string answer = this_user_record.username;
        answer += " ";
        answer += this_user_record.name;
        answer += " ";
        answer += this_user_record.mailAddr;
        answer += " ";
        answer += utils::int_to_string(this_user_record.privilege);
        return answer;
    }
    int this_user_no = data.find_value(username);
    if (this_user_no == 0) {
        return "-1";
    }
    UserRecord cur_user_record = read_record(cur_user_no);
    UserRecord this_user_record = read_record(this_user_no);
    if (cur_user_record.privilege <= this_user_record.privilege) {
        return "-1";
    }
    std::string answer = this_user_record.username;
    answer += " ";
    answer += this_user_record.name;
    answer += " ";
    answer += this_user_record.mailAddr;
    answer += " ";
    answer += utils::int_to_string(this_user_record.privilege);
    return answer;
}

std::string UserManager::modify_profile(const std::string &cur_username, const UserRecord &user_record) {
    if (logined_user.find(cur_username) == logined_user.end()) {
        return "-1";
    }
    int cur_user_no = data.find_value(cur_username);
    if (cur_username == user_record.username) {
        UserRecord this_user_record = read_record(cur_user_no);
        if (strlen(user_record.password) > 0) {
            memcpy(this_user_record.password, user_record.password, PASSWORD_LEN);
        }
        if (strlen(user_record.name) > 0) {
            memcpy(this_user_record.name, user_record.name, NAME_LEN);
        }
        if (strlen(user_record.mailAddr) > 0) {
            memcpy(this_user_record.mailAddr, user_record.mailAddr, MAILADDR_LEN);
        }
        if (user_record.privilege != -1) {
            if (user_record.privilege >= this_user_record.privilege) {
                return "-1";
            }
            this_user_record.privilege = user_record.privilege;
        }
        write_record(this_user_record);
        std::string answer = this_user_record.username;
        answer += " ";
        answer += this_user_record.name;
        answer += " ";
        answer += this_user_record.mailAddr;
        answer += " ";
        answer += utils::int_to_string(this_user_record.privilege);
        return answer;
    }
    int this_user_no = data.find_value(user_record.username);
    if (this_user_no == 0) {
        return "-1";
    }
    UserRecord cur_user_record = read_record(cur_user_no);
    UserRecord this_user_record = read_record(this_user_no);
    if (cur_user_record.privilege <= this_user_record.privilege) {
        return "-1";
    }
    if (strlen(user_record.password) > 0) {
        memcpy(this_user_record.password, user_record.password, PASSWORD_LEN);
    }
    if (strlen(user_record.name) > 0) {
        memcpy(this_user_record.name, user_record.name, NAME_LEN);
    }
    if (strlen(user_record.mailAddr) > 0) {
        memcpy(this_user_record.mailAddr, user_record.mailAddr, MAILADDR_LEN);
    }
    if (user_record.privilege != -1) {
        if (user_record.privilege >= this_user_record.privilege) {
            return "-1";
        }
        this_user_record.privilege = user_record.privilege;
    }
    write_record(this_user_record);
    std::string answer = this_user_record.username;
    answer += " ";
    answer += this_user_record.name;
    answer += " ";
    answer += this_user_record.mailAddr;
    answer += " ";
    answer += utils::int_to_string(this_user_record.privilege);
    return answer;
}
