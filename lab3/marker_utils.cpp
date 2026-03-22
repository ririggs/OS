#include "marker_utils.h"

#include <iostream>

int countMarkedBy(const std::vector<int>& arr, int markerId) {
    int count = 0;
    for (int v : arr)
        if (v == markerId) ++count;
    return count;
}

void clearMarkerEntries(std::vector<int>& arr, int markerId) {
    for (int& v : arr)
        if (v == markerId) v = 0;
}

void printArray(const std::vector<int>& arr, std::ostream& out) {
    for (std::size_t i = 0; i < arr.size(); ++i)
        out << arr[i] << (i + 1 < arr.size() ? " " : "\n");
    if (arr.empty()) out << "\n";
}
