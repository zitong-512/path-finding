#pragma once

#include "Algorithm.hpp"
#include "../DataStructures/PriorityQueue.hpp"

#include <cstddef>

// Record priority-queue removals and stop a cyclic experiment after a finite trace.
template <typename T>
Node<T>* runWithExpansionTrace(Algorithm<T>& algorithm, Grid<T>& grid,
                               std::size_t maxExpansions) {
    struct LimitReached {};
    PriorityQueue<T>::setPopObserver([&grid, maxExpansions](Node<T>* node) {
        if (grid.getExpansionOrder().size() >= maxExpansions) throw LimitReached{};
        grid.recordExpansion(node);
    });

    try {
        Node<T>* result = algorithm.run(grid);
        PriorityQueue<T>::clearPopObserver();
        return result;
    } catch (const LimitReached&) {
        PriorityQueue<T>::clearPopObserver();
        grid.markTruncated();
        return nullptr;
    } catch (...) {
        PriorityQueue<T>::clearPopObserver();
        throw;
    }
}
