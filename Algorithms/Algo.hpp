#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/PriorityQueue.hpp"

template <typename T>
class Algo1 final : public Algorithm<T> {
public:
    const char* getName() const override { return "Algo1"; }

    Node<T>* run(Grid<T>& grid) const override {
        grid.clearSearch();

        PriorityQueue<T> frontier;
        int discoveryCount = 0;
        Node<T>* goal = grid.getGoal();
        Node<T>* start = grid.getStart();
        frontier.push(start, 0);

        while(!frontier.isEmpty()){
            Node<T>* current = frontier.pop().first;

            for(Node<T>* neighbor : frontierNode->getNeighbors()){
                if(neighbor == goal)  return linkPath(grid, goal);
            }
            bool settled = neighbor->settled;
            if (settled) continue;
            frontier.push(neighbor, 0);
        }
        
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
