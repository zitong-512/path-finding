#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/Queue.hpp"


template <typename T>
class BreadthFirstSearch2Sided final : public Algorithm<T> {
public:
    const char* getName() const override { return "BreadthFirstSearch 2-Sided"; }

    Node<T>* run(Grid<T>& grid) const override {
        Node<T>* goal = runIterative(grid);
        return goal == nullptr ? nullptr : linkPath(grid, goal);
    }

    Node<T>* runIterative(Grid<T>& grid) const {
        grid.clearSearch();
        int discoveryCount = 0;
        Queue<T> frontier;
        Node<T>* start = grid.getStart();
        Node<T>* goal = grid.getGoal();
        start->discover(++discoveryCount);
        start->discoveredFromStart=true;
        goal->discoveredFromEnd=true;
        goal->discover(++discoveryCount);
        frontier.push(start);
        frontier.push(goal);

        while (!frontier.isEmpty()) {
            Node<T>* current = frontier.pop();
            if (current->discoveredFromStart==true and current->discoveredFromEnd==true) return current;
            for (Node<T>* neighbor : current->getNeighbors()){
                if(neighbor == nullptr) continue;
                if((current->discoveredFromStart and !neighbor->discoveredFromStart) or
                (current->discoveredFromEnd and !neighbor->discoveredFromEnd)){
                    if (current->discoveredFromStart==true) {
                        neighbor->discoveredFromStart=true;
                        neighbor->prev = current;
                    }
                    if (current->discoveredFromEnd==true) {
                        neighbor->discoveredFromEnd=true;
                        neighbor->next = current;
                    }
                    neighbor->discover(++discoveryCount);
                    neighbor->distanceFromGoal = current->distanceFromGoal + 1;
                    frontier.push(neighbor);
                    
                }
            }
        }
        return nullptr;
    }

    Node<T>* runRecursive(Grid<T>& grid) const {
        grid.clearSearch();
        int discoveryCount = 0;
        Queue<T> frontier;
        Node<T>* start = grid.getStart();
        start->discover(++discoveryCount);
        frontier.push(start);

        return exploreRecursive(grid, frontier, discoveryCount);
    }

    

private:
    static void enqueueNeighbors(Node<T>* current,
                                 Queue<T>& frontier, int& discoveryCount) {
        for (Node<T>* neighbor : current->getNeighbors()) {
            if (neighbor == nullptr) continue;
            if (neighbor->discovered) continue;
            // The first discovery gives a shortest route to this neighbor.
            neighbor->discover(++discoveryCount);
            neighbor->prev = current;
            frontier.push(neighbor);
        }
    }

    static Node<T>* exploreRecursive(Grid<T>& grid, Queue<T>& frontier,
                                         int& discoveryCount) {
        if (frontier.isEmpty()) return nullptr;

        Node<T>* current = frontier.pop();
        if (current == grid.getGoal()) return current;

        enqueueNeighbors(current, frontier, discoveryCount);
        return exploreRecursive(grid, frontier, discoveryCount);
    }

    static Node<T>* linkPath(Grid<T>& grid, Node<T>* goal) {
        // prev points backward through the search tree; next marks only the path.
        for (Node<T>* current = goal; current->prev != nullptr; current = current->prev) {
            current->prev->next = current;
        }
        return grid.getStart();
    }

};
