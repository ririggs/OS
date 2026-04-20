#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "common.h"
#include "ipc_utils.h"

static std::string moduleDir() {
    char buf[MAX_PATH];
    const DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::string path(buf, len);
    const auto pos = path.find_last_of("\\/");
    return (pos == std::string::npos) ? std::string{} : path.substr(0, pos + 1);
}

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

int main() {
    std::cout << "Enter binary file name: ";
    std::string filename;
    if (!(std::cin >> filename)) { std::cerr << "Bad input.\n"; return 1; }

    std::cout << "Enter number of records (ring capacity): ";
    uint32_t capacity = 0;
    if (!(std::cin >> capacity) || capacity == 0) {
        std::cerr << "Capacity must be a positive integer.\n"; return 1;
    }

    std::cout << "Enter number of Sender processes: ";
    int numSenders = 0;
    if (!(std::cin >> numSenders) || numSenders <= 0) {
        std::cerr << "Number of senders must be a positive integer.\n"; return 1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateFile failed (err " << GetLastError() << ").\n";
        return 1;
    }

    LARGE_INTEGER fileSize{};
    fileSize.QuadPart = static_cast<LONGLONG>(computeFileSize(capacity));
    if (!SetFilePointerEx(hFile, fileSize, nullptr, FILE_BEGIN) || !SetEndOfFile(hFile)) {
        std::cerr << "Failed to size the file (err " << GetLastError() << ").\n";
        CloseHandle(hFile); return 1;
    }

    RingHeader hdr{capacity, /*head=*/0, /*tail=*/0, /*count=*/0};
    if (!writeHeader(hFile, hdr)) {
        std::cerr << "Failed to write header (err " << GetLastError() << ").\n";
        CloseHandle(hFile); return 1;
    }
    FlushFileBuffers(hFile);

    const std::string nMutex = mutexName   (filename);
    const std::string nEmpty = semEmptyName(filename);
    const std::string nFull  = semFullName (filename);
    const std::string nReady = semReadyName(filename);

    HANDLE hMutex = CreateMutexA      (nullptr, FALSE, nMutex.c_str());
    HANDLE hEmpty = CreateSemaphoreA  (nullptr, static_cast<LONG>(capacity),
                                                static_cast<LONG>(capacity),
                                                nEmpty.c_str());
    HANDLE hFull  = CreateSemaphoreA  (nullptr, 0, static_cast<LONG>(capacity), nFull.c_str());
    HANDLE hReady = CreateSemaphoreA  (nullptr, 0, static_cast<LONG>(numSenders), nReady.c_str());

    if (!hMutex || !hEmpty || !hFull || !hReady) {
        std::cerr << "Failed to create sync objects (err " << GetLastError() << ").\n";
        return 1;
    }

    const std::string senderExe = moduleDir() + "lab4_sender.exe";
    std::vector<PROCESS_INFORMATION> procs(numSenders);

    for (int i = 0; i < numSenders; ++i) {
        STARTUPINFOA si{};
        si.cb = sizeof(si);

        std::string cmd = "\"" + senderExe + "\" \"" + filename + "\" "
                        + std::to_string(capacity) + " "
                        + std::to_string(i + 1);
        std::vector<char> cmdBuf(cmd.begin(), cmd.end());
        cmdBuf.push_back('\0');

        if (!CreateProcessA(
                /*lpApplicationName=*/ nullptr,
                /*lpCommandLine=*/     cmdBuf.data(),
                /*lpProcessAttrs=*/    nullptr,
                /*lpThreadAttrs=*/     nullptr,
                /*bInheritHandles=*/   FALSE,
                /*dwCreationFlags=*/   CREATE_NEW_CONSOLE,
                /*lpEnvironment=*/     nullptr,
                /*lpCurrentDirectory=*/nullptr,
                &si, &procs[i])) {
            std::cerr << "CreateProcess failed for sender " << (i + 1)
                      << " (err " << GetLastError() << ").\n";
            return 1;
        }
    }

    std::cout << "[Receiver] Waiting for " << numSenders
              << " sender(s) to report ready...\n";
    for (int i = 0; i < numSenders; ++i) {
        WaitForSingleObject(hReady, INFINITE);
    }
    std::cout << "[Receiver] All senders are ready.\n";

    while (true) {
        std::cout << "[Receiver] Command (r = read, q = quit): ";
        std::string cmd;
        if (!readLine(cmd)) break;
        if (cmd.empty()) continue;

        const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(cmd[0])));
        if (c == 'q') break;
        if (c != 'r') { std::cout << "Unknown command.\n"; continue; }

        std::cout << "[Receiver] Waiting for a message...\n";
        WaitForSingleObject(hFull, INFINITE);

        WaitForSingleObject(hMutex, INFINITE);

        RingHeader h{};
        if (!readHeader(hFile, h)) {
            ReleaseMutex(hMutex);
            std::cerr << "[Receiver] Failed to read header.\n";
            continue;
        }

        char slot[MSG_SIZE] = {};
        LARGE_INTEGER off{}; off.QuadPart = static_cast<LONGLONG>(slotOffset(h.head));
        SetFilePointerEx(hFile, off, nullptr, FILE_BEGIN);
        DWORD got = 0;
        ReadFile(hFile, slot, MSG_SIZE, &got, nullptr);

        h.head = advanceIndex(h.head, h.capacity);
        if (h.count > 0) --h.count;
        writeHeader(hFile, h);
        FlushFileBuffers(hFile);

        ReleaseMutex(hMutex);
        ReleaseSemaphore(hEmpty, 1, nullptr);

        const std::string msg = messageFromSlot(slot);
        std::cout << "[Receiver] Got message: \"" << msg << "\"\n";
    }

    std::cout << "[Receiver] Shutting down. Terminating sender processes...\n";
    for (auto& pi : procs) {
        TerminateProcess(pi.hProcess, 0);
        WaitForSingleObject(pi.hProcess, 1000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    CloseHandle(hReady);
    CloseHandle(hFull);
    CloseHandle(hEmpty);
    CloseHandle(hMutex);
    CloseHandle(hFile);

    std::cout << "[Receiver] Exit.\n";
    return 0;
}
