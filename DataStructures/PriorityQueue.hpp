#pragma once

#include "Node.hpp"

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>

// Unordered intrusive list. Lower priority numbers are popped first.
template <typename T>
class PriorityQueue {
public:
    using Entry = std::pair<Node<T>*, int>;

    PriorityQueue() = default;
    ~PriorityQueue() { clear(); }
    PriorityQueue(const PriorityQueue&) = delete;
    PriorityQueue& operator=(const PriorityQueue&) = delete;
    PriorityQueue(PriorityQueue&& other) noexcept { swap(other); }
    PriorityQueue& operator=(PriorityQueue&& other) noexcept {
        if (this != &other) { clear(); swap(other); }
        return *this;
    }

    bool isEmpty() const noexcept { return head_ == nullptr; }
    std::size_t getSize() const noexcept { return size_; }

    // Optional observer used by the animation runner; searches need not know about it.
    static void setPopObserver(std::function<void(Node<T>*)> observer) {
        popObserver_ = std::move(observer);
    }
    static void clearPopObserver() { popObserver_ = {}; }

    // Insert the node or lower its priority while it waits in this queue.
    bool push(Node<T>* node, int priority) {
        // Search for the node in the queue and lower its priority if needed.
        for (Node<T>* current = head_; current != nullptr; current = current->link) {
            if (node != current) continue;
            if (priority >= current->priority) return false;
            current->priority = priority;
            return true;
        }

        // The node is not already in the queue; add it to the back.
        node->priority = priority;
        node->link = nullptr;
        if (tail_ == nullptr) head_ = node;
        else tail_->link = node;
        tail_ = node;
        ++size_;
        return true;
    }

    Entry getFront() const {
        if (isEmpty()) throw std::out_of_range("PriorityQueue is empty");
        Node<T>* best = minimum();
        return {best, best->priority};
    }

    Entry pop() {
        if (isEmpty()) throw std::out_of_range("PriorityQueue is empty");
        Node<T>* previous = nullptr;
        Node<T>* bestPrevious = nullptr;
        Node<T>* best = head_;

        for (Node<T>* current = head_; current != nullptr; current = current->link) {
            if (current->priority < best->priority) {
                best = current;
                bestPrevious = previous;
            }
            previous = current;
        }

        if (bestPrevious == nullptr) head_ = best->link;
        else bestPrevious->link = best->link;
        if (best == tail_) tail_ = bestPrevious;
        best->link = nullptr;
        --size_;
        if (popObserver_) popObserver_(best);
        return {best, best->priority};
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
    Node<T>* minimum() const noexcept {
        Node<T>* best = head_;
        for (Node<T>* current = head_->link; current != nullptr; current = current->link) {
            if (current->priority < best->priority) best = current;
        }
        return best;
    }
    void swap(PriorityQueue& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }
    Node<T>* head_ = nullptr;
    Node<T>* tail_ = nullptr;
    std::size_t size_ = 0;
    inline static thread_local std::function<void(Node<T>*)> popObserver_;
};
