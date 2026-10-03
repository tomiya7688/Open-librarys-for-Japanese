#include <iostream>

#include "binary_tree.hpp"

int main() {
    olj::BinarySearchTree<int> tree;

    for (int value : {8, 3, 10, 1, 6, 14, 4, 7, 13}) {
        tree.insert(value);
    }

    std::cout << "contains 7: " << tree.contains(7) << '\n';
    std::cout << "size: " << tree.size() << '\n';

    std::cout << "inorder: ";
    for (int value : tree.inorder()) {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    tree.erase(8);

    std::cout << "after erase(8): ";
    for (int value : tree.inorder()) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}
