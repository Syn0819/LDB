#include "include/skiplist.h"


int main() {

    skiplist::SkipList<int, int> sl(10);
    sl.insert_element(2, 2);
    sl.search_element(2);
    
    return EXIT_SUCCESS;
}