#pragma once

#include <ostream>
#include <vector>

int countMarkedBy(const std::vector<int>& arr, int markerId);

void clearMarkerEntries(std::vector<int>& arr, int markerId);

void printArray(const std::vector<int>& arr, std::ostream& out);
