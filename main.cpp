#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include "info.h"
#include "file.h"
#include "user.h"
#include "station.h"
#include "train.h"
#include "seat.h"
#include "order.h"

std::vector<std::string> separateBy(std::string &str, char c) {
    std::vector<std::string> processed_list_of_strings;
    for (int i = 0, j; i < str.size(); i = j + 1) {
        j = i;
        if (str[i] == c) {
            continue;
        }
        while (j + 1 < str.size() && str[j + 1] != c) {
            ++j;
        }
        processed_list_of_strings.push_back(str.substr(i, j - i + 1));
    }
    return processed_list_of_strings;
}

pii string_to_date(std::string &str) {
    return pii((str[0] - '0') * 10 + str[1] - '0', (str[3] - '0') * 10 + str[4] - '0');
}

pii string_to_time(std::string &str) {
    return pii((str[0] - '0') * 10 + str[1] - '0', (str[3] - '0') * 10 + str[4] - '0');
}

void print_query_train(std::vector<QueryTrainReturn> &query_train_return) {
    if (query_train_return.size() == 0) {
        std::cout << "-1\n";
        return;
    }
    for (int i = 0; i < query_train_return.size(); ++i) {
        std::cout << query_train_return[i].station_name << " ";
        if (i == 0) {
            std::cout << "xx-xx xx:xx -> ";
        } else {
            std::string arrive_date = utils::int_to_string(query_train_return[i].arrive_date.first) + "-" + 
                utils::int_to_string(query_train_return[i].arrive_date.second);
            std::string arrive_time = utils::int_to_string(query_train_return[i].arrive_time.first) + ":" + 
                utils::int_to_string(query_train_return[i].arrive_time.second);
            std::cout << arrive_date << " " << arrive_time << " -> ";
        }
        if (i == query_train_return.size() - 1) {
            std::cout << "xx-xx xx:xx ";
        } else {
            std::string depart_date = utils::int_to_string(query_train_return[i].depart_date.first) + "-" + 
                utils::int_to_string(query_train_return[i].depart_date.second);
            std::string depart_time = utils::int_to_string(query_train_return[i].depart_time.first) + ":" + 
                utils::int_to_string(query_train_return[i].depart_time.second);
            std::cout << depart_date << " " << depart_time << " ";
        }
        std::cout << query_train_return[i].price << " ";
        if (i == query_train_return.size() - 1) {
            std::cout << "x\n";
        } else {
            std::cout << query_train_return[i].seat << "\n";
        }
    }
}

void print_query_ticket(std::vector<QueryTicketReturn> &query_ticket_return) {
    std::cout << query_ticket_return.size() << "\n";
    for (int i = 0; i < query_ticket_return.size(); ++i) {
        std::string depart_date = utils::int_to_string(query_ticket_return[i].depart_date.first) + "-" + 
            utils::int_to_string(query_ticket_return[i].depart_date.second);
        std::string depart_time = utils::int_to_string(query_ticket_return[i].depart_time.first) + ":" + 
            utils::int_to_string(query_ticket_return[i].depart_time.second);
        std::string arrive_date = utils::int_to_string(query_ticket_return[i].arrive_date.first) + "-" + 
            utils::int_to_string(query_ticket_return[i].arrive_date.second);
        std::string arrive_time = utils::int_to_string(query_ticket_return[i].arrive_time.first) + ":" + 
            utils::int_to_string(query_ticket_return[i].arrive_time.second);
        std::cout << query_ticket_return[i].train_id << " " << query_ticket_return[i].depart_station_name << " " 
            << depart_date << " " << depart_time << " -> " << query_ticket_return[i].arrive_station_name << " " 
            << arrive_date << " " << arrive_time << " " << query_ticket_return[i].price << " " 
            << query_ticket_return[i].seat << "\n";
    }
}

void print_query_order(std::vector<OrderRecord> &query_order_return) {
    if (query_order_return.size() == 1 && query_order_return[0].order_no == -1) {
        std::cout << "-1\n";
        return ;
    }
    std::cout << query_order_return.size() << "\n";
    for (int i = 0; i < query_order_return.size(); ++i) {
        std::string status, train_id, from, to;
        OrderRecord &this_order_record = query_order_return[i];
        if (this_order_record.status == 0) {
            status = "success";
        } else if (this_order_record.status == 1) {
            status = "pending";
        } else {
            status = "refunded";
        }
        train_id = train_manager.query_train_id(this_order_record.train_no);
        from = station_manager.query_station_name(this_order_record.depart_station_no);
        to = station_manager.query_station_name(this_order_record.arrive_station_no);
        std::string depart_date = utils::int_to_string(utils::int_to_date(this_order_record.depart_date).first) + "-" + 
            utils::int_to_string(utils::int_to_date(this_order_record.depart_date).second);
        std::string depart_time = utils::int_to_string(utils::int_to_time(this_order_record.depart_time).first) + ":" + 
            utils::int_to_string(utils::int_to_time(this_order_record.depart_time).second);
        std::string arrive_date = utils::int_to_string(utils::int_to_date(this_order_record.arrive_date).first) + "-" + 
            utils::int_to_string(utils::int_to_date(this_order_record.arrive_date).second);
        std::string arrive_time = utils::int_to_string(utils::int_to_time(this_order_record.arrive_time).first) + ":" + 
            utils::int_to_string(utils::int_to_time(this_order_record.arrive_time).second);
        std::cout << "[" << status << "] " << train_id << " " << from << " " << depart_date
            << " " << depart_time << " -> " << to << " " << arrive_date << " " << arrive_time 
            << " " << this_order_record.price << " " << this_order_record.seat << "\n";
    }
}

UserManager user_manager;
TrainManager train_manager;
OrderManager order_manager;

void set_up() {
    system("mkdir data >nul 2>nul");
    meta_manager.open_file();
    meta_manager.load();
    user_manager.open_file();
    station_manager.open_file();
    seat_manager.open_file();
    train_manager.open_file();
    order_manager.open_file();
}

bool execute(std::string &instruction) {
    std::vector<std::string> parts = separateBy(instruction, ' ');
    if (parts.size() < 2) {
        throw std::runtime_error("Invalid instruction!"); 
    }
    std::cout << parts[0] << " ";
    if (parts.size() == 2 && parts[1] == "exit") {
        // TO DO: Logout everyone
        user_manager.logout_all_user();
        std::cout << "bye\n";
        return false;
    } else if (parts.size() == 2 && parts[1] == "clean") {
        // TO DO: Clean all the data
    } else if (parts[1] == "add_user") {
        std::string c, u, p, n, m; 
        int g;
        for (int i = 2; i < parts.size(); i += 2) {
            if (parts[i] == "-c") {
                c = parts[i + 1];
            } else if (parts[i] == "-u") {
                u = parts[i + 1];
            } else if (parts[i] == "-p") {
                p = parts[i + 1];
            } else if (parts[i] == "-n") {
                n = parts[i + 1];
            } else if (parts[i] == "-m") {
                m = parts[i + 1];
            } else {
                g = utils::string_to_int(parts[i + 1]);
            }
        }
        UserRecord user_record(u, p, n, m, g);
        std::cout << user_manager.add_user(c, user_record) << "\n";
    } else if (parts[1] == "login") {
        std::string u, p;
        for (int i = 2; i < parts.size(); i += 2) {
            if (parts[i] == "-u") {
                u = parts[i + 1];
            } else {
                p = parts[i + 1];
            }
        }
        std::cout << user_manager.login_user(u, p) << "\n";
    } else if (parts[1] == "logout") {
        std::string u = parts[3];
        std::cout << user_manager.logout_user(u) << "\n";
    } else if (parts[1] == "query_profile") {
        std::string c, u;
        for (int i = 2; i < parts.size(); i += 2) {
            if (parts[i] == "-c") {
                c = parts[i + 1];
            } else {
                u = parts[i + 1];
            }
        }
        std::cout << user_manager.query_profile(c, u) << "\n";
    } else if (parts[1] == "modify_profile") {
        std::string c, u, p, n, m; 
        int g;
        for (int i = 2; i < parts.size(); i += 2) {
            if (parts[i] == "-c") {
                c = parts[i + 1];
            } else if (parts[i] == "-u") {
                u = parts[i + 1];
            } else if (parts[i] == "-p") {
                p = parts[i + 1];
            } else if (parts[i] == "-n") {
                n = parts[i + 1];
            } else if (parts[i] == "-m") {
                m = parts[i + 1];
            } else {
                g = utils::string_to_int(parts[i + 1]);
            }
        }
        UserRecord user_record(u, p, n, m, g);
        std::cout << user_manager.modify_profile(c, user_record) << "\n";
    } else if (parts[1] == "add_train") {
        std::string i, x, d, y;
        int n = 0, m = 0;
        std::vector<std::string> s, p, t, o;
        for (int j = 2; j < parts.size(); j += 2) {
            if (parts[j] == "-i") {
                i = parts[j + 1];
            } else if (parts[j] == "-n") {
                n = utils::string_to_int(parts[j + 1]);
            } else if (parts[j] == "-m") {
                m = utils::string_to_int(parts[j + 1]);
            } else if (parts[j] == "-s") {
                s = separateBy(parts[j + 1], '|');
            } else if (parts[j] == "-p") {
                p = separateBy(parts[j + 1], '|');
            } else if (parts[j] == "-x") {
                x = parts[j + 1];
            } else if (parts[j] == "-t") {
                t = separateBy(parts[j + 1], '|');
            } else if (parts[j] == "-o") {
                o = separateBy(parts[j + 1], '|');
            } else if (parts[j] == "-d") {
                d = parts[j + 1];
            } else {
                y = parts[j + 1];
            }
        }
        std::string station_name[STATION_NUM];
        int prices[STATION_NUM], travel_times[STATION_NUM], stop_times[STATION_NUM];
        for (int j = 0; j < s.size(); ++j) {
            station_name[j] = s[j];
        }
        for (int j = 0; j < p.size(); ++j) {
            prices[j] = utils::string_to_int(p[j]);
        }
        for (int j = 0; j < t.size(); ++j) {
            travel_times[j] = utils::string_to_int(t[j]);
        }
        for (int j = 0; j < o.size(); ++j) {
            stop_times[j] = utils::string_to_int(o[j]);
        }
        TrainRecord train_record(i, n, station_name, m, prices, x, travel_times, stop_times, d);
        train_record.type = y[0];
        std::cout << train_manager.add_train(train_record) << "\n";
    } else if (parts[1] == "delete_train") {
        std::string i = parts[3];
        std::cout << train_manager.delete_train(i) << "\n";
    } else if (parts[1] == "release_train") {
        std::string i = parts[3];
        std::cout << train_manager.release_train(i) << "\n";
    } else if (parts[1] == "query_train") {
        std::string i, d;
        for (int j = 2; j < parts.size(); j += 2) {
            if (parts[j] == "-i") {
                i = parts[j + 1];
            } else {
                d = parts[j + 1];
            }
        }
        pii date = string_to_date(d);
        std::vector<QueryTrainReturn> query_train_return = train_manager.query_train(i, date);
        print_query_train(query_train_return);
    } else if (parts[1] == "query_ticket") {
        std::string s, t, d, p = "time";
        for (int j = 2; j < parts.size(); j += 2) {
            if (parts[j] == "-s") {
                s = parts[j + 1];
            } else if (parts[j] == "-t") {
                t = parts[j + 1];
            } else if (parts[j] == "-d") {
                d = parts[j + 1];
            } else {
                p = parts[j + 1];
            }
        }
        pii date = string_to_date(d);
        pii time = pii(0, 0);
        int sorting = 0;
        if (p == "time") {
            sorting = 1;
        }
        std::vector<QueryTicketReturn> query_ticket_return = train_manager.query_ticket(s, t, date, time, 0, sorting);
        print_query_ticket(query_ticket_return);
    } else if (parts[1] == "query_transfer") {
        std::string s, t, d, p = "time";
        for (int j = 2; j < parts.size(); j += 2) {
            if (parts[j] == "-s") {
                s = parts[j + 1];
            } else if (parts[j] == "-t") {
                t = parts[j + 1];
            } else if (parts[j] == "-d") {
                d = parts[j + 1];
            } else {
                p = parts[j + 1];
            }
        }
        pii date = string_to_date(d);
        int sorting = 0;
        if (p == "time") {
            sorting = 1;
        }
        std::vector<QueryTicketReturn> query_transfer_return = train_manager.query_transfer(s, t, date, sorting);
        print_query_ticket(query_transfer_return);
    } else if (parts[1] == "buy_ticket") {
        std::string u, i, d, n, f, t, q = "false";
        for (int j = 2; j < parts.size(); j += 2) {
            if (parts[j] == "-u") {
                u = parts[j + 1];
            } else if (parts[j] == "-i") {
                i = parts[j + 1];
            } else if (parts[j] == "-d") {
                d = parts[j + 1];
            } else if (parts[j] == "-n") {
                n = parts[j + 1];
            } else if (parts[j] == "-f") {
                f = parts[j + 1];
            } else if (parts[j] == "-t") {
                t = parts[j + 1];
            } else {
                q = parts[j + 1];
            }
        }
        pii date = string_to_date(d);
        int tickets = utils::string_to_int(n);
        bool accept_pending = (q == "true") ? true : false;
        std::cout << order_manager.buy_ticket(u, i, date, tickets, f, t, accept_pending) << "\n";
    } else if (parts[1] == "query_order") {
        std::string u;
        for (int j = 2; j < parts.size(); j += 2) {
            if (parts[j] == "-u") {
                u = parts[j + 1];
            }
        }
        std::vector<OrderRecord> query_order_return = order_manager.query_order(u);
        print_query_order(query_order_return);
    } else if (parts[1] == "refund_ticket") {
        std::string u, n;
        for (int j = 2; j < parts.size(); j += 2) {
            if (parts[j] == "-u") {
                u = parts[j + 1];
            } else {
                n = parts[j + 1];
            }
        }
        int index = utils::string_to_int(n);
        std::cout << order_manager.refund_ticket(u, index) << "\n";
    }
    return true;
}

int main() {
    // std::cout << "Welcome to the ticket system!\n";
    set_up();
    while (true) {
        std::string instruction;
        std::getline(std::cin, instruction);
        if (execute(instruction) == false) {
            break;
        }
    }
    return 0;
}
