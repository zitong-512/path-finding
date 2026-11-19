#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/PriorityQueue.hpp"

#include <cstdlib>
#include <cmath>

template <typename T>
class Algo final : public Algorithm<T> {
public:
    const char* getName() const override { return "Algo"; }

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
        return std::sqrt((node->getX() - goal->getX()) * (node->getX() - goal->getX())
             + (node->getY() - goal->getY()) * (node->getY() - goal->getY()));
    }

    static Node<T>* linkPath(Grid<T>& grid, Node<T>* goal) {
        for (Node<T>* current = goal; current->prev != nullptr; current = current->prev) {
            Node<T>* node = current->prev;
            node->next = current;
        }
        return grid.getStart();
    }
};
