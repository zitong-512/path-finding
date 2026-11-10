#include "Algorithms/BreadthFirstSearch.hpp"
#include "Algorithms/DepthFirstSearch.hpp"
#include "Algorithms/Dijkstra.hpp"
#include "Algorithms/Greedy.hpp"
#include "DataStructures/Grid.hpp"
#include "Output/JsonWriter.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <string>

int main() {
    try {
        using CellValue = int; // Change to std::string for text values.
        // Edit these rows, then press Ctrl+Shift+B in VS Code.
        // S = start, G = goal, # = wall, . = open cell.
        Grid<CellValue> grid({
            "S.............",
            ".####.#####...",
            "....#.....#...",
            "###.#.###.#.#.",
            "....#...#...#.",
            ".######.#####.",
            ".............G",
        });

        // Choose BreadthFirstSearch, DepthFirstSearch, Dijkstra, or Greedy.
        std::unique_ptr<Algorithm<CellValue>> algorithm =
            std::make_unique<BreadthFirstSearch<CellValue>>();
        Node<CellValue>* first = algorithm->run(grid);
        JsonWriter::write("web/run.json", grid, algorithm->getName());
        std::cout << algorithm->getName() << " wrote web/run.json: ";
        const int steps = grid.getPathLength();
        if (first == nullptr) std::cout << "no path found\n";
        else std::cout << steps << " steps\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
