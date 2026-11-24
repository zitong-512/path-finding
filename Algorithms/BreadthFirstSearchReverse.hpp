#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/Queue.hpp"


template <typename T>
class BreadthFirstSearchReverse final : public Algorithm<T> {
public:
    const char* getName() const override { return "Breadth-first search reverse"; }

    Node<T>* run(Grid<T>& grid) const override {
        Node<T>* start = runIterative(grid);
        return start == nullptr ? nullptr : linkPath2(grid, start);
    }

    Node<T>* runIterative(Grid<T>& grid) const {
        grid.clearSearch();
        int discoveryCount = 0;
        Queue<T> frontier;
        Node<T>* goal = grid.getGoal();
        goal->discover(++discoveryCount);
        frontier.push(goal);
        goal->distanceFromGoal = 0;
            
        while (!frontier.isEmpty()) {
            Node<T>* current = frontier.pop();
            if (current == grid.getStart()) return current;
            for (Node<T>* neighbor : current->getNeighbors()) {
                if(neighbor == nullptr) continue;
                if(!neighbor->discovered){
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
        Node<T>* start = grid.getGoal();
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
        for (Node<T>* current = goal; current->next != nullptr; current = current->next) {
            current->next->prev = current;
        }
        return grid.getStart();
    }
    static Node<T>* linkPath2(Grid<T>& grid, Node<T>* start) {
        Node<T>* current = start;
        while (current != grid.getGoal()){
            Node<T>* priorityNeighbor = current;
            int priority = 100;
            for (Node<T>* neighbor : current->getNeighbors()) {
                if (neighbor == nullptr) continue; 
                if (!neighbor->discovered) continue;
                if (neighbor->distanceFromGoal < priority){
                    priority = neighbor->distanceFromGoal;
                    priorityNeighbor = neighbor;
                }
            }
            priorityNeighbor->prev = current;
            current->next = priorityNeighbor;
            current = priorityNeighbor;
        }
        return start;
    }

};
