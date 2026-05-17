#include <gtest/gtest.h>
#include "lock_manager.h"
#include <thread>
#include <vector>
#include <atomic>

class LockManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        lm = new LockManager();
    }
    void TearDown() override {
        delete lm;
    }
    LockManager* lm;
};

TEST_F(LockManagerTest, TryAcquireRead_NoLocks_ReturnsOk) {
    EXPECT_EQ(lm->tryAcquireRead(101), STATUS_OK);
    EXPECT_EQ(lm->getReaderCount(101), 1);
    EXPECT_EQ(lm->getWriterCount(101), 0);
    EXPECT_TRUE(lm->isLocked(101));
}

TEST_F(LockManagerTest, TryAcquireWrite_NoLocks_ReturnsOk) {
    EXPECT_EQ(lm->tryAcquireWrite(101), STATUS_OK);
    EXPECT_EQ(lm->getReaderCount(101), 0);
    EXPECT_EQ(lm->getWriterCount(101), 1);
    EXPECT_TRUE(lm->isLocked(101));
}

TEST_F(LockManagerTest, ReleaseRead_ClearsLock) {
    lm->tryAcquireRead(101);
    lm->releaseRead(101);
    EXPECT_FALSE(lm->isLocked(101));
    EXPECT_EQ(lm->getReaderCount(101), 0);
}

TEST_F(LockManagerTest, ReleaseWrite_ClearsLock) {
    lm->tryAcquireWrite(101);
    lm->releaseWrite(101);
    EXPECT_FALSE(lm->isLocked(101));
    EXPECT_EQ(lm->getWriterCount(101), 0);
}

TEST_F(LockManagerTest, ReleaseNonExistent_NoCrash) {
    lm->releaseRead(999);
    lm->releaseWrite(999);
    EXPECT_FALSE(lm->isLocked(999));
}

TEST_F(LockManagerTest, ReadBlockedByWrite) {
    lm->tryAcquireWrite(101);
    EXPECT_EQ(lm->tryAcquireRead(101), STATUS_LOCKED);
}

TEST_F(LockManagerTest, WriteBlockedByRead) {
    lm->tryAcquireRead(101);
    EXPECT_EQ(lm->tryAcquireWrite(101), STATUS_LOCKED);
}

TEST_F(LockManagerTest, WriteBlockedByWrite) {
    lm->tryAcquireWrite(101);
    EXPECT_EQ(lm->tryAcquireWrite(101), STATUS_LOCKED);
}

TEST_F(LockManagerTest, MultipleReadersAllowed) {
    EXPECT_EQ(lm->tryAcquireRead(101), STATUS_OK);
    EXPECT_EQ(lm->tryAcquireRead(101), STATUS_OK);
    EXPECT_EQ(lm->tryAcquireRead(101), STATUS_OK);
    EXPECT_EQ(lm->getReaderCount(101), 3);
}

TEST_F(LockManagerTest, ReadThenWrite_SuccessAfterRelease) {
    lm->tryAcquireRead(101);
    lm->releaseRead(101);
    EXPECT_EQ(lm->tryAcquireWrite(101), STATUS_OK);
}

TEST_F(LockManagerTest, WriteThenRead_SuccessAfterRelease) {
    lm->tryAcquireWrite(101);
    lm->releaseWrite(101);
    EXPECT_EQ(lm->tryAcquireRead(101), STATUS_OK);
}

TEST_F(LockManagerTest, DifferentKeysIndependent) {
    lm->tryAcquireWrite(101);
    EXPECT_EQ(lm->tryAcquireRead(202), STATUS_OK);
    EXPECT_EQ(lm->tryAcquireWrite(303), STATUS_OK);
}

TEST_F(LockManagerTest, ConcurrentReadersAndWriters) {
    const int key = 42;
    std::atomic<int> okReads{0};
    std::atomic<int> okWrites{0};
    std::atomic<int> lockedWrites{0};

    auto reader = [&]() {
        if (lm->tryAcquireRead(key) == STATUS_OK) {
            okReads++;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            lm->releaseRead(key);
        }
    };

    auto writer = [&]() {
        int res = lm->tryAcquireWrite(key);
        if (res == STATUS_OK) {
            okWrites++;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            lm->releaseWrite(key);
        } else {
            lockedWrites++;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 20; i++) threads.emplace_back(reader);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    for (int i = 0; i < 5; i++) threads.emplace_back(writer);

    for (auto& t : threads) t.join();

    EXPECT_EQ(okReads.load(), 20);
    EXPECT_LE(okWrites.load(), 1);
    EXPECT_EQ(okWrites.load() + lockedWrites.load(), 5);
    EXPECT_FALSE(lm->isLocked(key));
}
