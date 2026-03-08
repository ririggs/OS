#include <gtest/gtest.h>
#include "array_utils.h"

// ─────────────────────────────────────────────
// findMinMax
// ─────────────────────────────────────────────

TEST(FindMinMax, ThrowsOnEmptyArray) {
    EXPECT_THROW(findMinMax({}), std::invalid_argument);
}

TEST(FindMinMax, SingleElement) {
    auto r = findMinMax({7});
    EXPECT_EQ(r.minVal,   7);
    EXPECT_EQ(r.maxVal,   7);
    EXPECT_EQ(r.minIndex, 0);
    EXPECT_EQ(r.maxIndex, 0);
}

TEST(FindMinMax, NormalArray) {
    // {3, 1, 9, 4, 2} → min=1 at idx 1, max=9 at idx 2
    auto r = findMinMax({3, 1, 9, 4, 2});
    EXPECT_EQ(r.minVal,   1);
    EXPECT_EQ(r.minIndex, 1);
    EXPECT_EQ(r.maxVal,   9);
    EXPECT_EQ(r.maxIndex, 2);
}

TEST(FindMinMax, AllElementsEqual) {
    auto r = findMinMax({5, 5, 5});
    EXPECT_EQ(r.minVal, 5);
    EXPECT_EQ(r.maxVal, 5);
}

TEST(FindMinMax, AllNegative) {
    // {-3, -1, -9, -4} → min=-9 at idx 2, max=-1 at idx 1
    auto r = findMinMax({-3, -1, -9, -4});
    EXPECT_EQ(r.minVal,   -9);
    EXPECT_EQ(r.minIndex,  2);
    EXPECT_EQ(r.maxVal,   -1);
    EXPECT_EQ(r.maxIndex,  1);
}

TEST(FindMinMax, MinAtBeginning) {
    auto r = findMinMax({1, 5, 3});
    EXPECT_EQ(r.minVal,   1);
    EXPECT_EQ(r.minIndex, 0);
}

TEST(FindMinMax, MaxAtEnd) {
    auto r = findMinMax({2, 4, 9});
    EXPECT_EQ(r.maxVal,   9);
    EXPECT_EQ(r.maxIndex, 2);
}

TEST(FindMinMax, TwoElements) {
    auto r = findMinMax({10, 1});
    EXPECT_EQ(r.minVal,   1);
    EXPECT_EQ(r.minIndex, 1);
    EXPECT_EQ(r.maxVal,   10);
    EXPECT_EQ(r.maxIndex, 0);
}

TEST(FindMinMax, FirstOccurrenceOfMin) {
    // Min appears twice; first index must be returned
    auto r = findMinMax({1, 5, 1, 3});
    EXPECT_EQ(r.minIndex, 0);
}

TEST(FindMinMax, FirstOccurrenceOfMax) {
    // Max appears twice; first index must be returned
    auto r = findMinMax({9, 3, 9, 2});
    EXPECT_EQ(r.maxIndex, 0);
}

// ─────────────────────────────────────────────
// computeAverage
// ─────────────────────────────────────────────

TEST(ComputeAverage, ThrowsOnEmptyArray) {
    EXPECT_THROW(computeAverage({}), std::invalid_argument);
}

TEST(ComputeAverage, SingleElement) {
    EXPECT_DOUBLE_EQ(computeAverage({7}), 7.0);
}

TEST(ComputeAverage, NormalArray) {
    // (3+1+9+4+2) / 5 = 19/5 = 3.8
    EXPECT_DOUBLE_EQ(computeAverage({3, 1, 9, 4, 2}), 3.8);
}

TEST(ComputeAverage, AllEqual) {
    EXPECT_DOUBLE_EQ(computeAverage({5, 5, 5}), 5.0);
}

TEST(ComputeAverage, AllNegative) {
    // (-2 + -4) / 2 = -3.0
    EXPECT_DOUBLE_EQ(computeAverage({-2, -4}), -3.0);
}

TEST(ComputeAverage, MixedSignValues) {
    // (-5 + 5) / 2 = 0.0
    EXPECT_DOUBLE_EQ(computeAverage({-5, 5}), 0.0);
}

TEST(ComputeAverage, TwoElements) {
    // (1 + 10) / 2 = 5.5
    EXPECT_DOUBLE_EQ(computeAverage({1, 10}), 5.5);
}

TEST(ComputeAverage, LargeValues) {
    // Tests that long long accumulator prevents overflow
    // INT_MAX + INT_MAX = 4294967294 fits in long long
    const int big = 2'000'000'000;
    EXPECT_DOUBLE_EQ(computeAverage({big, big}), static_cast<double>(big));
}

// ─────────────────────────────────────────────
// replaceMinMax
// ─────────────────────────────────────────────

TEST(ReplaceMinMax, DifferentIndices) {
    std::vector<int> arr = {3, 1, 9, 4, 2};
    replaceMinMax(arr, 1, 2, 3);
    EXPECT_EQ(arr[1], 3); // was min=1
    EXPECT_EQ(arr[2], 3); // was max=9
    EXPECT_EQ(arr[0], 3); // unchanged
    EXPECT_EQ(arr[3], 4); // unchanged
    EXPECT_EQ(arr[4], 2); // unchanged
}

TEST(ReplaceMinMax, SameIndex) {
    // minIndex == maxIndex must not crash and write the value once
    std::vector<int> arr = {7};
    replaceMinMax(arr, 0, 0, 42);
    EXPECT_EQ(arr[0], 42);
}

TEST(ReplaceMinMax, TwoElementArray) {
    std::vector<int> arr = {1, 10};
    replaceMinMax(arr, 0, 1, 5);
    EXPECT_EQ(arr[0], 5);
    EXPECT_EQ(arr[1], 5);
}

TEST(ReplaceMinMax, OnlyMinReplaced_WhenMaxIsSameAsMin) {
    std::vector<int> arr = {5, 5, 5};
    // All equal: minIndex=0, maxIndex=0 — only one write should happen
    replaceMinMax(arr, 0, 0, 99);
    EXPECT_EQ(arr[0], 99);
    EXPECT_EQ(arr[1],  5); // unchanged
    EXPECT_EQ(arr[2],  5); // unchanged
}

TEST(ReplaceMinMax, NegativeReplaceValue) {
    std::vector<int> arr = {1, 5, 9};
    replaceMinMax(arr, 0, 2, -3);
    EXPECT_EQ(arr[0], -3);
    EXPECT_EQ(arr[2], -3);
    EXPECT_EQ(arr[1],  5); // unchanged
}
