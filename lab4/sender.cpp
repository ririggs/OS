#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "common.h"
#include "ipc_utils.h"

static bool writeHeader(HANDLE hFile, const RingHeader& h) {
    LARGE_INTEGER zero{};
    if (!SetFilePointerEx(hFile, zero, nullptr, FILE_BEGIN)) return false;
    DWORD written = 0;
    if (!WriteFile(hFile, &h, sizeof(h), &written, nullptr)) return false;
    return written == sizeof(h);
}

static bool readHeader(HANDLE hFile, RingHeader& h) {
    LARGE_INTEGER zero{};
    if (!SetFilePointerEx(hFile, zero, nullptr, FILE_BEGIN)) return false;
    DWORD got = 0;
    if (!ReadFile(hFile, &h, sizeof(h), &got, nullptr)) return false;
    return got == sizeof(h);
}

static void trimCR(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
}

static bool readLine(std::string& out) {
    if (!std::getline(std::cin, out)) return false;
    trimCR(out);
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: lab4_sender.exe <filename> <capacity> [id]\n";
        return 1;
    }
    const std::string filename = argv[1];
    const uint32_t    capacity = static_cast<uint32_t>(std::strtoul(argv[2], nullptr, 10));
    const int         id       = (argc >= 4) ? std::atoi(argv[3]) : 0;
    (void)capacity;

    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "[Sender " << id << "] Failed to open \"" << filename
                  << "\" (err " << GetLastError() << ").\n";
        std::cerr << "Press Enter to exit..."; std::cin.get();
        return 1;
    }

    HANDLE hMutex = OpenMutexA    (SYNCHRONIZE, FALSE, mutexName   (filename).c_str());
    HANDLE hEmpty = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE,
                                   semEmptyName(filename).c_str());
    HANDLE hFull  = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE,
                                   semFullName (filename).c_str());
    HANDLE hReady = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE,            FALSE,
                                   semReadyName(filename).c_str());

    if (!hMutex || !hEmpty || !hFull || !hReady) {
        std::cerr << "[Sender " << id << "] Failed to open sync objects "
                     "(err " << GetLastError() << ").\n";
        std::cerr << "Press Enter to exit..."; std::cin.get();
        return 1;
    }

    ReleaseSemaphore(hReady, 1, nullptr);
    std::cout << "[Sender " << id << "] Ready. Connected to \"" << filename << "\".\n";

    while (true) {
        std::cout << "[Sender " << id << "] Command (s = send, q = quit): ";
        std::string cmd;
        if (!readLine(cmd)) break;
        if (cmd.empty()) continue;

        const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(cmd[0])));
        if (c == 'q') break;
        if (c != 's') { std::cout << "Unknown command.\n"; continue; }

        std::cout << "[Sender " << id << "] Message (< " << MSG_SIZE << " chars): ";
        std::string msg;
        if (!readLine(msg)) break;
        if (!validateMessage(msg)) {
            std::cout << "[Sender " << id << "] Invalid length (1.." << (MSG_SIZE - 1)
                      << " characters required).\n";
            continue;
        }

        std::cout << "[Sender " << id << "] Waiting for a free slot...\n";
        WaitForSingleObject(hEmpty, INFINITE);
        WaitForSingleObject(hMutex, INFINITE);

        RingHeader h{};
        if (!readHeader(hFile, h)) {
            ReleaseMutex(hMutex);
            ReleaseSemaphore(hEmpty, 1, nullptr);
            std::cerr << "[Sender " << id << "] Failed to read header.\n";
            continue;
        }

        char slot[MSG_SIZE] = {};
        std::memcpy(slot, msg.data(), msg.size());

        LARGE_INTEGER off{}; off.QuadPart = static_cast<LONGLONG>(slotOffset(h.tail));
        SetFilePointerEx(hFile, off, nullptr, FILE_BEGIN);
        DWORD written = 0;
        WriteFile(hFile, slot, MSG_SIZE, &written, nullptr);

        h.tail = advanceIndex(h.tail, h.capacity);
        ++h.count;
        writeHeader(hFile, h);
        FlushFileBuffers(hFile);

        ReleaseMutex(hMutex);
        ReleaseSemaphore(hFull, 1, nullptr);

        std::cout << "[Sender " << id << "] Sent: \"" << msg << "\"\n";
    }

    CloseHandle(hReady);
    CloseHandle(hFull);
    CloseHandle(hEmpty);
    CloseHandle(hMutex);
    CloseHandle(hFile);

    std::cout << "[Sender " << id << "] Exit.\n";
    return 0;
}
