#include "include/skiplist.h"

#define EXIT_SCCESS 0
#define EXIT_FAILED 1

int main() {

    skiplist::Node<int, int> node(1, 1, 1);
    std::cout << node.get_value() << std::endl;

    return EXIT_SCCESS;
}