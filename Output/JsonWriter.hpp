#pragma once

#include "../DataStructures/Grid.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace json_detail {
inline void writeString(std::ostream& out, std::string_view value) {
    constexpr char hex[] = "0123456789abcdef";
    out << '"';
    for (unsigned char ch : value) {
        if (ch == '"' || ch == '\\') {
            out << '\\' << static_cast<char>(ch);
        } else if (ch < 0x20) {
            out << "\\u00" << hex[ch >> 4] << hex[ch & 0x0f];
        } else {
            out << static_cast<char>(ch);
        }
    }
    out << '"';
}
}

class JsonWriter {
public:
    template <typename T>
    static void write(const std::string& filename, const Grid<T>& grid,
                      const std::string& algorithmName) {
        std::ostringstream json;
        json << "{\n  \"algorithm\": ";
        json_detail::writeString(json, algorithmName);
        json << ",\n  \"rows\": [";
        for (std::size_t i = 0; i < grid.getRows().size(); ++i) {
            if (i != 0) json << ", ";
            json_detail::writeString(json, grid.getRows()[i]);
        }
        json << "],\n  \"discoveredTimes\": [";
        for (int i = 0; i < grid.getNodeCount(); ++i) {
            if (i != 0) json << ", ";
            json << grid.getNode(i).discoveredTime;
        }
        json << "],\n  \"path\": [";
        const int length = grid.getPathLength();
        if (length >= 0) {
            const Node<T>* current = grid.getStart();
            for (int step = 0; step <= length; ++step) {
                if (step != 0) json << ", ";
                json << grid.getIndex(current->getX(), current->getY());
                current = current->next;
            }
        }
        json << "]\n}\n";

        std::ofstream file(filename, std::ios::binary);
        if (!file) throw std::runtime_error("Could not write " + filename);
        file << json.str();
        if (!file) throw std::runtime_error("Could not finish writing " + filename);
    }
};
