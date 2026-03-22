#include <gtest/gtest.h>

#include <sstream>
#include <vector>

#include "marker_utils.h"

// ─────────────────────────────────────────────────────────────────
// countMarkedBy
// ─────────────────────────────────────────────────────────────────

TEST(CountMarkedBy, EmptyArray) {
    EXPECT_EQ(countMarkedBy({}, 1), 0);
}

TEST(CountMarkedBy, AllZerosNotCountedForMarkerId1) {
    std::vector<int> arr(5, 0);
    EXPECT_EQ(countMarkedBy(arr, 1), 0);
}

TEST(CountMarkedBy, AllMarkedBySelf) {
    EXPECT_EQ(countMarkedBy({2, 2, 2, 2}, 2), 4);
}

TEST(CountMarkedBy, SingleMatch) {
    EXPECT_EQ(countMarkedBy({0, 0, 3, 0}, 3), 1);
}

TEST(CountMarkedBy, MultipleMarkersPresent) {
    std::vector<int> arr = {1, 2, 1, 3, 0, 1, 2};
    EXPECT_EQ(countMarkedBy(arr, 1), 3);
    EXPECT_EQ(countMarkedBy(arr, 2), 2);
    EXPECT_EQ(countMarkedBy(arr, 3), 1);
    EXPECT_EQ(countMarkedBy(arr, 4), 0);
}

TEST(CountMarkedBy, NoMatchReturnsZero) {
    EXPECT_EQ(countMarkedBy({1, 2, 3}, 5), 0);
}

TEST(CountMarkedBy, SingleElementMatch) {
    EXPECT_EQ(countMarkedBy({7}, 7), 1);
}

TEST(CountMarkedBy, SingleElementNoMatch) {
    EXPECT_EQ(countMarkedBy({7}, 1), 0);
}

// ─────────────────────────────────────────────────────────────────
// clearMarkerEntries
// ─────────────────────────────────────────────────────────────────

TEST(ClearMarkerEntries, EmptyArrayUnchanged) {
    std::vector<int> arr;
    clearMarkerEntries(arr, 1);
    EXPECT_TRUE(arr.empty());
}

TEST(ClearMarkerEntries, ClearAllMatchingElements) {
    std::vector<int> arr = {1, 1, 1};
    clearMarkerEntries(arr, 1);
    EXPECT_EQ(arr, (std::vector<int>{0, 0, 0}));
}

TEST(ClearMarkerEntries, ClearOnlyMatchingElements) {
    std::vector<int> arr = {1, 2, 1, 3, 2};
    clearMarkerEntries(arr, 1);
    EXPECT_EQ(arr, (std::vector<int>{0, 2, 0, 3, 2}));
}

TEST(ClearMarkerEntries, NothingToClearLeavesArrayIntact) {
    std::vector<int> arr = {2, 3, 4};
    clearMarkerEntries(arr, 1);
    EXPECT_EQ(arr, (std::vector<int>{2, 3, 4}));
}

TEST(ClearMarkerEntries, DoesNotAffectOtherMarkers) {
    std::vector<int> arr = {1, 2, 3, 1, 2};
    clearMarkerEntries(arr, 2);
    EXPECT_EQ(arr, (std::vector<int>{1, 0, 3, 1, 0}));
}

TEST(ClearMarkerEntries, SingleElementCleared) {
    std::vector<int> arr = {5};
    clearMarkerEntries(arr, 5);
    EXPECT_EQ(arr[0], 0);
}

TEST(ClearMarkerEntries, SingleElementNotCleared) {
    std::vector<int> arr = {5};
    clearMarkerEntries(arr, 9);
    EXPECT_EQ(arr[0], 5);
}

TEST(ClearMarkerEntries, IdempotentOnZeroedArray) {
    std::vector<int> arr = {0, 0, 0};
    clearMarkerEntries(arr, 1);
    EXPECT_EQ(arr, (std::vector<int>{0, 0, 0}));
}

// ─────────────────────────────────────────────────────────────────
// printArray
// ─────────────────────────────────────────────────────────────────

TEST(PrintArray, EmptyArrayPrintsNewline) {
    std::vector<int> arr;
    std::ostringstream oss;
    printArray(arr, oss);
    EXPECT_EQ(oss.str(), "\n");
}

TEST(PrintArray, SingleElement) {
    std::ostringstream oss;
    printArray({42}, oss);
    EXPECT_EQ(oss.str(), "42\n");
}

TEST(PrintArray, MultipleElementsSpaceSeparated) {
    std::ostringstream oss;
    printArray({1, 2, 3}, oss);
    EXPECT_EQ(oss.str(), "1 2 3\n");
}

TEST(PrintArray, WithZeros) {
    std::ostringstream oss;
    printArray({0, 1, 0, 2}, oss);
    EXPECT_EQ(oss.str(), "0 1 0 2\n");
}

TEST(PrintArray, AllZeros) {
    std::ostringstream oss;
    printArray({0, 0, 0}, oss);
    EXPECT_EQ(oss.str(), "0 0 0\n");
}

TEST(PrintArray, NegativeValues) {
    std::ostringstream oss;
    printArray({-1, -2, -3}, oss);
    EXPECT_EQ(oss.str(), "-1 -2 -3\n");
}

TEST(PrintArray, NoTrailingSpaceBeforeNewline) {
    std::ostringstream oss;
    printArray({7, 8}, oss);
    const std::string out = oss.str();
    // должно заканчиваться на "8\n", а не на "8 \n"
    ASSERT_GE(out.size(), 2u);
    EXPECT_EQ(out[out.size() - 2], '8');
    EXPECT_EQ(out[out.size() - 1], '\n');
}

// ─────────────────────────────────────────────────────────────────
// Взаимодействие между countMarkedBy и clearMarkerEntries
// ─────────────────────────────────────────────────────────────────

TEST(Interaction, CountAfterClearIsZero) {
    std::vector<int> arr = {1, 2, 1, 3, 1};
    EXPECT_EQ(countMarkedBy(arr, 1), 3);
    clearMarkerEntries(arr, 1);
    EXPECT_EQ(countMarkedBy(arr, 1), 0);
}

TEST(Interaction, OtherMarkersUnaffectedAfterClear) {
    std::vector<int> arr = {1, 2, 1, 2};
    clearMarkerEntries(arr, 1);
    EXPECT_EQ(countMarkedBy(arr, 2), 2);  // marker 2 все еще существует
}
