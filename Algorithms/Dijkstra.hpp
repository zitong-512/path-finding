#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/PriorityQueue.hpp"

template <typename T>
class Dijkstra final : public Algorithm<T> {
public:
    const char* getName() const override { return "Dijkstra's algorithm"; }

    Node<T>* run(Grid<T>& grid) const override {
        grid.clearSearch();

        PriorityQueue<T> frontier;
        int discoveryCount = 0;
        Node<T>* start = grid.getStart();
        start->discover(++discoveryCount);
        frontier.push(start, 0);

        while (!frontier.isEmpty()) {

            const auto [current, cost] = frontier.pop();
            current->settled = true;
            if (current == grid.getGoal()) return linkPath(grid, grid.getGoal());

            for (Node<T>* neighbor : current->getNeighbors()) {
                if (neighbor == nullptr) continue; // If we don't have any neighbors.
                if (neighbor->settled) continue; // If we already found the best path to this neighbor.

                int nextCost = cost + 1;

                // Push into frontier and update `prev` if necessary.
                if (frontier.push(neighbor, nextCost)) neighbor->prev = current;
                if (!neighbor->discovered) neighbor->discover(++discoveryCount);
            }
        }
        return nullptr;
    }

private:
    static Node<T>* linkPath(Grid<T>& grid, Node<T>* goal) {
        for (Node<T>* current = goal; current->prev != nullptr; current = current->prev) {
            current->prev->next = current;
        }
        return grid.getStart();
    }
};
