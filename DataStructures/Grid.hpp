#pragma once

#include "Node.hpp"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

template <typename T>
class Grid {
public:
    explicit Grid(std::vector<std::string> rows, T defaultValue = T{})
        : rows_(std::move(rows)),
          width_(rows_.empty() ? 0 : static_cast<int>(rows_[0].size())),
          height_(static_cast<int>(rows_.size())) {
        if (width_ == 0) throw std::invalid_argument("The grid must not be empty");

        nodes_.reserve(static_cast<std::size_t>(width_) * height_);
        for (int y = 0; y < height_; ++y) {
            if (static_cast<int>(rows_[y].size()) != width_) {
                throw std::invalid_argument("All grid rows must have the same width");
            }
            for (int x = 0; x < width_; ++x) {
                const char cell = rows_[y][x];
                if (cell != '.' && cell != '#' && cell != 'S' && cell != 'G') {
                    throw std::invalid_argument("Use only '.', '#', 'S', and 'G' in the grid");
                }
                if (cell == 'S') {
                    if (start_ != nullptr) throw std::invalid_argument("The grid needs exactly one S");
                }
                if (cell == 'G') {
                    if (goal_ != nullptr) throw std::invalid_argument("The grid needs exactly one G");
                }
                nodes_.emplace_back(x, y, cell == '#', defaultValue);
                if (cell == 'S') start_ = &nodes_.back();
                if (cell == 'G') goal_ = &nodes_.back();
            }
        }
        if (start_ == nullptr || goal_ == nullptr) {
            throw std::invalid_argument("The grid needs one S and one G");
        }

        // Link only walkable cells. Reserve above keeps these pointers stable.
        const auto walkableAt = [this](int x, int y) -> Node<T>* {
            if (x < 0 || x >= width_ || y < 0 || y >= height_) return nullptr;
            Node<T>& candidate = getAt(x, y);
            return candidate.getBlocked() ? nullptr : &candidate;
        };
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                Node<T>& current = getAt(x, y);
                if (current.getBlocked()) continue;
                current.right = walkableAt(x + 1, y);
                current.down = walkableAt(x, y + 1);
                current.left = walkableAt(x - 1, y);
                current.up = walkableAt(x, y - 1);
            }
        }
    }

    Grid(const Grid&) = delete;
    Grid& operator=(const Grid&) = delete;
    Grid(Grid&&) = delete;
    Grid& operator=(Grid&&) = delete;

    int getIndex(int x, int y) const { return y * width_ + x; }
    Node<T>* getStart() { return start_; }
    const Node<T>* getStart() const { return start_; }
    Node<T>* getGoal() { return goal_; }
    const Node<T>* getGoal() const { return goal_; }
    int getStartIndex() const { return getIndex(start_->getX(), start_->getY()); }
    int getGoalIndex() const { return getIndex(goal_->getX(), goal_->getY()); }
    int getNodeCount() const { return static_cast<int>(nodes_.size()); }
    Node<T>& getNode(int index) { return nodes_[index]; }
    const Node<T>& getNode(int index) const { return nodes_[index]; }
    const std::vector<std::string>& getRows() const { return rows_; }

    void clearSearch() {
        for (Node<T>& current : nodes_) {
            current.prev = nullptr;
            current.next = nullptr;
            current.link = nullptr;
            current.priority = 0;
            current.discovered = false;
            current.settled = false;
            current.discoveredTime = 0;
        }
    }

    int getPathLength() const {
        const Node<T>* current = start_;
        for (int steps = 0; current != nullptr && steps < getNodeCount(); ++steps) {
            if (current == goal_) return steps;
            current = current->next;
        }
        return -1;
    }

private:
    Node<T>& getAt(int x, int y) { return nodes_[getIndex(x, y)]; }

    std::vector<std::string> rows_;
    int width_;
    int height_;
    std::vector<Node<T>> nodes_;
    Node<T>* start_ = nullptr;
    Node<T>* goal_ = nullptr;
};
