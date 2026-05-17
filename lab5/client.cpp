#include <windows.h>
#include <iostream>
#include <string>

#include "common.h"

using namespace std;

HANDLE ConnectToServer() {
    for (;;) {
        HANDLE h = CreateFile(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );
        if (h != INVALID_HANDLE_VALUE)
            return h;

        DWORD err = GetLastError();
        if (err == ERROR_PIPE_BUSY) {
            if (!WaitNamedPipe(PIPE_NAME, 15000)) {
                cerr << "Timeout waiting for pipe." << endl;
                return INVALID_HANDLE_VALUE;
            }
            continue;
        }
        cerr << "Cannot connect to server (error " << err << ")." << endl;
        return INVALID_HANDLE_VALUE;
    }
}

BOOL PipeWrite(HANDLE h, const void* buf, DWORD size) {
    DWORD written;
    return WriteFile(h, buf, size, &written, NULL) && written == size;
}

BOOL PipeRead(HANDLE h, void* buf, DWORD size) {
    DWORD read;
    return ReadFile(h, buf, size, &read, NULL) && read == size;
}

Response SendRequest(HANDLE h, const Request& req, int retryKey) {
    Response resp;
    for (;;) {
        if (!PipeWrite(h, &req, sizeof(req))) {
            resp.status = STATUS_NOT_FOUND;
            return resp;
        }
        if (!PipeRead(h, &resp, sizeof(resp))) {
            resp.status = STATUS_NOT_FOUND;
            return resp;
        }

        if (resp.status == STATUS_LOCKED) {
            cout << "  [LOCKED] Record " << retryKey
                 << " is blocked by another client."
                 << " Retrying in " << RETRY_DELAY_SEC << "s..."
                 << endl;
            Sleep(RETRY_DELAY_MS);
            continue;
        }
        return resp;
    }
}

void DoRead(HANDLE hPipe) {
    int key;
    cout << "Enter employee ID to read: ";
    cin >> key;
    cin.ignore();

    Request req = { REQ_READ, key };
    Response resp = SendRequest(hPipe, req, key);

    if (resp.status == STATUS_NOT_FOUND) {
        cout << "Employee #" << key << " not found." << endl;
        return;
    }

    employee& e = resp.record;
    cout << "  ID: " << e.num
         << "  Name: " << e.name
         << "  Hours: " << e.hours << endl;

    cout << "Press ENTER to release read lock..." << endl;
    cin.get();

    req.type = REQ_RELEASE_READ;
    PipeWrite(hPipe, &req, sizeof(req));
    PipeRead(hPipe, &resp, sizeof(resp));
    cout << "Read lock released." << endl;
}

void DoModify(HANDLE hPipe) {
    int key;
    cout << "Enter employee ID to modify: ";
    cin >> key;
    cin.ignore();

    Request req = { REQ_MODIFY, key };
    Response resp = SendRequest(hPipe, req, key);

    if (resp.status == STATUS_NOT_FOUND) {
        cout << "Employee #" << key << " not found." << endl;
        return;
    }

    employee old = resp.record;
    cout << "Current record:" << endl;
    cout << "  ID: " << old.num
         << "  Name: " << old.name
         << "  Hours: " << old.hours << endl;

    employee upd = old;
    cout << "Enter new name (max 9 chars): ";
    cin.getline(upd.name, sizeof(upd.name));
    cout << "Enter new hours: ";
    cin >> upd.hours;
    cin.ignore();

    cout << "Press ENTER to send modified record to server..." << endl;
    cin.get();

    req.type = REQ_WRITE_DATA;
    PipeWrite(hPipe, &req, sizeof(req));
    PipeWrite(hPipe, &upd, sizeof(upd));
    PipeRead(hPipe, &resp, sizeof(resp));

    if (resp.status == STATUS_OK)
        cout << "Record updated successfully." << endl;
    else
        cout << "Failed to update record." << endl;

    cout << "Press ENTER to release write lock..." << endl;
    cin.get();

    req.type = REQ_RELEASE_WRITE;
    PipeWrite(hPipe, &req, sizeof(req));
    PipeRead(hPipe, &resp, sizeof(resp));
    cout << "Write lock released." << endl;
}

int main() {
    cout << "=== Employee Client ===" << endl;
    cout << "Connecting to server..." << endl;

    HANDLE hPipe = ConnectToServer();
    if (hPipe == INVALID_HANDLE_VALUE) {
        cout << "Press ENTER to exit..." << endl;
        cin.get();
        return 1;
    }
    cout << "Connected." << endl;

    for (;;) {
        cout << "\n--- Menu ---" << endl;
        cout << "1. Read employee record" << endl;
        cout << "2. Modify employee record" << endl;
        cout << "3. Exit" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1: DoRead(hPipe);   break;
        case 2: DoModify(hPipe); break;
        case 3:
            CloseHandle(hPipe);
            return 0;
        default:
            cout << "Invalid choice." << endl;
        }
    }
}
