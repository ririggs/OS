#pragma once

#include <cstdint>
#include <string>

uint64_t computeFileSize(uint32_t capacity);
uint64_t slotOffset(uint32_t slot);

uint32_t advanceIndex(uint32_t idx, uint32_t capacity);
bool validateMessage(const std::string& msg);

std::string sanitizeNameComponent(const std::string& s);
std::string messageFromSlot(const char* slotBuf);

std::string mutexName(const std::string& file);
std::string semEmptyName(const std::string& file);
std::string semFullName(const std::string& file);
std::string semReadyName(const std::string& file);
