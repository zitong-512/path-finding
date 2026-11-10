#include "../Algorithms/BreadthFirstSearch.hpp"
#include "../Algorithms/DepthFirstSearch.hpp"
#include "../Algorithms/Dijkstra.hpp"
#include "../Algorithms/Greedy.hpp"
#include "../DataStructures/PriorityQueue.hpp"
#include "../DataStructures/Queue.hpp"
#include "../DataStructures/Stack.hpp"

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
template <typename T>
std::vector<int> path(const Grid<T>& grid) {
    std::vector<int> result;
    if (grid.getPathLength() < 0) return result;
    const Node<T>* current = grid.getStart();
    while (current != nullptr) {
        result.push_back(grid.getIndex(current->getX(), current->getY()));
        current = current->next;
    }
    return result;
}

template <typename T>
std::vector<int> discoveryTimes(const Grid<T>& grid) {
    std::vector<int> result;
    for (int i = 0; i < grid.getNodeCount(); ++i) {
        result.push_back(grid.getNode(i).discoveredTime);
    }
    return result;
}

template <typename T>
void assertDiscoveryTimes(const Grid<T>& grid) {
    std::vector<bool> seen(static_cast<std::size_t>(grid.getNodeCount()) + 1, false);
    int count = 0;
    for (int i = 0; i < grid.getNodeCount(); ++i) {
        const auto& node = grid.getNode(i);
        const int time = node.discoveredTime;
        assert(node.discovered == (time > 0));
        if (time == 0) continue;
        assert(time <= grid.getNodeCount());
        assert(!seen[time]);
        seen[time] = true;
        ++count;
    }
    for (int time = 1; time <= count; ++time) assert(seen[time]);
}

void testContainers() {
    Grid<int> grid({"S...G"});
    Node<int>* first = &grid.getNode(0);
    Node<int>* second = &grid.getNode(1);
    Node<int>* third = &grid.getNode(2);

    Queue<int> queue;
    assert(queue.isEmpty());
    try { queue.getFront(); assert(false); } catch (const std::out_of_range&) {}
    try { queue.pop(); assert(false); } catch (const std::out_of_range&) {}
    queue.push(first);
    queue.push(second);
    assert(queue.getSize() == 2 && queue.getFront() == first);
    Queue<int> movedQueue(std::move(queue));
    assert(queue.isEmpty() && movedQueue.getSize() == 2);
    assert(movedQueue.pop() == first && first->link == nullptr);
    assert(movedQueue.pop() == second && second->link == nullptr);
    assert(movedQueue.isEmpty());
    movedQueue.push(first);
    movedQueue.clear();
    assert(first->link == nullptr);

    PriorityQueue<int> priorityQueue;
    assert(priorityQueue.isEmpty());
    try { priorityQueue.getFront(); assert(false); } catch (const std::out_of_range&) {}
    try { priorityQueue.pop(); assert(false); } catch (const std::out_of_range&) {}
    assert(priorityQueue.push(first, 4));
    assert(priorityQueue.push(second, 1));
    assert(priorityQueue.push(third, 3));
    assert(!priorityQueue.push(first, 5));
    assert(priorityQueue.push(first, 0));
    assert(priorityQueue.getSize() == 3);
    assert(priorityQueue.getFront() == std::make_pair(first, 0));
    PriorityQueue<int> movedPriorityQueue(std::move(priorityQueue));
    assert(priorityQueue.isEmpty() && movedPriorityQueue.getSize() == 3);
    assert(movedPriorityQueue.pop() == std::make_pair(first, 0));
    assert(movedPriorityQueue.pop() == std::make_pair(second, 1));
    assert(movedPriorityQueue.pop() == std::make_pair(third, 3));
    assert(first->link == nullptr && second->link == nullptr && third->link == nullptr);
    assert(movedPriorityQueue.push(first, 2));
    movedPriorityQueue.clear();
    assert(movedPriorityQueue.isEmpty() && first->link == nullptr);

    Stack<int> stack;
    assert(stack.isEmpty());
    try { stack.getTop(); assert(false); } catch (const std::out_of_range&) {}
    try { stack.pop(); assert(false); } catch (const std::out_of_range&) {}
    stack.push(first);
    stack.push(second);
    assert(stack.getSize() == 2 && stack.getTop() == second);
    Stack<int> movedStack;
    movedStack = std::move(stack);
    assert(stack.isEmpty() && movedStack.getTop() == second);
    assert(movedStack.pop() == second && second->link == nullptr);
    assert(movedStack.getTop() == first);
    movedStack.clear();
    assert(movedStack.isEmpty() && first->link == nullptr);
}
template <template <typename> class Search, typename T>
void testSearch() {
    Grid<T> grid({"S..", "...", "G.."});
    assert(grid.getStart() == &grid.getNode(grid.getStartIndex()));
    assert(grid.getGoal() == &grid.getNode(grid.getGoalIndex()));
    Search<T> search;
    assert(search.run(grid) == &grid.getNode(grid.getStartIndex()));
    const auto iterativePath = path(grid);
    const auto iterativeTimes = discoveryTimes(grid);
    assert(grid.getStart()->discoveredTime == 1);
    assertDiscoveryTimes(grid);
    Node<T>* recursiveResult = search.runRecursive(grid);
    assertDiscoveryTimes(grid);
    if constexpr (std::is_same_v<Search<T>, BreadthFirstSearch<T>>) {
        assert(recursiveResult == grid.getGoal());
        assert(grid.getStart()->next == nullptr);
        assert(search.run(grid) == grid.getStart());
        assert(path(grid) == iterativePath);
        assert(discoveryTimes(grid) == iterativeTimes);
    } else {
        assert(recursiveResult == grid.getStart());
        assert(path(grid).front() == grid.getStartIndex());
        assert(path(grid).back() == grid.getGoalIndex());
    }
    assert(grid.getNode(grid.getGoalIndex()).discovered);
    grid.clearSearch();
    for (int i = 0; i < grid.getNodeCount(); ++i) {
        assert(!grid.getNode(i).discovered);
        assert(grid.getNode(i).discoveredTime == 0);
    }

    Grid<T> blocked({"S#G"});
    assert(search.run(blocked) == nullptr);
    assert(blocked.getPathLength() == -1);
    assert(search.runRecursive(blocked) == nullptr);
    assert(blocked.getPathLength() == -1);
    assert(blocked.getStart()->discoveredTime == 1);
    assert(!blocked.getNode(blocked.getGoalIndex()).discovered);
    assert(blocked.getGoal()->discoveredTime == 0);
}

void testBfsPredecessors() {
    Grid<int> grid({"S..", "...", "G.."});
    BreadthFirstSearch<int> bfs;
    assert(bfs.runIterative(grid) == grid.getGoal());
    assert(grid.getStart()->next == nullptr);
    assert(bfs.run(grid) == grid.getStart());
    assert(path(grid) == std::vector<int>({0, 3, 6}));
    assert(grid.getNode(0).discoveredTime == 1);
    assert(grid.getNode(1).discoveredTime == 2);
    assert(grid.getNode(3).discoveredTime == 3);
    assert(grid.getNode(6).prev == &grid.getNode(3));
    assert(grid.getNode(3).prev == &grid.getNode(0));
    assert(grid.getNode(0).next == &grid.getNode(3));
    assert(grid.getNode(3).next == &grid.getNode(6));
    assert(grid.getNode(6).next == nullptr);

    grid.clearSearch();
    assert(grid.getNode(6).prev == nullptr);
    assert(grid.getNode(0).next == nullptr);
    assert(!grid.getNode(0).discovered && !grid.getNode(6).discovered);
    assert(grid.getNode(0).discoveredTime == 0 && grid.getNode(6).discoveredTime == 0);
}

void testDfsIterative() {
    Grid<int> grid({"S..", "...", "G.."});
    DepthFirstSearch<int> dfs;
    assert(dfs.runIterative(grid) == grid.getGoal());
    assert(grid.getNode(3).discoveredTime == 2);
    assert(grid.getNode(1).discoveredTime == 3);
    assert(grid.getStart()->next == nullptr);
    assert(grid.getPathLength() == -1);
    const Node<int>* current = grid.getGoal();
    for (int steps = 0; current != grid.getStart() && steps < grid.getNodeCount(); ++steps) {
        current = current->prev;
        assert(current != nullptr);
    }
    assert(current == grid.getStart());
    assert(dfs.run(grid) == grid.getStart());
    assert(grid.getPathLength() >= 0);
}

template <typename T>
void testDijkstra() {
    Grid<T> grid({"S..", ".#.", "..G"});
    Dijkstra<T> search;
    Algorithm<T>& algorithm = search;

    assert(algorithm.run(grid) == grid.getStart());
    assert(grid.getPathLength() == 4);
    assert(grid.getStart()->settled);
    assert(grid.getGoal()->settled);
    assertDiscoveryTimes(grid);
    const auto route = path(grid);
    assert(route.front() == grid.getStartIndex());
    assert(route.back() == grid.getGoalIndex());
    for (std::size_t i = 1; i < route.size(); ++i) {
        const auto& from = grid.getNode(route[i - 1]);
        const auto& to = grid.getNode(route[i]);
        assert(!to.getBlocked());
        assert(std::abs(from.getX() - to.getX()) + std::abs(from.getY() - to.getY()) == 1);
    }

    const auto times = discoveryTimes(grid);
    assert(search.run(grid) == grid.getStart());
    assert(path(grid) == route);
    assert(discoveryTimes(grid) == times);

    grid.clearSearch();
    for (int i = 0; i < grid.getNodeCount(); ++i) {
        assert(!grid.getNode(i).settled);
    }

    Grid<T> blocked({"S#G"});
    assert(search.run(blocked) == nullptr);
    assert(blocked.getPathLength() == -1);
    assert(blocked.getStart()->settled);
    assert(!blocked.getGoal()->settled);
    assert(blocked.getStart()->discoveredTime == 1);
    assert(blocked.getGoal()->discoveredTime == 0);
}

template <typename T>
void testGreedy() {
    Grid<T> grid({"S..", ".#.", "..G"});
    Greedy<T> search;
    Algorithm<T>& algorithm = search;

    assert(algorithm.run(grid) == grid.getStart());
    assert(grid.getPathLength() == 4);
    assert(grid.getStart()->settled);
    assert(grid.getGoal()->settled);
    assertDiscoveryTimes(grid);
    const auto route = path(grid);
    assert(route.front() == grid.getStartIndex());
    assert(route.back() == grid.getGoalIndex());

    assert(search.run(grid) == grid.getStart());
    assert(path(grid) == route);

    Grid<T> blocked({"S#G"});
    assert(search.run(blocked) == nullptr);
    assert(blocked.getPathLength() == -1);
    assert(blocked.getStart()->settled);
    assert(!blocked.getGoal()->discovered);
}

void testNodeValues() {
    Grid<int> numbers({"SG"}, 5);
    assert(numbers.getNode(numbers.getStartIndex()).value == 5);
    numbers.getNode(numbers.getGoalIndex()).value = 42;
    BreadthFirstSearch<int> bfs;
    bfs.run(numbers);
    assert(numbers.getPathLength() == 1);
    assert(numbers.getNode(numbers.getGoalIndex()).value == 42);

    Grid<std::string> labels({"SG"}, "empty");
    assert(labels.getNode(labels.getGoalIndex()).value == "empty");
    labels.getNode(labels.getStartIndex()).value = "start";
    assert(labels.getNode(labels.getGoalIndex()).value == "empty");
    DepthFirstSearch<std::string> dfs;
    dfs.run(labels);
    assert(labels.getPathLength() == 1);
    assert(labels.getNode(labels.getStartIndex()).value == "start");
}
}

int main() {
    testContainers();
    testNodeValues();
    testBfsPredecessors();
    testDfsIterative();
    testDijkstra<int>();
    testDijkstra<std::string>();
    testGreedy<int>();
    testGreedy<std::string>();
    testSearch<BreadthFirstSearch, int>();
    testSearch<DepthFirstSearch, int>();
    testSearch<BreadthFirstSearch, std::string>();
    testSearch<DepthFirstSearch, std::string>();
}
