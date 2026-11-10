#pragma once

#include "Node.hpp"

#include <cstddef>
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

    // Insert the node or lower its priority while it waits in this queue.
    bool push(Node<T>* node, int priority) {
        // Search for the node in the queue and lower its priority if needed.
        for (Node<T>* current = head_; current != nullptr; current = current->link){
            if(node != current)continue;
            if(current->priority >= priority) return false;
            current->priority = priority;
            return true;
        
        }
        // The node is not already in the queue; add it to the front.
        node->link = nullptr;
        if (head_ == nullptr){head_ = node;}
        else tail_->link = node;
        tail_ = node;
        size++;
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
        for (Node<T>* current = head_; current != nullptr; current = current->link){
            if (current->priority < best->priority){
                best = current;
                bestPrevious = previous
            }
            previous = current;
        }
            if (head_->link == nullptr){tail = nullptr}
            previous->link = best->link
            best->link = nullptr
            size--;
            return best;
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
    Node<T>* minimum() const noexcept {
        Node<T>* best = head_;
        for (Node<T>* current = head_->link; current != nullptr; current = current->link) {
            if (current->priority < best->priority) best = current;
        }
        return best;
    }
    void swap(PriorityQueue& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
    }
    Node<T>* head_ = nullptr;
    std::size_t size_ = 0;
};
