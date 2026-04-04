#include <iostream>
#include <string>
#include "..\info.h"
#include "..\file.h"
#include "..\user.h"
#include "..\station.h"
#include "..\seat.h"
#include "..\train.h"
#include "..\order.h"
UserManager user_manager;
TrainManager train_manager;
OrderManager order_manager;
int main(){
    system("rmdir /s /q data >nul 2>nul");
    system("mkdir data >nul 2>nul");
    meta_manager.open_file(); meta_manager.load();
    user_manager.open_file(); station_manager.open_file(); seat_manager.open_file(); train_manager.open_file(); order_manager.open_file();
    UserRecord admin("I_am_the_admin","awsl","n","m",10);
    std::cout << user_manager.add_user("cur", admin) << "\n";
    std::cout << user_manager.login_user("I_am_the_admin","awsl") << "\n";
    UserRecord cro("Croissant","p","name","mail",8);
    std::cout << user_manager.add_user("I_am_the_admin", cro) << "\n";
    std::cout << user_manager.login_user("Croissant","p") << "\n";
    UserRecord may("Mayer","old","nm","mm",7);
    std::cout << user_manager.add_user("Croissant", may) << "\n";
    std::cout << user_manager.login_user("Mayer","old") << "\n";
    std::cout << user_manager.query_profile("Croissant","Mayer") << "\n";
    UserRecord mod("Mayer","newpass","","",7);
    std::cout << user_manager.modify_profile("Croissant", mod) << "\n";
    std::cout << user_manager.query_profile("Croissant","Mayer") << "\n";
}
