#pragma once
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "info.h"

template <typename T1, typename T2>
struct Node {
    inline static int node_count = 0;
    int node_type = 0;
};

namespace bpt_detail {

template <typename T>
struct dependent_false : std::false_type {};

template <typename T>
struct KeyCodec {
    static_assert(dependent_false<T>::value, "Unsupported B+ tree key type.");
};

template <>
struct KeyCodec<int> {
    using disk_type = int;

    static int compare(const int &lhs, const int &rhs) {
        if (lhs < rhs) return -1;
        if (rhs < lhs) return 1;
        return 0;
    }

    static disk_type to_disk(const int &value) {
        return value;
    }

    static int from_disk(const disk_type &value) {
        return value;
    }
};

template <>
struct KeyCodec<std::pair<int, int>> {
    struct disk_type {
        int first;
        int second;
    };

    static int compare(const std::pair<int, int> &lhs, const std::pair<int, int> &rhs) {
        if (lhs.first != rhs.first) return lhs.first < rhs.first ? -1 : 1;
        if (lhs.second != rhs.second) return lhs.second < rhs.second ? -1 : 1;
        return 0;
    }

    static disk_type to_disk(const std::pair<int, int> &value) {
        return disk_type{value.first, value.second};
    }

    static std::pair<int, int> from_disk(const disk_type &value) {
        return std::make_pair(value.first, value.second);
    }
};

template <>
struct KeyCodec<std::string> {
    using disk_type = std::array<char, 64>;

    static int compare(const std::string &lhs, const std::string &rhs) {
        if (lhs < rhs) return -1;
        if (rhs < lhs) return 1;
        return 0;
    }

    static disk_type to_disk(const std::string &value) {
        disk_type result{};
        std::size_t len = value.size();
        if (len >= result.size()) len = result.size() - 1;
        std::memcpy(result.data(), value.data(), len);
        result[len] = '\0';
        return result;
    }

    static std::string from_disk(const disk_type &value) {
        return std::string(value.data());
    }
};

template <typename T>
struct ValueCodec {
    using disk_type = T;

    static disk_type to_disk(const T &value) {
        return value;
    }

    static T from_disk(const disk_type &value) {
        return value;
    }
};

template <typename T>
int compare_disk_value(const typename KeyCodec<T>::disk_type &lhs, const T &rhs) {
    return KeyCodec<T>::compare(KeyCodec<T>::from_disk(lhs), rhs);
}

template <typename T>
int compare_disk_disk(const typename KeyCodec<T>::disk_type &lhs, const typename KeyCodec<T>::disk_type &rhs) {
    return KeyCodec<T>::compare(KeyCodec<T>::from_disk(lhs), KeyCodec<T>::from_disk(rhs));
}

template <typename Key, typename Value>
struct FileName;

template <>
struct FileName<std::string, int> {
    static std::string get(int id) {
        return "data/bplustree_string_int_" + std::to_string(id) + ".dat";
    }
};

template <>
struct FileName<int, int> {
    static std::string get(int id) {
        return "data/bplustree_int_int_" + std::to_string(id) + ".dat";
    }
};

template <>
struct FileName<std::pair<int, int>, int> {
    static std::string get(int id) {
        return "data/bplustree_pii_int_" + std::to_string(id) + ".dat";
    }
};

inline std::string sanitize_file_token(const std::string &name) {
    std::string result;
    for (std::size_t i = 0; i < name.size(); ++i) {
        char ch = name[i];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
            || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-') {
            result.push_back(ch);
        } else {
            result.push_back('_');
        }
    }
    if (result.empty()) {
        result = "default";
    }
    return result;
}

template <typename Key, typename Value>
std::string named_tree_file(const std::string &tree_name) {
    return FileName<Key, Value>::get(0) + "." + sanitize_file_token(tree_name);
}

template <typename Key, typename Value>
int next_tree_id() {
    static int counter = 0;
    return counter++;
}

struct MetaPage {
    int root_page;
    int page_count;
    char padding[PAGE_SIZE - static_cast<int>(2 * sizeof(int))];
};

template <typename DiskKey, typename DiskValue>
struct PageLayout {
    struct Header {
        int is_leaf;
        int size;
        int parent;
        int next;
    };

    static constexpr int leaf_capacity =
        (PAGE_SIZE - static_cast<int>(sizeof(Header))) /
        static_cast<int>(sizeof(DiskKey) + sizeof(DiskValue));

    static constexpr int internal_capacity =
        (PAGE_SIZE - static_cast<int>(sizeof(Header)) - static_cast<int>(sizeof(int))) /
        static_cast<int>(sizeof(DiskKey) + sizeof(int));

    struct LeafPage {
        Header header;
        DiskKey keys[leaf_capacity];
        DiskValue values[leaf_capacity];
        char padding[PAGE_SIZE - static_cast<int>(sizeof(Header))
                     - leaf_capacity * static_cast<int>(sizeof(DiskKey))
                     - leaf_capacity * static_cast<int>(sizeof(DiskValue))];
    };

    struct InternalPage {
        Header header;
        DiskKey keys[internal_capacity];
        int children[internal_capacity + 1];
        char padding[PAGE_SIZE - static_cast<int>(sizeof(Header))
                     - internal_capacity * static_cast<int>(sizeof(DiskKey))
                     - (internal_capacity + 1) * static_cast<int>(sizeof(int))];
    };
};

template <typename Key, typename Value>
class DiskBPlusTree {
public:
    using KeyCodecType = KeyCodec<Key>;
    using ValueCodecType = ValueCodec<Value>;
    using DiskKey = typename KeyCodecType::disk_type;
    using DiskValue = typename ValueCodecType::disk_type;
    using Layout = PageLayout<DiskKey, DiskValue>;
    using Header = typename Layout::Header;
    using LeafPage = typename Layout::LeafPage;
    using InternalPage = typename Layout::InternalPage;

    static_assert(sizeof(LeafPage) == PAGE_SIZE, "Leaf page must occupy exactly one 4KiB page.");
    static_assert(sizeof(InternalPage) == PAGE_SIZE, "Internal page must occupy exactly one 4KiB page.");

    explicit DiskBPlusTree(const std::string &path) : path_(path) {
        open_file();
    }

    bool insert(const Key &key, const Value &value) {
        DiskKey disk_key = KeyCodecType::to_disk(key);
        DiskValue disk_value = ValueCodecType::to_disk(value);
        int leaf_id = find_leaf_page(key);
        LeafPage leaf = read_leaf(leaf_id);
        int pos = leaf_upper_bound(leaf, key, value);
        if (leaf.header.size < Layout::leaf_capacity) {
            insert_into_leaf(leaf, pos, disk_key, disk_value);
            write_leaf(leaf_id, leaf);
            if (pos == 0) update_parent_key_after_first_key_change(leaf_id);
            return true;
        }
        split_leaf_with_insert(leaf_id, leaf, pos, disk_key, disk_value);
        return true;
    }

    bool erase_all(const Key &key) {
        bool erased = false;
        while (erase_one(key)) {
            erased = true;
        }
        return erased;
    }

    bool erase_pair(const Key &key, const Value &value) {
        int leaf_id = find_leftmost_leaf_page(key);
        while (leaf_id != 0) {
            LeafPage leaf = read_leaf(leaf_id);
            int pos = leaf_lower_bound(leaf, key);
            while (pos < leaf.header.size && compare_disk_value<Key>(leaf.keys[pos], key) == 0) {
                if (ValueCodecType::from_disk(leaf.values[pos]) == value) {
                    erase_from_leaf(leaf_id, leaf, pos);
                    return true;
                }
                ++pos;
            }
            if (leaf.header.next == 0) break;
            LeafPage next_leaf = read_leaf(leaf.header.next);
            if (next_leaf.header.size == 0 || compare_disk_value<Key>(next_leaf.keys[0], key) > 0) break;
            leaf_id = leaf.header.next;
        }
        return false;
    }

    Value find_one(const Key &key) {
        int leaf_id = find_leftmost_leaf_page(key);
        while (leaf_id != 0) {
            LeafPage leaf = read_leaf(leaf_id);
            int pos = leaf_lower_bound(leaf, key);
            if (pos < leaf.header.size && compare_disk_value<Key>(leaf.keys[pos], key) == 0) {
                return ValueCodecType::from_disk(leaf.values[pos]);
            }
            if (leaf.header.next == 0) break;
            LeafPage next_leaf = read_leaf(leaf.header.next);
            if (next_leaf.header.size == 0 || compare_disk_value<Key>(next_leaf.keys[0], key) > 0) break;
            leaf_id = leaf.header.next;
        }
        return Value();
    }

    std::vector<Value> find_all(const Key &key) {
        std::vector<Value> result;
        int leaf_id = find_leftmost_leaf_page(key);
        while (leaf_id != 0) {
            LeafPage leaf = read_leaf(leaf_id);
            int pos = leaf_lower_bound(leaf, key);
            for (int i = pos; i < leaf.header.size; ++i) {
                if (compare_disk_value<Key>(leaf.keys[i], key) != 0) {
                    return result;
                }
                result.push_back(ValueCodecType::from_disk(leaf.values[i]));
            }
            if (leaf.header.next == 0) break;
            LeafPage next_leaf = read_leaf(leaf.header.next);
            if (next_leaf.header.size == 0 || compare_disk_value<Key>(next_leaf.keys[0], key) > 0) break;
            leaf_id = leaf.header.next;
        }
        return result;
    }

private:
    std::string path_;
    std::fstream file_;

    void open_file() {
        file_.open(path_.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        if (!file_.good()) {
            file_.clear();
            std::ofstream create(path_.c_str(), std::ios::binary);
            MetaPage meta{};
            meta.root_page = 1;
            meta.page_count = 2;
            create.write(reinterpret_cast<const char *>(&meta), sizeof(meta));
            LeafPage root{};
            root.header.is_leaf = 1;
            root.header.size = 0;
            root.header.parent = 0;
            root.header.next = 0;
            create.write(reinterpret_cast<const char *>(&root), sizeof(root));
            create.close();
            file_.open(path_.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        }
    }

    MetaPage read_meta() {
        MetaPage meta{};
        file_.seekg(0);
        file_.read(reinterpret_cast<char *>(&meta), sizeof(meta));
        return meta;
    }

    void write_meta(const MetaPage &meta) {
        file_.seekp(0);
        file_.write(reinterpret_cast<const char *>(&meta), sizeof(meta));
        file_.flush();
    }

    template <typename Page>
    Page read_page(int page_id) {
        Page page{};
        file_.seekg(static_cast<std::streamoff>(page_id) * PAGE_SIZE);
        file_.read(reinterpret_cast<char *>(&page), sizeof(page));
        return page;
    }

    template <typename Page>
    void write_page(int page_id, const Page &page) {
        file_.seekp(static_cast<std::streamoff>(page_id) * PAGE_SIZE);
        file_.write(reinterpret_cast<const char *>(&page), sizeof(page));
        file_.flush();
    }

    Header read_header(int page_id) {
        Header header{};
        file_.seekg(static_cast<std::streamoff>(page_id) * PAGE_SIZE);
        file_.read(reinterpret_cast<char *>(&header), sizeof(header));
        return header;
    }

    LeafPage read_leaf(int page_id) {
        return read_page<LeafPage>(page_id);
    }

    InternalPage read_internal(int page_id) {
        return read_page<InternalPage>(page_id);
    }

    void write_leaf(int page_id, const LeafPage &page) {
        write_page(page_id, page);
    }

    void write_internal(int page_id, const InternalPage &page) {
        write_page(page_id, page);
    }

    int allocate_leaf() {
        MetaPage meta = read_meta();
        int page_id = meta.page_count++;
        write_meta(meta);
        LeafPage page{};
        page.header.is_leaf = 1;
        page.header.size = 0;
        page.header.parent = 0;
        page.header.next = 0;
        write_leaf(page_id, page);
        return page_id;
    }

    int allocate_internal() {
        MetaPage meta = read_meta();
        int page_id = meta.page_count++;
        write_meta(meta);
        InternalPage page{};
        page.header.is_leaf = 0;
        page.header.size = 0;
        page.header.parent = 0;
        page.header.next = 0;
        write_internal(page_id, page);
        return page_id;
    }

    int root_page() {
        return read_meta().root_page;
    }

    void set_root_page(int page_id) {
        MetaPage meta = read_meta();
        meta.root_page = page_id;
        write_meta(meta);
    }

    int find_leaf_page(const Key &key) {
        int page_id = root_page();
        while (true) {
            Header header = read_header(page_id);
            if (header.is_leaf) return page_id;
            InternalPage page = read_internal(page_id);
            int idx = internal_child_index(page, key);
            page_id = page.children[idx];
        }
    }

    int find_leftmost_leaf_page(const Key &key) {
        int page_id = root_page();
        while (true) {
            Header header = read_header(page_id);
            if (header.is_leaf) return page_id;
            InternalPage page = read_internal(page_id);
            int idx = internal_child_index_leftmost(page, key);
            page_id = page.children[idx];
        }
    }

    int leaf_lower_bound(const LeafPage &page, const Key &key) const {
        int l = 0, r = page.header.size;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (compare_disk_value<Key>(page.keys[mid], key) < 0) {
                l = mid + 1;
            } else {
                r = mid;
            }
        }
        return l;
    }

    int leaf_upper_bound(const LeafPage &page, const Key &key, const Value &value) const {
        int pos = leaf_lower_bound(page, key);
        while (pos < page.header.size && compare_disk_value<Key>(page.keys[pos], key) == 0
               && !(value < ValueCodecType::from_disk(page.values[pos]))) {
            ++pos;
        }
        return pos;
    }

    int internal_child_index(const InternalPage &page, const Key &key) const {
        int idx = 0;
        while (idx < page.header.size && compare_disk_value<Key>(page.keys[idx], key) <= 0) {
            ++idx;
        }
        return idx;
    }

    int internal_child_index_leftmost(const InternalPage &page, const Key &key) const {
        int idx = 0;
        while (idx < page.header.size && compare_disk_value<Key>(page.keys[idx], key) < 0) {
            ++idx;
        }
        return idx;
    }

    void insert_into_leaf(LeafPage &page, int pos, const DiskKey &key, const DiskValue &value) {
        for (int i = page.header.size; i > pos; --i) {
            page.keys[i] = page.keys[i - 1];
            page.values[i] = page.values[i - 1];
        }
        page.keys[pos] = key;
        page.values[pos] = value;
        ++page.header.size;
    }

    void split_leaf_with_insert(int leaf_id, LeafPage &leaf, int insert_pos, const DiskKey &insert_key, const DiskValue &insert_value) {
        const int total = leaf.header.size + 1;
        std::vector<DiskKey> all_keys(total);
        std::vector<DiskValue> all_values(total);
        for (int i = 0, j = 0; i < total; ++i) {
            if (i == insert_pos) {
                all_keys[i] = insert_key;
                all_values[i] = insert_value;
            } else {
                all_keys[i] = leaf.keys[j];
                all_values[i] = leaf.values[j];
                ++j;
            }
        }

        int right_id = allocate_leaf();
        LeafPage right = read_leaf(right_id);
        right.header.parent = leaf.header.parent;
        right.header.next = leaf.header.next;
        int left_size = total / 2;
        int right_size = total - left_size;
        for (int i = 0; i < left_size; ++i) {
            leaf.keys[i] = all_keys[i];
            leaf.values[i] = all_values[i];
        }
        for (int i = 0; i < right_size; ++i) {
            right.keys[i] = all_keys[left_size + i];
            right.values[i] = all_values[left_size + i];
        }
        leaf.header.size = left_size;
        right.header.size = right_size;
        leaf.header.next = right_id;
        write_leaf(leaf_id, leaf);
        write_leaf(right_id, right);
        insert_into_parent(leaf_id, KeyCodecType::from_disk(right.keys[0]), right_id);
    }

    void insert_into_parent(int left_id, const Key &separator, int right_id) {
        Header left_header = read_header(left_id);
        if (left_header.parent == 0) {
            int root_id = allocate_internal();
            InternalPage root = read_internal(root_id);
            root.header.size = 1;
            root.children[0] = left_id;
            root.children[1] = right_id;
            root.keys[0] = KeyCodecType::to_disk(separator);
            write_internal(root_id, root);
            set_parent(left_id, root_id);
            set_parent(right_id, root_id);
            set_root_page(root_id);
            return;
        }

        int parent_id = left_header.parent;
        InternalPage parent = read_internal(parent_id);
        int child_pos = 0;
        while (child_pos <= parent.header.size && parent.children[child_pos] != left_id) {
            ++child_pos;
        }
        if (parent.header.size < Layout::internal_capacity) {
            for (int i = parent.header.size; i > child_pos; --i) {
                parent.keys[i] = parent.keys[i - 1];
            }
            for (int i = parent.header.size + 1; i > child_pos + 1; --i) {
                parent.children[i] = parent.children[i - 1];
            }
            parent.keys[child_pos] = KeyCodecType::to_disk(separator);
            parent.children[child_pos + 1] = right_id;
            ++parent.header.size;
            set_parent(right_id, parent_id);
            write_internal(parent_id, parent);
            return;
        }
        split_internal_with_insert(parent_id, parent, child_pos, KeyCodecType::to_disk(separator), right_id);
    }

    void split_internal_with_insert(int page_id, InternalPage &page, int insert_pos, const DiskKey &insert_key, int right_child_id) {
        const int total_keys = page.header.size + 1;
        std::vector<DiskKey> all_keys(total_keys);
        std::vector<int> all_children(total_keys + 1);
        for (int i = 0; i < insert_pos; ++i) all_keys[i] = page.keys[i];
        all_keys[insert_pos] = insert_key;
        for (int i = insert_pos; i < page.header.size; ++i) all_keys[i + 1] = page.keys[i];
        for (int i = 0; i <= insert_pos; ++i) all_children[i] = page.children[i];
        all_children[insert_pos + 1] = right_child_id;
        for (int i = insert_pos + 1; i <= page.header.size; ++i) all_children[i + 1] = page.children[i];

        int right_id = allocate_internal();
        InternalPage right = read_internal(right_id);
        right.header.parent = page.header.parent;
        int mid = total_keys / 2;
        DiskKey promote_key = all_keys[mid];
        int right_key_count = total_keys - mid - 1;

        page.header.size = mid;
        for (int i = 0; i < mid; ++i) page.keys[i] = all_keys[i];
        for (int i = 0; i <= mid; ++i) page.children[i] = all_children[i];

        right.header.size = right_key_count;
        for (int i = 0; i < right_key_count; ++i) right.keys[i] = all_keys[mid + 1 + i];
        for (int i = 0; i <= right_key_count; ++i) {
            right.children[i] = all_children[mid + 1 + i];
            set_parent(right.children[i], right_id);
        }

        write_internal(page_id, page);
        write_internal(right_id, right);
        insert_into_parent(page_id, KeyCodecType::from_disk(promote_key), right_id);
    }

    void set_parent(int page_id, int parent_id) {
        Header header = read_header(page_id);
        if (header.is_leaf) {
            LeafPage page = read_leaf(page_id);
            page.header.parent = parent_id;
            write_leaf(page_id, page);
        } else {
            InternalPage page = read_internal(page_id);
            page.header.parent = parent_id;
            write_internal(page_id, page);
        }
    }

    int min_leaf_size() const {
        return (Layout::leaf_capacity + 1) / 2;
    }

    int min_internal_size() const {
        return Layout::internal_capacity / 2;
    }

    bool erase_one(const Key &key) {
        int leaf_id = find_leftmost_leaf_page(key);
        while (leaf_id != 0) {
            LeafPage leaf = read_leaf(leaf_id);
            int pos = leaf_lower_bound(leaf, key);
            if (pos < leaf.header.size && compare_disk_value<Key>(leaf.keys[pos], key) == 0) {
                erase_from_leaf(leaf_id, leaf, pos);
                return true;
            }
            if (leaf.header.next == 0) {
                return false;
            }
            LeafPage next_leaf = read_leaf(leaf.header.next);
            if (next_leaf.header.size == 0 || compare_disk_value<Key>(next_leaf.keys[0], key) > 0) {
                return false;
            }
            leaf_id = leaf.header.next;
        }
        return false;
    }

    void erase_from_leaf(int leaf_id, LeafPage &leaf, int pos) {
        for (int i = pos; i + 1 < leaf.header.size; ++i) {
            leaf.keys[i] = leaf.keys[i + 1];
            leaf.values[i] = leaf.values[i + 1];
        }
        --leaf.header.size;
        write_leaf(leaf_id, leaf);
        if (leaf_id == root_page()) return;
        if (leaf.header.size >= min_leaf_size()) {
            if (pos == 0 && leaf.header.size > 0) update_parent_key_after_first_key_change(leaf_id);
            return;
        }
        rebalance_leaf(leaf_id, leaf);
    }

    void update_parent_key_after_first_key_change(int page_id) {
        Header header = read_header(page_id);
        if (header.parent == 0) return;
        InternalPage parent = read_internal(header.parent);
        int idx = 0;
        while (idx <= parent.header.size && parent.children[idx] != page_id) ++idx;
        if (idx > 0) {
            LeafPage leaf = read_leaf(page_id);
            if (leaf.header.size > 0) {
                parent.keys[idx - 1] = leaf.keys[0];
                write_internal(header.parent, parent);
            }
        }
    }

    void rebalance_leaf(int leaf_id, LeafPage leaf) {
        int parent_id = leaf.header.parent;
        InternalPage parent = read_internal(parent_id);
        int pos = 0;
        while (pos <= parent.header.size && parent.children[pos] != leaf_id) ++pos;

        if (pos > 0) {
            int left_id = parent.children[pos - 1];
            LeafPage left = read_leaf(left_id);
            if (left.header.parent == parent_id && left.header.size > min_leaf_size()) {
                for (int i = leaf.header.size; i > 0; --i) {
                    leaf.keys[i] = leaf.keys[i - 1];
                    leaf.values[i] = leaf.values[i - 1];
                }
                leaf.keys[0] = left.keys[left.header.size - 1];
                leaf.values[0] = left.values[left.header.size - 1];
                ++leaf.header.size;
                --left.header.size;
                parent.keys[pos - 1] = leaf.keys[0];
                write_leaf(left_id, left);
                write_leaf(leaf_id, leaf);
                write_internal(parent_id, parent);
                return;
            }
        }

        if (pos < parent.header.size) {
            int right_id = parent.children[pos + 1];
            LeafPage right = read_leaf(right_id);
            if (right.header.parent == parent_id && right.header.size > min_leaf_size()) {
                leaf.keys[leaf.header.size] = right.keys[0];
                leaf.values[leaf.header.size] = right.values[0];
                ++leaf.header.size;
                for (int i = 0; i + 1 < right.header.size; ++i) {
                    right.keys[i] = right.keys[i + 1];
                    right.values[i] = right.values[i + 1];
                }
                --right.header.size;
                parent.keys[pos] = right.keys[0];
                write_leaf(leaf_id, leaf);
                write_leaf(right_id, right);
                write_internal(parent_id, parent);
                return;
            }
        }

        if (pos > 0) {
            int left_id = parent.children[pos - 1];
            LeafPage left = read_leaf(left_id);
            for (int i = 0; i < leaf.header.size; ++i) {
                left.keys[left.header.size + i] = leaf.keys[i];
                left.values[left.header.size + i] = leaf.values[i];
            }
            left.header.size += leaf.header.size;
            left.header.next = leaf.header.next;
            write_leaf(left_id, left);
            remove_child_from_parent(parent_id, pos - 1);
            return;
        }

        int right_id = parent.children[pos + 1];
        LeafPage right = read_leaf(right_id);
        for (int i = 0; i < right.header.size; ++i) {
            leaf.keys[leaf.header.size + i] = right.keys[i];
            leaf.values[leaf.header.size + i] = right.values[i];
        }
        leaf.header.size += right.header.size;
        leaf.header.next = right.header.next;
        write_leaf(leaf_id, leaf);
        remove_child_from_parent(parent_id, pos);
    }

    void remove_child_from_parent(int parent_id, int key_index) {
        InternalPage parent = read_internal(parent_id);
        for (int i = key_index; i + 1 < parent.header.size; ++i) {
            parent.keys[i] = parent.keys[i + 1];
        }
        for (int i = key_index + 1; i < parent.header.size; ++i) {
            parent.children[i] = parent.children[i + 1];
        }
        parent.children[parent.header.size] = parent.children[parent.header.size + 1];
        --parent.header.size;
        write_internal(parent_id, parent);
        rebalance_internal(parent_id, parent);
    }

    void rebalance_internal(int page_id, InternalPage page) {
        if (page_id == root_page()) {
            if (page.header.size == 0) {
                int new_root = page.children[0];
                set_root_page(new_root);
                set_parent(new_root, 0);
            }
            return;
        }

        if (page.header.size >= min_internal_size()) return;

        int parent_id = page.header.parent;
        InternalPage parent = read_internal(parent_id);
        int pos = 0;
        while (pos <= parent.header.size && parent.children[pos] != page_id) ++pos;

        if (pos > 0) {
            int left_id = parent.children[pos - 1];
            InternalPage left = read_internal(left_id);
            if (left.header.parent == parent_id && left.header.size > min_internal_size()) {
                for (int i = page.header.size; i > 0; --i) page.keys[i] = page.keys[i - 1];
                for (int i = page.header.size + 1; i > 0; --i) page.children[i] = page.children[i - 1];
                page.keys[0] = parent.keys[pos - 1];
                page.children[0] = left.children[left.header.size];
                set_parent(page.children[0], page_id);
                parent.keys[pos - 1] = left.keys[left.header.size - 1];
                --left.header.size;
                ++page.header.size;
                write_internal(left_id, left);
                write_internal(page_id, page);
                write_internal(parent_id, parent);
                return;
            }
        }

        if (pos < parent.header.size) {
            int right_id = parent.children[pos + 1];
            InternalPage right = read_internal(right_id);
            if (right.header.parent == parent_id && right.header.size > min_internal_size()) {
                page.keys[page.header.size] = parent.keys[pos];
                page.children[page.header.size + 1] = right.children[0];
                set_parent(page.children[page.header.size + 1], page_id);
                parent.keys[pos] = right.keys[0];
                for (int i = 0; i + 1 < right.header.size; ++i) right.keys[i] = right.keys[i + 1];
                for (int i = 0; i < right.header.size; ++i) right.children[i] = right.children[i + 1];
                right.children[right.header.size] = right.children[right.header.size + 1];
                --right.header.size;
                ++page.header.size;
                write_internal(page_id, page);
                write_internal(right_id, right);
                write_internal(parent_id, parent);
                return;
            }
        }

        if (pos > 0) {
            int left_id = parent.children[pos - 1];
            InternalPage left = read_internal(left_id);
            left.keys[left.header.size] = parent.keys[pos - 1];
            for (int i = 0; i < page.header.size; ++i) left.keys[left.header.size + 1 + i] = page.keys[i];
            for (int i = 0; i <= page.header.size; ++i) {
                left.children[left.header.size + 1 + i] = page.children[i];
                set_parent(page.children[i], left_id);
            }
            left.header.size += page.header.size + 1;
            write_internal(left_id, left);
            remove_child_from_parent(parent_id, pos - 1);
            return;
        }

        int right_id = parent.children[pos + 1];
        InternalPage right = read_internal(right_id);
        page.keys[page.header.size] = parent.keys[pos];
        for (int i = 0; i < right.header.size; ++i) page.keys[page.header.size + 1 + i] = right.keys[i];
        for (int i = 0; i <= right.header.size; ++i) {
            page.children[page.header.size + 1 + i] = right.children[i];
            set_parent(right.children[i], page_id);
        }
        page.header.size += right.header.size + 1;
        write_internal(page_id, page);
        remove_child_from_parent(parent_id, pos);
    }
};

}  // namespace bpt_detail

template <typename T1, typename T2>
struct Bplustree {
    Bplustree();
    explicit Bplustree(const std::string &tree_name);
    Node<T1, T2> root_node;
    std::string tree_file_name;
    bool insert_value(T1 key, T2 data);
    bool erase_value(T1 key);
    bool erase_value(T1 key, T2 data);
    T2 find_value(T1 key);
    std::vector<T2> find_all_values(T1 key);
};

template <typename T1, typename T2>
inline Bplustree<T1, T2>::Bplustree() {
    root_node.node_type = bpt_detail::next_tree_id<T1, T2>();
    tree_file_name = bpt_detail::FileName<T1, T2>::get(root_node.node_type);
    bpt_detail::DiskBPlusTree<T1, T2> tree(tree_file_name);
}

template <typename T1, typename T2>
inline Bplustree<T1, T2>::Bplustree(const std::string &tree_name) {
    root_node.node_type = 0;
    tree_file_name = bpt_detail::named_tree_file<T1, T2>(tree_name);
    bpt_detail::DiskBPlusTree<T1, T2> tree(tree_file_name);
}

template <typename T1, typename T2>
inline bool Bplustree<T1, T2>::insert_value(T1 key, T2 data) {
    bpt_detail::DiskBPlusTree<T1, T2> tree(tree_file_name);
    return tree.insert(key, data);
}

template <typename T1, typename T2>
inline bool Bplustree<T1, T2>::erase_value(T1 key) {
    bpt_detail::DiskBPlusTree<T1, T2> tree(tree_file_name);
    return tree.erase_all(key);
}

template <typename T1, typename T2>
inline bool Bplustree<T1, T2>::erase_value(T1 key, T2 data) {
    bpt_detail::DiskBPlusTree<T1, T2> tree(tree_file_name);
    return tree.erase_pair(key, data);
}

template <typename T1, typename T2>
inline T2 Bplustree<T1, T2>::find_value(T1 key) {
    bpt_detail::DiskBPlusTree<T1, T2> tree(tree_file_name);
    return tree.find_one(key);
}

template <typename T1, typename T2>
inline std::vector<T2> Bplustree<T1, T2>::find_all_values(T1 key) {
    bpt_detail::DiskBPlusTree<T1, T2> tree(tree_file_name);
    return tree.find_all(key);
}
