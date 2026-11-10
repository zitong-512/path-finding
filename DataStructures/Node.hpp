#pragma once

#include <array>
#include <utility>

template <typename T>
class Node {
public:
    Node(int x, int y, bool blocked, T initialValue = T{})
        : value(std::move(initialValue)), x_(x), y_(y), blocked_(blocked) {}

    int getX() const { return x_; }
    int getY() const { return y_; }
    bool getBlocked() const { return blocked_; }

    // User data associated with this cell; search does not change it.
    T value;

    // A null pointer means an edge of the grid or a wall.
    Node<T>* up = nullptr;
    Node<T>* right = nullptr;
    Node<T>* down = nullptr;
    Node<T>* left = nullptr;

    // Set when first discovered: the node that led to this one.
    Node<T>* prev = nullptr;
    // Filled in after a search: follow next from S to G.
    Node<T>* next = nullptr;
    // The queue, stack, or priority queue chains waiting nodes through link.
    Node<T>* link = nullptr;
    // Dijkstra uses distance here; Greedy uses the heuristic.
    int priority = 0;

    // Search state, reset by Grid::clearSearch before each run.
    bool discovered = false;
    // Set when Dijkstra or Greedy removes this node from the frontier.
    bool settled = false;
    // Zero until discovered; otherwise the 1-based discovery order.
    int discoveredTime = 0;

    void discover(int time) {
        discovered = true;
        discoveredTime = time;
    }

    std::array<Node<T>*, 4> getNeighbors() const {
        return {right, down, left, up};
    }

private:
    int x_;
    int y_;
    bool blocked_;
};
