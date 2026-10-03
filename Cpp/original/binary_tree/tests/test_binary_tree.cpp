#include "binary_tree.hpp"

#include <cassert>
#include <vector>

int main() {
    olj::BinarySearchTree<int> tree;

    assert(tree.empty());

    assert(tree.insert(8));
    assert(tree.insert(3));
    assert(tree.insert(10));
    assert(tree.insert(1));
    assert(tree.insert(6));
    assert(tree.insert(14));
    assert(tree.insert(4));
    assert(tree.insert(7));
    assert(tree.insert(13));
    assert(!tree.insert(6));

    assert(tree.size() == 9);
    assert(tree.contains(7));
    assert(!tree.contains(99));

    assert((tree.inorder() ==
            std::vector<int>{1, 3, 4, 6, 7, 8, 10, 13, 14}));
    assert((tree.preorder() ==
            std::vector<int>{8, 3, 1, 6, 4, 7, 10, 14, 13}));
    assert((tree.postorder() ==
            std::vector<int>{1, 4, 7, 6, 3, 13, 14, 10, 8}));

    assert(tree.erase(1));
    assert(tree.erase(14));
    assert(tree.erase(8));
    assert(!tree.erase(123));

    assert(tree.size() == 6);
    assert((tree.inorder() ==
            std::vector<int>{3, 4, 6, 7, 10, 13}));

    auto copied = tree;
    assert(copied.inorder() == tree.inorder());

    copied.insert(42);
    assert(copied.contains(42));
    assert(!tree.contains(42));

    tree.clear();
    assert(tree.empty());
    assert(tree.size() == 0);
}
