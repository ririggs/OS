#pragma once

#include <vector>
#include <stdexcept>

/// Result of a min/max scan over an integer array.
struct MinMaxResult {
    int minVal;
    int maxVal;
    int minIndex;
    int maxIndex;
};

/// Find the minimum and maximum values in @p arr and their first occurrences.
/// Throws std::invalid_argument if @p arr is empty.
MinMaxResult findMinMax(const std::vector<int>& arr);

/// Compute the arithmetic mean of @p arr elements.
/// Uses a 64-bit accumulator to avoid overflow.
/// Throws std::invalid_argument if @p arr is empty.
double computeAverage(const std::vector<int>& arr);

/// Replace arr[minIndex] and arr[maxIndex] with @p replaceValue.
/// Skips the second write when both indices are equal (single-element or all-equal array).
void replaceMinMax(std::vector<int>& arr, int minIndex, int maxIndex, int replaceValue);
