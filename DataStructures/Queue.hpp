#pragma once

#include "Node.hpp"

#include <cstddef>
#include <stdexcept>
#include <utility>

// Intrusive FIFO queue. Grid owns the nodes; this queue only links them.
template <typename T>
class Queue {
public:
    Queue() = default;
    ~Queue() { clear(); }
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;
    Queue(Queue&& other) noexcept { swap(other); }
    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) { clear(); swap(other); }
        return *this;
    }

    bool isEmpty() const noexcept { return head_ == nullptr; }
    std::size_t getSize() const noexcept { return size_; }
    Node<T>* getFront() const {
        if (isEmpty()) throw std::out_of_range("Queue is empty");
        return head_;
    }

    void push(Node<T>* node) {
        node->link = nullptr;
        if (head_ == nullptr){head_ = node;}
        else tail_->link = node;
        tail_ = node;
        size++;
    }

    Node<T>* pop() {
        if (isEmpty()) throw std::out_of_range("Queue is empty");
        if (head_->link == nullptr){
            tail_ = nullptr;
        }
        Node<T>* node = head_;
        head_ = head_->link;
        node->link = nullptr;
        size_--;
        return node;

    }

    void clear() noexcept {
        while (head_ != nullptr) {
            Node<T>* node = head_;
            head_ = node->link;
            node->link = nullptr;
        }
        tail_ = nullptr;
        size_ = 0;
    }

private:
    void swap(Queue& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }
    Node<T>* head_ = nullptr;
    Node<T>* tail_ = nullptr;
    std::size_t size_ = 0;
};
