#pragma once

#include <map>
#include <mutex>
#include "types.h"

class LockManager {
public:
    int tryAcquireRead(int key);
    int tryAcquireWrite(int key);
    void releaseRead(int key);
    void releaseWrite(int key);

    bool isLocked(int key) const;
    int getReaderCount(int key) const;
    int getWriterCount(int key) const;

private:
    struct LockEntry {
        int readers = 0;
        int writers = 0;
    };
    mutable std::mutex lockMtx_;
    std::map<int, LockEntry> locks_;
};
