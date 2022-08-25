#include "include/skiplist.h"

int main() {

    // skiplist::SkipList<int, int> sl(10);
    std::shared_ptr<skiplist::SkipList<int, int>> sl(new skiplist::SkipList<int, int> (10));
    

    sl->insert_element(2, 2);
    sl->search_element(2);
    sl->delete_element(2);
    return EXIT_SUCCESS;
}