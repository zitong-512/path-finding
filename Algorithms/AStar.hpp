#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/PriorityQueue.hpp"

#include <cstdlib>

template <typename T>
class AStar final : public Algorithm<T> {
public:
    const char* getName() const override { return "A* search"; }

    Node<T>* run(Grid<T>& grid) const override {
        grid.clearSearch();

        PriorityQueue<T> frontier;
        int discoveryCount = 0;
        Node<T>* goal = grid.getGoal();
        Node<T>* start = grid.getStart();

        start->distance = 0;
        start->heuristic = heuristic(start, goal);
        start->discover(++discoveryCount);
        frontier.push(start, start->distance + start->heuristic);

        while (!frontier.isEmpty()) {
            Node<T>* current = frontier.pop().first;
            current->settled = true;
            if (current == goal) return linkPath(grid, current);

            for (Node<T>* neighbor : current->getNeighbors()) {
                if (neighbor == nullptr || neighbor->settled) continue;

                const int nextCost = current->distance + 1;
                if (nextCost >= neighbor->distance) continue;

                if (!neighbor->discovered){
                    neighbor->heuristic = heuristic(neighbor, goal);
                    neighbor->discover(++discoveryCount);
                }

                // The queue scans its list, so lowering an existing priority is safe.
                frontier.push(neighbor, nextCost + neighbor->heuristic);
                neighbor->distance = nextCost;
                neighbor->prev = current;
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
        for (Node<T>* current = goal; current->prev != nullptr; current = current->prev) {
            Node<T>* node = current->prev;
            node->next = current;
        }
        return grid.getStart();
    }
};
