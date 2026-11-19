# Pathfinding exercises

Create maps in `main.cpp` that make the existing search algorithms behave as
described below. For exercises 1 through 4, change only the map; keep the
algorithm implementations and neighbor order unchanged. Exercises 5 through
10 allow changes to the heuristic and the numeric types needed to represent
it. Exercises 11 through 17 introduce new algorithms. Exercises 18 through 25
extend the search problem and may require changes to the grid, search state,
JSON output, and browser. Keep movement costs at
one and neighbor order unchanged throughout, except for ordering candidates
by the priorities explicitly requested in an exercise.
Use the algorithms shown in the browser, including the iterative versions
of BFS and DFS.

## Rules and measurements

- Use exactly one `S` and one `G`, `#` for walls, and `.` for open cells,
  except where an exercise explicitly introduces additional symbols or goals.
- All rows must have the same length. Movement is horizontal or vertical.
- For exercises 1 through 10, every non-wall cell must be reachable from `S`,
  and `G` must be reachable. The new algorithm exercises also require tests
  where no route exists.
- Count **discovered cells**: cells whose `discoveredTime` is greater than zero
  when the algorithm stops. Count `S` and `G` too. This is the discovery count
  shown by the animation, not the number of nodes removed from the frontier.
- Measure path length in moves, not cells.

The original forward search algorithms discover cells when first adding them to the frontier and stop when
removing `G`. A discovered cell may still be waiting in the queue or stack.
Neighbors are ordered right, down, left, up; iterative DFS pushes them in reverse
order so the right neighbor is removed first.

A successful BFS cannot discover only one cell: `S` and `G` are distinct cells.
The first challenge uses a small discovery count instead.

## Exercise 1 A nearby goal and a long distraction

Create a map with at least **20 non-wall cells** where:

- BFS discovers **at most 5 cells** before it stops.
- DFS discovers **every non-wall cell** before it stops.

Both algorithms must find the goal. Explain how the frontier order makes DFS
explore the distraction before removing the goal from its stack, even if the
goal has already been discovered.

## Exercise 2 Make Greedy search the whole map

Create a map with at least **20 non-wall cells** where Greedy reaches the goal
but discovers **every non-wall cell** before it stops.

Include at least one **dead-end branch outside the returned path**, so a single
corridor from `S` to `G` does not satisfy the challenge. Greedy must discover
every cell in that branch too.

Record Greedy's discovery count and path length. Explain how the map makes
Greedy investigate the dead-end branch despite prioritizing cells with a
smaller Manhattan distance to the goal. Distinguish cells that are discovered
from cells that are removed from the frontier.

## Exercise 3 Guide the search with a heuristic

Create a map with at least **30 non-wall cells** where:

- Dijkstra, Greedy, and A* all reach the goal and return paths of **the same length**.
- A* discovers **at most half as many cells as Dijkstra** before stopping.
- Greedy also discovers **at most half as many cells as Dijkstra** before stopping.

Explain which parts of the map Dijkstra discovers that the other two leave
undiscovered. Compare their priorities: distance from the start for Dijkstra,
Manhattan distance to the goal for Greedy, and their sum for A*. Does Greedy's
success on this map guarantee that it always finds a shortest path?

All moves in this project cost one. Keep those costs and the Manhattan
heuristic unchanged for exercises 2 and 3.

## Exercise 4 Turn a weighted graph into a map

Create a map equivalent to this **undirected weighted graph**, with `S` as the
start and `G` as the goal. The numbers are edge costs, not vertex values.

```text
         4
    S -------- A
    |          |
  3 |          | 5
    |          |
    B -------- G
         3
```

Represent each named vertex by a distinct map cell, and each edge of weight
`w` by a corridor of **exactly `w` unit moves**. For example, an edge of weight
4 becomes:

```text
U -- 1 -- o -- 1 -- o -- 1 -- o -- 1 -- V
```

That is **three intermediate cells and four moves**, not four intermediate
cells. The program assigns cost one to each move; do not assign weights to
`Node::value` or change the algorithms.

Find the path from `S` to `G` on the graph by hand using **BFS, DFS, Dijkstra,
Greedy, and A***, then run each corresponding algorithm on your map.

Explain why replacing weighted edges with unit moves preserves route costs.
Does this alone guarantee identical BFS, DFS, and Greedy choices for every
weighted graph and every map layout? Consider the effect of subdivision,
neighbor order, and the heuristic.

## Preparing for the heuristic exercises

Keep the original Manhattan versions available for comparison. Use Dijkstra's
path length as the shortest-path reference, and run each comparison on exactly
the same map with the same start and goal.

The current heuristic and priority fields use integers. For Euclidean distance
and fractional scaling, use floating-point types consistently throughout the
heuristic calculation, stored heuristic, queue priority, and queue entry type.
Avoid truncating values back to integers. Path costs can remain integers because
every move still costs one. Document these supporting changes.

## Exercise 5 Give A* no estimate

Set A*'s heuristic to **zero for every cell**. Before running it, predict which
existing algorithm it will behave like.

Test on three maps: an open area, a branching maze, and a map with a long detour.
Compare A* with Dijkstra using path length, discovery count, and discovery order.
Explain what remains of A*'s priority formula `f = g + h` when `h = 0`.

## Exercise 6 Manhattan versus Euclidean distance

Implement **Euclidean distance** as an alternative heuristic for A*:

```text
Manhattan: h = abs(x - goalX) + abs(y - goalY)
Euclidean: h = sqrt((x - goalX)^2 + (y - goalY)^2)
```

Create a map where the two versions discover **different numbers of cells**.
Both must return a shortest path, checked against Dijkstra.

Record the results and explain which heuristic gives a stronger estimate for
four-direction movement. Show why neither estimate exceeds the actual remaining
path cost. Does a stronger estimate guarantee strictly fewer discoveries on
every map, including maps with equal-priority choices?

## Exercise 7 Change the influence of the heuristic

Use A* with `h = k * Manhattan` for **k = 0, 0.5, 1, 2, and 5**.
Create a map where at least one version with `k > 1` returns a **longer path**
than Dijkstra and the version with `k = 1`.

For the same map, record each version's discovery count and path length.
Explain how scaling changes the balance between the route cost so far and the
estimate to the goal. Which values can overestimate the true remaining cost?
Do your results show that increasing `k` always reduces discoveries, or only
that it can do so?

## Exercise 8 Make Greedy ignore a coordinate

Replace Greedy's heuristic with **horizontal distance only**:

```text
h = abs(x - goalX)
```

Create a map where this version discovers **more cells** than the original
Manhattan version. Then repeat the challenge using **vertical distance only**:

```text
h = abs(y - goalY)
```

You may use a different map for the second comparison. Record discovery counts
and path lengths for each pair. Explain how ignoring a coordinate changes
priorities and creates ties, and how walls exploit that missing information.

## Exercise 9 Give the heuristic knowledge of a wall

Create a map divided into two regions by a wall with **exactly one opening**.
Place `S` and `G` in opposite regions so every valid route must use that opening.

Design an A* heuristic that accounts for the opening. Keep movement costs at
one; do not precompute exact shortest paths with another search algorithm.
Create a map where your version discovers **fewer cells** than Manhattan A*
while returning the same shortest path length as Dijkstra.

Define your heuristic for cells in both regions. Explain why it is zero at
the goal and never exceeds the true remaining cost. Also check **consistency**:
for every adjacent pair of open cells `u` and `v`, it must satisfy
`h(u) <= 1 + h(v)`. This matters because the current A* implementation does
not reopen settled cells. Show how the estimate accounts for a detour that
Manhattan distance ignores.

## Exercise 10 Scale Greedy without changing its choices

Run Greedy with `h = k * Manhattan` for **k = 0.5, 1, 2, and 5** on three
different maps. Try to make positive scaling change its returned path.

Compare path lengths, discovery counts, and discovery order. Explain why
positive scaling preserves both priority order and ties when values are
represented without rounding or overflow. Why does the same argument not
apply to A*'s formula `g + k * h`?

Finally, try **k = 0**. Explain why this case differs from positive scaling
and predict which search behavior the queue's tie handling produces.

## Preparing for the new algorithm exercises

Implement each new algorithm as an `Algorithm<T>` subclass in `Algorithms/`,
with a matching class and header name and a default constructor. Rebuild and
check that it appears in the browser selector. Keep the existing algorithms
available as references.

Reset search state before each run. Return the start node on success and
`nullptr` on failure, and construct a forward path from `S` to `G` using
`prev` and `next`. Prevent cycles in both the search and the returned path.
Test a corridor, a branching maze, and a map with no route to the goal. Test
both `int` and `std::string` node values.

Distinguish **unique discovered cells**, **total expansions**, and **maximum
frontier size**. Count an expansion whenever a cell is selected for processing,
including the terminal cell. Repeated expansions count again; unique discoveries
do not. Record these measurements separately when the browser's discovery
animation cannot show them. For repeated searches, retain a separate set of
cells encountered across all iterations rather than treating one iteration's
`discoveredTime` values as the total.

A node has only one intrusive frontier pointer, `link`, so it cannot belong
to two such frontiers at once. Use separate storage when needed. Additional
arrays or standard containers are allowed for independent visited states,
distances, and predecessor information.

## Exercise 11 Run BFS from the goal
HOMEWORK

Implement BFS starting at **`G`**, stopping when it removes **`S`** from its
queue. The returned path must still run forward from `S` to `G`.

On three different maps, compare your algorithm with ordinary BFS. Both must
return a shortest path of the same length, though the actual route may differ
when several shortest paths exist.

Create a map where the two directions discover different numbers of cells.
Explain why searching backward is valid on this undirected grid, and draw how
you convert the reverse search's predecessor information into the required
forward `prev` and `next` links.

## Exercise 12 Build a distance map from the goal

Extend the previous exercise: run BFS from `G` until the queue is empty, assigning
each reachable cell its shortest distance to `G`. Give unreachable cells a
distinct value that cannot be confused with a valid distance.

Starting at `S`, construct a path by repeatedly choosing a neighbor whose
distance is **exactly one smaller**, until reaching distance zero at `G`.
Verify the returned path length against ordinary BFS.

Choose three different starting cells on the same map and extract a path from
each using the **same precomputed distance map**. You may expose a helper that
accepts a starting cell; keep the browser's `run(grid)` result rooted at `S`.
Include an unreachable starting cell in a separate test.

Explain why the decreasing distances prevent cycles, how ties between equally
good neighbors are resolved, and why moving the goal or changing walls requires
recomputing the distance map. Report preprocessing work separately from path
extraction work.

## Exercise 13 Combine two searches with bidirectional BFS

Implement BFS from `S` and `G` simultaneously, keeping independent distances,
visited states, and predecessor information for the two directions.

Process **complete BFS layers**, alternating directions or choosing the smaller
frontier at layer boundaries. Track candidate connections between the two
searches and justify a stopping condition that guarantees a shortest path;
do not rely on an arbitrary first encounter during uneven partial layers.
Join the two partial paths without duplicating the meeting cell or creating
a cycle.

Verify your path length against ordinary BFS on multiple maps, including a
case with several shortest routes and a case with no route.

Create one map where bidirectional BFS performs **fewer total expansions**
than ordinary BFS and another where the improvement is small or absent.
Include work from both directions: a cell expanded by both searches counts
twice toward total expansions but once toward unique discoveries. For maximum
frontier size, measure the combined number of waiting entries in both queues.
Explain why searching from two ends is not equally beneficial on every map.

## Exercise 14 Get stuck with hill climbing

Implement hill climbing using Manhattan distance. At each step, choose the
neighbor with the smallest heuristic, breaking ties by the existing neighbor
order. Move only if the heuristic is **strictly smaller** than the current
cell's value. Stop with failure if no improving move exists.

Create one map where the algorithm reaches `G` and another where it stops
despite a valid route existing. Use BFS to demonstrate that the second map
is solvable. Record the cell where hill climbing stops and the heuristic
values of its neighbors.

Explain how this differs from Greedy best-first search, which retains alternative
frontier cells. Does hill climbing guarantee a shortest path or even finding
an existing path? Discuss specifically what strict improvement means with
Manhattan distance and unit four-direction movement.

## Exercise 15 Repeat depth-limited DFS

Implement **iterative deepening DFS**: run depth-limited DFS with limits
`0, 1, 2, ...` until reaching the goal. Reset iteration-specific state between
attempts and prevent cycles along the current path.

Distinguish finding the goal, cutting off a search at its depth limit, and
exhausting all possible routes. Ensure the algorithm terminates on an unreachable
goal. Do not let a global visited flag incorrectly suppress a later route
that reaches the same cell with more depth remaining.

Match ordinary BFS's shortest path length on three solvable maps. Compare
total expansions across **all iterations**, unique discoveries, and maximum
active search depth with ordinary DFS and BFS. Explain the tradeoff between
repeating work and storing a wide frontier.

## Exercise 16 Discard candidates with beam search

Implement **level-by-level beam search** with beam width `k`. Expand the current
level, collect and deduplicate its candidate successors, and retain only the
best `k` according to Manhattan distance for the next level. Resolve equal
heuristics by candidate generation order. Define how you prevent cycles and
how visited state interacts with discarded candidates.

Create a map where width **1 fails despite a valid route**, but a larger width
succeeds. Compare widths `1`, `2`, and `4` with ordinary BFS, recording success
or failure, path length, total expansions, and maximum frontier size. Count
temporary candidate storage separately from the retained beam.

Explain why removing candidates can discard the only valid route. Does a
larger width guarantee success, or a shortest path, on every map? Base your
answer on the specific visited-state policy you implemented.

## Exercise 17 Search with an A* cost threshold

Implement **IDA*** using depth-first search bounded by `f = g + h`, with
Manhattan distance as `h`. Start with the threshold `h(S)`. After an unsuccessful
iteration, use the **smallest exceeded f value** as the next threshold.

Prevent cycles on the current path. Distinguish a threshold cutoff from
exhausting all possible routes so an unreachable goal terminates with failure.
Retain separate measurements across iterations.

Match ordinary A*'s shortest path length on three solvable maps. Record the
sequence of thresholds, total expansions including repeated work, unique
discoveries, and maximum active path length. Compare these with ordinary A*'s
expansions and maximum frontier size, noting any additional stored state.

Explain how IDA* combines A*'s cost estimate with depth-first exploration and
why reducing frontier storage can require revisiting cells.

## Preparing for the extended search exercises

For exercises 18 through 25, document any new map symbols, validation rules,
and changes to the goal representation. Keep the original algorithms usable
on ordinary maps. Define the search state and optimization objective before
implementing each extension, and test both solvable and unsolvable cases.

Some exercises require distinct states at the same cell. Store visited flags,
costs, and predecessors by **complete search state**, not just cell position.
Report unique states separately from unique cells.

A single `prev` and `next` pointer per cell cannot represent a route that
revisits that cell with different state. For those exercises, reconstruct an
ordered sequence of states and extend the output and playback to show repeats,
waiting, or inventory changes. Do not force such routes into cyclic cell links.

## Exercise 18 Choose between multiple goals

Allow several `G` cells and implement BFS that finds the **nearest reachable
goal by number of moves**. Then adapt A* to use the minimum Manhattan distance
to any goal as its heuristic. Stop when a goal is selected for processing,
not merely when it is first discovered.

Create a map where the nearest goal by Manhattan distance is **not** the
nearest by actual route length. Include another test with an unreachable goal
and a reachable alternative, and one where no goal is reachable.

Compare BFS and A* using the selected goal, path length, and discovery count.
For equally short routes to different goals, allow either result but document
tie handling. Explain why taking the minimum heuristic still gives a lower
bound on the cost to a nearest goal.

## Exercise 19 Collect a key before opening a door

Add a key cell `K` and a locked door `D`. Entering the key cell permanently
collects the key; entering the door is legal only after collecting it. All
legal moves cost one, and the key is not consumed by opening the door.

Implement a shortest-path search whose state includes **position and whether
the key has been collected**. Create a map where reaching `G` requires revisiting
a cell after collecting the key. Also test an inaccessible key and a map
where the goal can be reached without using the door.

Show the returned state sequence, including where inventory changes. Explain
why a single `discovered` flag per cell can incorrectly reject the required
return visit, and verify that the door is never crossed without the key.

## Exercise 20 Prefer the safest route

Mark dangerous cells with a new symbol. Find a route that **first minimizes
the number of dangerous-cell entries and then minimizes moves** among equally
safe routes. Count each entry into a dangerous cell, including repeat entries;
do not charge an entry for the initial placement at `S`.

Implement a search using the lexicographic cost pair `(danger entries, moves)`.
Create a map where the safest route is longer than the ordinary shortest
route. Add a second map with equally safe routes of different lengths to
demonstrate the secondary objective.

Report both costs and compare with ordinary BFS. Explain why a simple fixed
penalty for danger does not express the same priority on arbitrarily large
maps unless a sufficient bound on route lengths is known.

## Exercise 21 Count every shortest path

Extend BFS to compute the **number of distinct shortest paths** from `S` to
`G`, as well as their shared length. Paths are distinct if their ordered cell
sequences differ. Count only shortest paths, not arbitrary walks.

Design three maps with exactly **1, 2, and 6 shortest paths**, respectively.
Verify these small examples by listing the paths manually. Test an unreachable
goal, which must have count zero.

Explain how counts accumulate when different predecessors reach a cell at the
same shortest distance, and when it is safe to stop processing. Avoid enumerating
all paths in the algorithm. Document how you detect or avoid integer overflow
on maps with many shortest paths.

## Exercise 22 Minimize turns among shortest paths

Find a route that **first minimizes moves and then minimizes turns**. Include
arrival direction in the search state. A turn occurs when a move's direction
differs from the preceding move; the first move does not count as a turn.

Create a map with equally short routes having different turn counts. Show
that your algorithm selects the route with fewer turns. Also create a map
where a longer route has fewer turns, and verify that the shorter route still
wins because moves are the primary objective.

Report the cost pair `(moves, turns)` and the full direction sequence. Explain
why reaching the same cell from different directions can lead to different
future costs, making a single best record per cell insufficient.

## Exercise 23 Find cells that every route needs

Identify every **non-endpoint cell whose removal disconnects `S` from `G`**.
Use a map where the goal is initially reachable. Start with a reference
implementation that temporarily blocks each candidate cell and reruns BFS.

Create maps with no mandatory cells, exactly one mandatory cell, and several
mandatory cells. Restore each cell before testing the next candidate and never
classify `S`, `G`, or walls as removable candidates.

Then investigate a more efficient approach using DFS articulation-point
analysis. An articulation point can separate an irrelevant side branch;
determine which ones specifically separate `S` from `G`. Match the reference
implementation's results and compare the amount of work as maps grow.

## Exercise 24 Wait for a moving obstacle

Add one obstacle that follows a predefined legal route with period `T`. At
each time step the agent may move to a neighbor or **wait in place**, each
costing one time step. Define the obstacle's position at every phase of its
cycle, and ensure it does not initially occupy `S`.

A transition from time `t` to `t + 1` is legal only if the agent's destination
is unoccupied at `t + 1`. Also forbid the agent and obstacle from swapping
positions during the same step. Apply these rules to waiting too.

Search states must contain **position and time modulo T**. Create a map where
waiting is necessary for success, and another where no collision-free route
exists. Find a route with minimum arrival time and animate both participants
step by step, visibly including waits.

Explain why revisiting a cell at another phase may be necessary and why keeping
time modulo `T` makes the state space finite in this periodic environment.

## Exercise 25 Build a landmark heuristic

Choose fixed landmark cells and precompute BFS distances from each landmark
to every reachable cell. On this undirected unit-cost grid, implement A* with:

```text
h(cell) = max over landmarks L of abs(d(L, cell) - d(L, G))
```

Use only terms where both distances are finite; if no term is available, use
zero. Never subtract unreachable-distance sentinel values. Explain the lower
bound using the triangle inequality and check consistency for adjacent cells.

Create a map where landmark A* discovers **fewer cells than Manhattan A***
while returning the same shortest path length as Dijkstra. Compare at least
two landmark placements and explain why placement affects the estimate.

Finally, try `max(Manhattan, landmark heuristic)`. Report discovery counts,
path lengths, preprocessing work, and stored distance-table size separately.
Explain when precomputation could pay off across repeated searches and when
wall changes require rebuilding the tables.

## What to submit for each exercise

1. The map as C++ row strings, ready to paste into `main.cpp`.
2. Your predicted discovery counts and path lengths before running it.
3. The observed discovery counts and path lengths for each algorithm in the exercise.
4. A short explanation connecting the results to frontier order and priorities.
5. For exercises 5 through 10, the heuristic code and any supporting numeric
   type changes, plus the comparison results requested by the exercise.
6. For exercises 11 through 17, the new algorithm code, test cases and results,
   requested performance measurements, and an explanation of termination,
   path reconstruction, and whether a shortest path is guaranteed.
7. For exercises 18 through 25, the implementation, any new map and output
   formats, the complete search-state definition, and evidence that the
   requested objective and edge cases are handled correctly.

## Additional exercises

Add future map challenges here.
