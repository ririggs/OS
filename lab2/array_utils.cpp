#include "array_utils.h"

MinMaxResult findMinMax(const std::vector<int>& arr) {
    if (arr.empty()) {
        throw std::invalid_argument("findMinMax: array must not be empty");
    }

    MinMaxResult r{arr[0], arr[0], 0, 0};

    for (int i = 1; i < static_cast<int>(arr.size()); ++i) {
        if (arr[i] < r.minVal) {
            r.minVal   = arr[i];
            r.minIndex = i;
        }
        if (arr[i] > r.maxVal) {
            r.maxVal   = arr[i];
            r.maxIndex = i;
        }
    }

    return r;
}

double computeAverage(const std::vector<int>& arr) {
    if (arr.empty()) {
        throw std::invalid_argument("computeAverage: array must not be empty");
    }

    long long sum = 0;
    for (int x : arr) {
        sum += x;
    }

    return static_cast<double>(sum) / static_cast<int>(arr.size());
}

void replaceMinMax(std::vector<int>& arr, int minIndex, int maxIndex, int replaceValue) {
    arr[minIndex] = replaceValue;
    if (maxIndex != minIndex) {
        arr[maxIndex] = replaceValue;
    }
}
