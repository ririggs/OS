#include "lock_manager.h"

int LockManager::tryAcquireRead(int key) {
    std::lock_guard<std::mutex> lg(lockMtx_);
    LockEntry& e = locks_[key];
    if (e.writers > 0) return STATUS_LOCKED;
    e.readers++;
    return STATUS_OK;
}

int LockManager::tryAcquireWrite(int key) {
    std::lock_guard<std::mutex> lg(lockMtx_);
    LockEntry& e = locks_[key];
    if (e.writers > 0 || e.readers > 0) return STATUS_LOCKED;
    e.writers = 1;
    return STATUS_OK;
}

void LockManager::releaseRead(int key) {
    std::lock_guard<std::mutex> lg(lockMtx_);
    auto it = locks_.find(key);
    if (it != locks_.end()) {
        it->second.readers--;
        if (it->second.readers <= 0 && it->second.writers <= 0)
            locks_.erase(it);
    }
}

void LockManager::releaseWrite(int key) {
    std::lock_guard<std::mutex> lg(lockMtx_);
    auto it = locks_.find(key);
    if (it != locks_.end()) {
        it->second.writers = 0;
        if (it->second.readers <= 0)
            locks_.erase(it);
    }
}

bool LockManager::isLocked(int key) const {
    std::lock_guard<std::mutex> lg(lockMtx_);
    auto it = locks_.find(key);
    if (it == locks_.end()) return false;
    return it->second.readers > 0 || it->second.writers > 0;
}

int LockManager::getReaderCount(int key) const {
    std::lock_guard<std::mutex> lg(lockMtx_);
    auto it = locks_.find(key);
    if (it == locks_.end()) return 0;
    return it->second.readers;
}

int LockManager::getWriterCount(int key) const {
    std::lock_guard<std::mutex> lg(lockMtx_);
    auto it = locks_.find(key);
    if (it == locks_.end()) return 0;
    return it->second.writers;
}
