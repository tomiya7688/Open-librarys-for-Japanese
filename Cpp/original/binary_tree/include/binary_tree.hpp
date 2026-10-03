#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace olj {

template <typename T, typename Compare = std::less<T>>
class BinarySearchTree {
private:
    struct Node {
        T value;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;

        explicit Node(const T& value) : value(value) {}
        explicit Node(T&& value) : value(std::move(value)) {}
    };

    std::unique_ptr<Node> root_;
    std::size_t size_ = 0;
    Compare compare_{};

    static std::unique_ptr<Node> clone(const std::unique_ptr<Node>& node) {
        if (!node) {
            return nullptr;
        }

        auto copy = std::make_unique<Node>(node->value);
        copy->left = clone(node->left);
        copy->right = clone(node->right);
        return copy;
    }

    bool equals(const T& a, const T& b) const {
        return !compare_(a, b) && !compare_(b, a);
    }

    template <typename U>
    bool insert_impl(U&& value) {
        std::unique_ptr<Node>* current = &root_;

        while (*current) {
            if (equals(value, (*current)->value)) {
                return false;
            }

            current = compare_(value, (*current)->value)
                ? &((*current)->left)
                : &((*current)->right);
        }

        *current = std::make_unique<Node>(std::forward<U>(value));
        ++size_;
        return true;
    }

    bool erase_impl(std::unique_ptr<Node>& node, const T& value) {
        if (!node) {
            return false;
        }

        if (compare_(value, node->value)) {
            return erase_impl(node->left, value);
        }

        if (compare_(node->value, value)) {
            return erase_impl(node->right, value);
        }

        if (!node->left) {
            node = std::move(node->right);
        } else if (!node->right) {
            node = std::move(node->left);
        } else {
            auto left = std::move(node->left);
            auto right = std::move(node->right);

            std::unique_ptr<Node>* successor_link = &right;
            while ((*successor_link)->left) {
                successor_link = &((*successor_link)->left);
            }

            auto successor = std::move(*successor_link);
            *successor_link = std::move(successor->right);
            successor->left = std::move(left);
            successor->right = std::move(right);
            node = std::move(successor);
        }

        --size_;
        return true;
    }

    static void inorder_impl(const std::unique_ptr<Node>& node, std::vector<T>& out) {
        if (!node) {
            return;
        }

        inorder_impl(node->left, out);
        out.push_back(node->value);
        inorder_impl(node->right, out);
    }

    static void preorder_impl(const std::unique_ptr<Node>& node, std::vector<T>& out) {
        if (!node) {
            return;
        }

        out.push_back(node->value);
        preorder_impl(node->left, out);
        preorder_impl(node->right, out);
    }

    static void postorder_impl(const std::unique_ptr<Node>& node, std::vector<T>& out) {
        if (!node) {
            return;
        }

        postorder_impl(node->left, out);
        postorder_impl(node->right, out);
        out.push_back(node->value);
    }

public:
    BinarySearchTree() = default;

    explicit BinarySearchTree(Compare compare)
        : compare_(std::move(compare)) {}

    BinarySearchTree(const BinarySearchTree& other)
        : root_(clone(other.root_)),
          size_(other.size_),
          compare_(other.compare_) {}

    BinarySearchTree& operator=(const BinarySearchTree& other) {
        if (this == &other) {
            return *this;
        }

        BinarySearchTree copy(other);
        swap(copy);
        return *this;
    }

    BinarySearchTree(BinarySearchTree&&) noexcept = default;
    BinarySearchTree& operator=(BinarySearchTree&&) noexcept = default;

    void swap(BinarySearchTree& other) {
        using std::swap;
        swap(root_, other.root_);
        swap(size_, other.size_);
        swap(compare_, other.compare_);
    }

    bool insert(const T& value) {
        return insert_impl(value);
    }

    bool insert(T&& value) {
        return insert_impl(std::move(value));
    }

    [[nodiscard]] bool contains(const T& value) const {
        const Node* current = root_.get();

        while (current) {
            if (equals(value, current->value)) {
                return true;
            }

            current = compare_(value, current->value)
                ? current->left.get()
                : current->right.get();
        }

        return false;
    }

    bool erase(const T& value) {
        return erase_impl(root_, value);
    }

    void clear() noexcept {
        root_.reset();
        size_ = 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] std::vector<T> inorder() const {
        std::vector<T> result;
        result.reserve(size_);
        inorder_impl(root_, result);
        return result;
    }

    [[nodiscard]] std::vector<T> preorder() const {
        std::vector<T> result;
        result.reserve(size_);
        preorder_impl(root_, result);
        return result;
    }

    [[nodiscard]] std::vector<T> postorder() const {
        std::vector<T> result;
        result.reserve(size_);
        postorder_impl(root_, result);
        return result;
    }
};

template <typename T, typename Compare>
void swap(BinarySearchTree<T, Compare>& a, BinarySearchTree<T, Compare>& b) {
    a.swap(b);
}

}  // namespace olj
