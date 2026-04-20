#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include "common.h"
#include "ipc_utils.h"

TEST(ComputeFileSize, ZeroCapacityIsJustHeader) {
    EXPECT_EQ(computeFileSize(0), static_cast<uint64_t>(HEADER_SIZE));
}

TEST(ComputeFileSize, OneSlot) {
    EXPECT_EQ(computeFileSize(1), static_cast<uint64_t>(HEADER_SIZE) + MSG_SIZE);
}

TEST(ComputeFileSize, LinearInCapacity) {
    EXPECT_EQ(computeFileSize(10),
              static_cast<uint64_t>(HEADER_SIZE) + 10ULL * MSG_SIZE);
    EXPECT_EQ(computeFileSize(100),
              static_cast<uint64_t>(HEADER_SIZE) + 100ULL * MSG_SIZE);
}

TEST(ComputeFileSize, DoesNotOverflowOnLargeCapacity) {
    const uint32_t cap = 1'000'000;
    const uint64_t expected =
        static_cast<uint64_t>(HEADER_SIZE) + static_cast<uint64_t>(cap) * MSG_SIZE;
    EXPECT_EQ(computeFileSize(cap), expected);
}

TEST(SlotOffset, SlotZeroStartsAtHeaderEnd) {
    EXPECT_EQ(slotOffset(0), static_cast<uint64_t>(HEADER_SIZE));
}

TEST(SlotOffset, SubsequentSlotsAreContiguous) {
    EXPECT_EQ(slotOffset(1), slotOffset(0) + MSG_SIZE);
    EXPECT_EQ(slotOffset(2), slotOffset(0) + 2 * MSG_SIZE);
    EXPECT_EQ(slotOffset(42),
              static_cast<uint64_t>(HEADER_SIZE) + 42ULL * MSG_SIZE);
}

TEST(AdvanceIndex, WrapsAroundCapacity) {
    EXPECT_EQ(advanceIndex(0, 3), 1u);
    EXPECT_EQ(advanceIndex(1, 3), 2u);
    EXPECT_EQ(advanceIndex(2, 3), 0u);
}

TEST(AdvanceIndex, CapacityOneAlwaysZero) {
    EXPECT_EQ(advanceIndex(0, 1), 0u);
}

TEST(AdvanceIndex, ZeroCapacityIsSafe) {
    EXPECT_EQ(advanceIndex(0, 0), 0u);
    EXPECT_EQ(advanceIndex(5, 0), 0u);
}

TEST(AdvanceIndex, LoopProducesFullCycle) {
    const uint32_t cap = 5;
    uint32_t idx = 0;
    for (uint32_t i = 0; i < cap; ++i) {
        EXPECT_EQ(idx, i);
        idx = advanceIndex(idx, cap);
    }
    EXPECT_EQ(idx, 0u);
}

TEST(ValidateMessage, EmptyIsInvalid) {
    EXPECT_FALSE(validateMessage(""));
}

TEST(ValidateMessage, OneCharIsValid) {
    EXPECT_TRUE(validateMessage("x"));
}

TEST(ValidateMessage, AtLimitMinusOneIsValid) {
    std::string s(MSG_SIZE - 1, 'a');  
    EXPECT_TRUE(validateMessage(s));
}

TEST(ValidateMessage, AtLimitIsInvalid) {
    std::string s(MSG_SIZE, 'a');      
    EXPECT_FALSE(validateMessage(s));
}

TEST(ValidateMessage, OverLimitIsInvalid) {
    std::string s(MSG_SIZE + 5, 'a');
    EXPECT_FALSE(validateMessage(s));
}

TEST(SanitizeNameComponent, AlnumPassesThrough) {
    EXPECT_EQ(sanitizeNameComponent("abc123"), "abc123");
}

TEST(SanitizeNameComponent, SlashAndBackslashReplaced) {
    EXPECT_EQ(sanitizeNameComponent("a/b\\c"), "a_b_c");
}

TEST(SanitizeNameComponent, DotsAndSpacesReplaced) {
    EXPECT_EQ(sanitizeNameComponent("hello world.bin"), "hello_world_bin");
}

TEST(SanitizeNameComponent, EmptyInputStaysEmpty) {
    EXPECT_EQ(sanitizeNameComponent(""), "");
}

TEST(SanitizeNameComponent, AllSpecialsReplaced) {
    EXPECT_EQ(sanitizeNameComponent("!@#$%^&*()"), "__________");
}

TEST(MessageFromSlot, ReadsUpToFirstNull) {
    char slot[MSG_SIZE] = {};
    std::memcpy(slot, "hello", 5);  
    EXPECT_EQ(messageFromSlot(slot), "hello");
}

TEST(MessageFromSlot, EmptySlotProducesEmptyString) {
    char slot[MSG_SIZE] = {};
    EXPECT_EQ(messageFromSlot(slot), "");
}

TEST(MessageFromSlot, FullSlotIsTruncatedAtMsgSize) {
    char slot[MSG_SIZE];
    std::memset(slot, 'A', MSG_SIZE);  
    EXPECT_EQ(messageFromSlot(slot), std::string(MSG_SIZE, 'A'));
}

TEST(MessageFromSlot, StopsAtNullEvenIfMoreBytesExist) {
    char slot[MSG_SIZE] = {};
    std::memcpy(slot, "ab", 2);
    slot[5] = 'Z';  
    EXPECT_EQ(messageFromSlot(slot), "ab");
}

TEST(ObjectNames, AllStartWithLocalNamespace) {
    const std::string f = "queue.bin";
    EXPECT_EQ(mutexName   (f).rfind("Local\\lab4_mutex_", 0), 0u);
    EXPECT_EQ(semEmptyName(f).rfind("Local\\lab4_empty_", 0), 0u);
    EXPECT_EQ(semFullName (f).rfind("Local\\lab4_full_",  0), 0u);
    EXPECT_EQ(semReadyName(f).rfind("Local\\lab4_ready_", 0), 0u);
}

TEST(ObjectNames, AreDistinctForSameFile) {
    const std::string f = "queue.bin";
    const std::string a = mutexName(f);
    const std::string b = semEmptyName(f);
    const std::string c = semFullName(f);
    const std::string d = semReadyName(f);
    EXPECT_NE(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(b, c);
    EXPECT_NE(b, d);
    EXPECT_NE(c, d);
}

TEST(ObjectNames, SanitizesPathSeparators) {
    const std::string f = "C:\\tmp\\queue.bin";
    const std::string n = mutexName(f);
    ASSERT_EQ(n.rfind("Local\\", 0), 0u);
    const std::string suffix = n.substr(std::strlen("Local\\"));
    EXPECT_EQ(suffix.find('\\'), std::string::npos);
    EXPECT_EQ(suffix.find(':'),  std::string::npos);
    EXPECT_EQ(suffix.find('.'),  std::string::npos);
}

TEST(ObjectNames, DifferentFilesProduceDifferentNames) {
    EXPECT_NE(mutexName("q1.bin"), mutexName("q2.bin"));
    EXPECT_NE(semEmptyName("q1.bin"), semEmptyName("q2.bin"));
}

TEST(RingLogic, WriteThenReadProducesSameMessageBytes) {
    char slot[MSG_SIZE] = {};
    const std::string msg = "hi-there";
    std::memcpy(slot, msg.data(), msg.size());
    EXPECT_EQ(messageFromSlot(slot), msg);
}

TEST(RingLogic, IndexCycleMatchesSlotOffsets) {
    const uint32_t cap = 4;
    uint32_t idx = 0;
    uint64_t prevOff = slotOffset(idx);
    for (uint32_t step = 0; step < cap * 3; ++step) {
        idx = advanceIndex(idx, cap);
        const uint64_t off = slotOffset(idx);
        EXPECT_LT(off, computeFileSize(cap));
        const bool contiguous = (off == prevOff + MSG_SIZE);
        const bool wrapped    = (off == slotOffset(0) && idx == 0);
        EXPECT_TRUE(contiguous || wrapped);
        prevOff = off;
    }
}
