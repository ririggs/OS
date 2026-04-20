#include "ipc_utils.h"

#include <cctype>
#include <cstring>

#include "common.h"

uint64_t computeFileSize(uint32_t capacity) {
    return static_cast<uint64_t>(HEADER_SIZE) +
           static_cast<uint64_t>(capacity) * MSG_SIZE;
}

uint64_t slotOffset(uint32_t slot) {
    return static_cast<uint64_t>(HEADER_SIZE) +
           static_cast<uint64_t>(slot) * MSG_SIZE;
}

uint32_t advanceIndex(uint32_t idx, uint32_t capacity) {
    if (capacity == 0) return 0;
    return (idx + 1) % capacity;
}

bool validateMessage(const std::string& msg) {
    return !msg.empty() && msg.size() < MSG_SIZE;
}

std::string sanitizeNameComponent(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        const auto uc = static_cast<unsigned char>(c);
        out.push_back(std::isalnum(uc) ? c : '_');
    }
    return out;
}

std::string messageFromSlot(const char* slotBuf) {
    std::size_t len = 0;
    while (len < MSG_SIZE && slotBuf[len] != '\0') ++len;
    return std::string(slotBuf, len);
}

std::string mutexName(const std::string& file) {
    return "Local\\lab4_mutex_" + sanitizeNameComponent(file);
}

std::string semEmptyName(const std::string& file) {
    return "Local\\lab4_empty_" + sanitizeNameComponent(file);
}

std::string semFullName(const std::string& file) {
    return "Local\\lab4_full_" + sanitizeNameComponent(file);
}

std::string semReadyName(const std::string& file) {
    return "Local\\lab4_ready_" + sanitizeNameComponent(file);
}
