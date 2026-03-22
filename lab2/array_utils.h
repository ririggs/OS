#pragma once

#include <vector>
#include <stdexcept>

struct MinMaxResult {
    int minVal;
    int maxVal;
    int minIndex;
    int maxIndex;
};

MinMaxResult findMinMax(const std::vector<int>& arr);

double computeAverage(const std::vector<int>& arr);

void replaceMinMax(std::vector<int>& arr, int minIndex, int maxIndex, int replaceValue);
