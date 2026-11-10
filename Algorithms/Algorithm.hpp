#pragma once

#include "../DataStructures/Grid.hpp"

#include <vector>

template <typename T>
class Algorithm {
public:
    virtual ~Algorithm() = default;
    virtual const char* getName() const = 0;
    virtual Node<T>* run(Grid<T>& grid) const = 0;

protected:
    static Node<T>* finish(Grid<T>& grid, const std::vector<int>& parent, bool found) {
        if (!found) return nullptr;

        for (int current = grid.getGoalIndex(); parent[current] != -1; current = parent[current]) {
            grid.getNode(parent[current]).next = &grid.getNode(current);
        }
        return grid.getStart();
    }
};
