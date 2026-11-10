#pragma once

#include "Node.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>

// Intrusive LIFO stack. Grid owns the nodes; this stack only links them.
template <typename T>
class Stack {
public:
    Stack() = default;
    ~Stack() { clear(); }
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;
    Stack(Stack&& other) noexcept { swap(other); }
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) { clear(); swap(other); }
        return *this;
    }

    bool isEmpty() const noexcept { return head_ == nullptr; }
    std::size_t getSize() const noexcept { return size_; }
    Node<T>* getTop() const {
        if (isEmpty()) throw std::out_of_range("Stack is empty");
        return head_;
    }

    void push(Node<T>* node) {
        node->link = head_;
        head_ = node;
        ++size_;
    }

    Node<T>* pop() {
        if (isEmpty()) throw std::out_of_range("Stack is empty");
        Node<T>* node = head_;
        head_ = node->link;
        node->link = nullptr;
        --size_;
        return node;
    }

    void clear() noexcept {
        while (head_ != nullptr) {
            Node<T>* node = head_;
            head_ = node->link;
            node->link = nullptr;
        }
        size_ = 0;
    }

private:
    void swap(Stack& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
    }
    Node<T>* head_ = nullptr;
    std::size_t size_ = 0;
};
