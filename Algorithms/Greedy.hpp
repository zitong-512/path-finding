#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/PriorityQueue.hpp"

#include <cstdlib>

template <typename T>
class Greedy final : public Algorithm<T> {
public:
    const char* getName() const override { return "Greedy best-first search"; }

    Node<T>* run(Grid<T>& grid) const override {
        grid.clearSearch();

        // Each waiting node is prioritized by its distance to the goal
        PriorityQueue<T> frontier;
        int discoveryCount = 0;
        Node<T>* goal = grid.getGoal();
        Node<T>* start = grid.getStart();
        start->heuristic = heuristic(start, goal);
        start->discover(++discoveryCount); // Here we discover a node
        frontier.push(start, start->heuristic);

        while (!frontier.isEmpty()) {
            // The list is unordered; pop scans it for the lowest heuristic.
            const auto [current, heuristicValue] = frontier.pop();
            current->settled = true; // Here we settle a node

            if (current == goal) return linkPath(grid, current);

            for (Node<T>* neighbor : current->getNeighbors()) {
                if (neighbor == nullptr) continue;
                if (neighbor->settled) continue;

                // Is this redundant?
                if (!neighbor->discovered) neighbor->heuristic = heuristic(neighbor, goal);
                if (!frontier.push(neighbor, neighbor->heuristic)) continue;

                neighbor->prev = current;
                if (!neighbor->discovered) neighbor->discover(++discoveryCount); // Here we discover a node
            }
        }
        return nullptr;
    }

private:
    // Manhattan/Taxicab Distance
    static int heuristic(const Node<T>* node, const Node<T>* goal) {
        return std::abs(node->getX() - goal->getX())
             + std::abs(node->getY() - goal->getY());
    }

    static Node<T>* linkPath(Grid<T>& grid, Node<T>* goal) {
        for (Node<T>* current = goal; current->prev != nullptr; current = current->prev) {
            current->prev->next = current;
        }
        return grid.getStart();
    }
};
