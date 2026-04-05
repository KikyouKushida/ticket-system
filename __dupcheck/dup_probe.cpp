#include "bplustree.h"
#include <iostream>
int main() {
    Bplustree<int,int> t("dup_probe");
    for (int i = 1; i <= 700; ++i) t.insert_value(42, i);
    std::vector<int> v = t.find_all_values(42);
    std::cout << v.size() << "\n";
    std::cout << v.front() << " " << v.back() << "\n";
    return 0;
}
