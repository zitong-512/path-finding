# Grid pathfinding

A small C++17 grid search example. `Grid<T>` owns `Node<T>` objects. Each
node holds a `T value` plus `up`, `right`, `down`, and `left` pointers; edges
and walls leave the corresponding pointers null. `Grid<T>` stores pointers to
its start and goal nodes and exposes them through `getStart()` and `getGoal()`.
Breadth-first search records
the first predecessor in `prev`, then links the found path through `next`.
Each node also has a `discovered` flag to avoid revisiting it and a 1-based
`discoveredTime` that records when it was first discovered. Zero means it was
not discovered. Each search run owns its discovery counter and asks the node to
record its time. Dijkstra and Greedy also mark nodes `settled` when removed from
their frontiers. The grid resets these search fields, `prev`, and `next` before each
search. The animation plays back the discovery times.
Iterative DFS marks a node when it is pushed onto the stack, so each node is
stacked once. Its discovery times may differ from recursive DFS, which marks a
node when the recursion enters it.
`Algorithm<T>`, `BreadthFirstSearch<T>`, `DepthFirstSearch<T>`, `Dijkstra<T>`, and `Greedy<T>` work with
either `int` or `std::string` values. `Grid<T>` owns all `Node<T>` objects.
`Queue<T>`, `Stack<T>`, and `PriorityQueue<T>` chain those nodes through `link`
without allocating or owning extra nodes. `prev` and `next` remain reserved for
the search path. A node can belong to only one frontier at a time.
BFS also uses `Queue<T>` recursively.
Dijkstra's priority queue is an unordered linked list of nodes. It stores the
waiting distance in each node's `priority` field; improving a route updates
that field, and popping scans for the smallest value. Greedy uses the same
field for Manhattan distance to the goal.

The C++ backend writes `web/run.json` through `Output/JsonWriter.hpp`. The
static frontend (`web/index.html`, `web/style.css`, and `web/app.js`) draws the
grid and animates the search. The JSON writer follows `Node::next` to export
the path; the algorithms do not store a second path. Node values stay in C++
and are not shown in the animation.

For example, both grids can run the same search algorithm:

```cpp
Grid<int> numbers({"S.G"}, 0);
numbers.getNode(numbers.getStartIndex()).value = 7;
BreadthFirstSearch<int> numberSearch;
numberSearch.run(numbers);

Grid<std::string> labels({"S.G"}, "empty");
labels.getNode(labels.getGoalIndex()).value = "destination";
BreadthFirstSearch<std::string> labelSearch;
labelSearch.run(labels);
```

In `main.cpp`, change `using CellValue = int;` to `using CellValue = std::string;`
to switch the application's node value type.

## Use in VS Code on Windows

1. Have a C++17 compiler (`g++`, `clang++`, or MSVC `cl`) available on `PATH`.
2. Install the recommended **Live Preview** extension by Microsoft. Install
   **Run on Save** as well if you want every C++ save to rebuild automatically.
3. Press **Ctrl+Shift+B** once. This compiles and runs the app, producing
   `web/run.json`.
4. Open `web/index.html` with **Live Preview: Show Preview** and move the
   preview editor beside `main.cpp`. Keep it open while editing.
5. Change the map near the top of `main.cpp`, or switch between
   `BreadthFirstSearch`, `DepthFirstSearch`, `Dijkstra`, and `Greedy` there. Press **Ctrl+Shift+B**, or
   just save if you installed Run on Save. The open page detects the new JSON
   and restarts the animation automatically.

`S` is the start, `G` is the goal, `#` is a wall, and `.` is open space. Rows
must be equally wide. Movement is in four directions. Breadth-first search
finds a shortest path when each move has the same cost; depth-first search finds
a path but does not guarantee the shortest one. Dijkstra also finds a shortest
path using a priority queue. All grid moves currently cost one, so Dijkstra
and breadth-first search have the same path length. Greedy follows the estimated
distance to the goal and can find a longer path. Node `value` does not set
movement cost. The HTML has pause, restart,
and scrub controls. The iterative implementations avoid recursion depth limits;
the recursive implementations can exhaust the call stack on very large grids.

BFS and DFS also expose `runIterative(grid)` and `runRecursive(grid)`
for direct calls. All four algorithms provide `run(grid)` through the common
`Algorithm` interface; it returns the first node of the linked path, or
`nullptr` when no path exists. The BFS and DFS iterative methods and BFS's recursive method
return the goal node without linking the path. DFS's recursive method returns
the linked path's first node. The grid owns these nodes and clears their search
links before another run.

From PowerShell, the equivalent command is `./build-and-run.ps1`.
