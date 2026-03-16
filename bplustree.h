#pragma once

template <typename T1, typename T2>
struct Node {
    static int node_count;
    int node_type;
};


template <typename T1, typename T2>
struct Bplustree {
    Bplustree();
    Node<T1, T2> root_node;
    bool insert_value(T1 key, T2 data);
    bool erase_value(T1 key);
    T2 find_value(T1 key);
};