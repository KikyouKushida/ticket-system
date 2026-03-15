#include "info.h"

struct Page {
    int page_id;
    int page_type;
    bool dirty;
    char data[PAGE_SIZE - 9];
};