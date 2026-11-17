#pragma once

#include <cmath>
#include "Algorithm.hpp"
#include "../DataStructures/PriorityQueue.hpp"

template <typename T>
class Algo1 final : public Algorithm<T> {
public:
    const char* getName() const override { return "Algo1"; }
    // git reset --hard origin/main
    Node<T>* run(Grid<T>& grid) const override {
        grid.clearSearch();

        PriorityQueue<T> frontier;
        Node<T>* start = grid.getStart();
        Node<T>* goal = grid.getGoal();
        int discoveryCount = 0;
        int cost = 0;
        
        start->discover(++discoveryCount);
        frontier.push(start, 0);

        while (!frontier.isEmpty()) {
            Node<T>* current = frontier.pop().first;
            current->settled = true;
            cost++;
            if (current == grid.getGoal()) return linkPath(grid, grid.getGoal());

            for (Node<T>* neighbor : current->getNeighbors()) {
                if (current->right == neighbor) continue;
                if (neighbor == nullptr || neighbor->settled) continue;
                if (frontier.push(neighbor, heuristic(neighbor, goal) + cost)) neighbor->prev = current;
                if (!neighbor->discovered) neighbor->discover(++discoveryCount);
            }
        }
        return nullptr;
    }

private:
    static int heuristic(const Node<T>* node, const Node<T>* goal) {
        return std::sqrt(std::pow(std::abs(node->getY() - goal->getY()), 2)
         + std::pow(std::abs(node->getX() - goal->getX()), 2));
    }

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
