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

        // Each waiting node is prioritized by its distance to the goal.

        // Define the frontier 
        // Define Discoverycount
        // Define goal
        // Define start
        // Set the heuristic to start. Each node has a heuristic attribute and we have here a heuristic function.
        // Discover start
        // Push start

        PriorityQueue<T> frontier;
        int discoveryCount = 0;
        Node<T>* goal = grid.getGoal();
        Node<T>* start = grid.getStart();
        start->discover(++discoveryCount);
        frontier.push(start, heuristic(start, goal));

        while (!frontier.isEmpty()) {
            // The list is unordered; pop scans it for the lowest heuristic.
            Node<T>* current = frontier.pop().first;
            current->settled = true;

            if (current == goal) return linkPath(grid, current);

            for (Node<T>* neighbor : current->getNeighbors()) {
                if (neighbor == nullptr) continue;
                if (neighbor->settled) continue;
                if (!frontier.push(neighbor, heuristic(neighbor, goal))) continue;
                neighbor->prev = current;
                if (!neighbor->discovered) neighbor->discover(++discoveryCount);
            }
        }
        return nullptr;
    }

private:
    static int heuristic(const Node<T>* node, const Node<T>* goal) {
        return std::abs(node->getX() - goal->getX())
             + std::abs(node->getY() - goal->getY());
    }

    static Node<T>* linkPath(Grid<T>& grid, Node<T>* goal) {
        for ()
        for (Node<T>* current = goal; current->prev != nullptr; current = current->prev) {
            current->prev->next = current;
        }
        return grid.getStart();
    }
};
