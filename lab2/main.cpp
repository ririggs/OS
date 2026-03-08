#include <windows.h>
#include <iostream>
#include <vector>
#include <iomanip>

#include "array_utils.h"

struct SharedData {
    std::vector<int> arr;
    MinMaxResult     minMax{};
    double           average{};
};

SharedData g_data;

DWORD WINAPI MinMaxThread(LPVOID) {
    const int n = static_cast<int>(g_data.arr.size());

    for (int i = 1; i < n; ++i) {
        Sleep(7); 
        Sleep(7); 
    }

    g_data.minMax = findMinMax(g_data.arr);

    std::cout << "[min_max] Min = " << g_data.minMax.minVal
              << " (index " << g_data.minMax.minIndex << "), "
              << "Max = " << g_data.minMax.maxVal
              << " (index " << g_data.minMax.maxIndex << ")\n";

    return 0;
}

DWORD WINAPI AverageThread(LPVOID) {
    const int n = static_cast<int>(g_data.arr.size());

    for (int i = 0; i < n; ++i) {
        Sleep(12); 
    }

    g_data.average = computeAverage(g_data.arr);

    std::cout << "[average] Average = "
              << std::fixed << std::setprecision(2)
              << g_data.average << "\n";

    return 0;
}

int main() {
    int n;
    std::cout << "Enter array size: ";
    std::cin >> n;

    if (n <= 0) {
        std::cerr << "Array size must be positive.\n";
        return 1;
    }

    g_data.arr.resize(n);
    std::cout << "Enter " << n << " integers:\n";
    for (int i = 0; i < n; ++i) {
        std::cin >> g_data.arr[i];
    }

    std::cout << "\n[main] Array: ";
    for (int i = 0; i < n; ++i) {
        std::cout << g_data.arr[i] << (i < n - 1 ? " " : "\n");
    }

    HANDLE hMinMax  = CreateThread(NULL, 0, MinMaxThread,  NULL, 0, NULL);
    HANDLE hAverage = CreateThread(NULL, 0, AverageThread, NULL, 0, NULL);

    if (!hMinMax || !hAverage) {
        std::cerr << "[main] Failed to create threads. Error: " << GetLastError() << "\n";
        return 1;
    }

    WaitForSingleObject(hMinMax,  INFINITE);
    WaitForSingleObject(hAverage, INFINITE);

    CloseHandle(hMinMax);
    CloseHandle(hAverage);

    const int avgInt = static_cast<int>(g_data.average);
    replaceMinMax(g_data.arr, g_data.minMax.minIndex, g_data.minMax.maxIndex, avgInt);

    std::cout << "\n[main] Result (min and max replaced with average = "
              << std::fixed << std::setprecision(2) << g_data.average << "):\n";
    for (int i = 0; i < n; ++i) {
        std::cout << g_data.arr[i] << (i < n - 1 ? " " : "\n");
    }

    return 0;
}
