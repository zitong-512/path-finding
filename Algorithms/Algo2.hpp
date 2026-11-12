#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/PriorityQueue.hpp"

template <typename T>
class Algo2 final : public Algorithm<T> {
public:
    const char* getName() const override { return "Algo2"; }

    Node<T>* run(Grid<T>& grid) const override {
        grid.clearSearch();

        PriorityQueue<T> frontier;
        Node<T>* start = grid.getStart();
        start->settled = true;
        int discoveryCount = 0;
        start->discover(++discoveryCount);
        frontier.push(start, 0);

        while (!frontier.isEmpty()) {
            const auto [current, cost] = frontier.pop();
            current->settled = true;

            if (current == grid.getGoal()) return linkPath(grid, grid.getGoal());

            for (Node<T>* neighbor : current->getNeighbors()) {
                if (neighbor == nullptr || neighbor->settled) continue;
                if (frontier.push(neighbor, cost + 1)) neighbor->prev = current;
                if (!neighbor->discovered) neighbor->discover(++discoveryCount);
            }
        }
        return nullptr;
    }

private:
    static Node<T>* linkPath(Grid<T>& grid, Node<T>* goal) {
        // Repeated visits may create a cycle in prev; check before linking the path.
        Node<T>* current = goal;
        for (int steps = 0; current != grid.getStart() && steps < grid.getNodeCount(); ++steps) {
            if (current == nullptr) return nullptr;
            current = current->prev;
        }
        if (current != grid.getStart()) return nullptr;
        for (Node<T>* current = goal; current != grid.getStart(); current = current->prev) {
            current->prev->next = current;
        }
        return grid.getStart();
    }
};
