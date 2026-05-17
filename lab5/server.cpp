#include <windows.h>
#include <iostream>
#include <vector>
#include <set>
#include <mutex>
#include <string>

#include "common.h"
#include "lock_manager.h"
#include "file_manager.h"

using namespace std;

string filename;
vector<employee> employees;
mutex clientCountMtx;
int activeClients = 0;

LockManager* g_lockMgr = nullptr;
FileManager* g_fileMgr = nullptr;

DWORD WINAPI ClientHandler(LPVOID param) {
    HANDLE hPipe = (HANDLE)param;

    BOOL connected = ConnectNamedPipe(hPipe, NULL)
        ? TRUE
        : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!connected) {
        CloseHandle(hPipe);
        {
            lock_guard<mutex> lg(clientCountMtx);
            activeClients--;
        }
        return 1;
    }
    cout << "[Server] Client connected." << endl;

    set<int> heldReads, heldWrites;
    DWORD bytes;

    for (;;) {
        Request req;
        if (!ReadFile(hPipe, &req, sizeof(req), &bytes, NULL) || bytes == 0)
            break;

        Response resp;
        ZeroMemory(&resp, sizeof(resp));
        resp.status = STATUS_OK;

        switch (req.type) {

        case REQ_READ: {
            resp.status = g_lockMgr->tryAcquireRead(req.key);
            if (resp.status == STATUS_OK) {
                int idx = g_fileMgr->findIndexByKey(req.key);
                if (idx >= 0) {
                    resp.record = employees[idx];
                    heldReads.insert(req.key);
                } else {
                    resp.status = STATUS_NOT_FOUND;
                    g_lockMgr->releaseRead(req.key);
                }
            }
            WriteFile(hPipe, &resp, sizeof(resp), &bytes, NULL);
            break;
        }

        case REQ_MODIFY: {
            resp.status = g_lockMgr->tryAcquireWrite(req.key);
            if (resp.status == STATUS_OK) {
                int idx = g_fileMgr->findIndexByKey(req.key);
                if (idx >= 0) {
                    resp.record = employees[idx];
                    heldWrites.insert(req.key);
                } else {
                    resp.status = STATUS_NOT_FOUND;
                    g_lockMgr->releaseWrite(req.key);
                }
            }
            WriteFile(hPipe, &resp, sizeof(resp), &bytes, NULL);
            break;
        }

        case REQ_WRITE_DATA: {
            employee emp;
            if (ReadFile(hPipe, &emp, sizeof(emp), &bytes, NULL) && bytes > 0) {
                g_fileMgr->updateRecord(req.key, emp);
                employees = g_fileMgr->loadEmployees();
                resp.status = STATUS_OK;
            } else {
                resp.status = STATUS_NOT_FOUND;
            }
            WriteFile(hPipe, &resp, sizeof(resp), &bytes, NULL);
            break;
        }

        case REQ_RELEASE_READ:
            g_lockMgr->releaseRead(req.key);
            heldReads.erase(req.key);
            WriteFile(hPipe, &resp, sizeof(resp), &bytes, NULL);
            break;

        case REQ_RELEASE_WRITE:
            g_lockMgr->releaseWrite(req.key);
            heldWrites.erase(req.key);
            WriteFile(hPipe, &resp, sizeof(resp), &bytes, NULL);
            break;

        default:
            resp.status = STATUS_NOT_FOUND;
            WriteFile(hPipe, &resp, sizeof(resp), &bytes, NULL);
            break;
        }
    }

    for (int key : heldReads)  g_lockMgr->releaseRead(key);
    for (int key : heldWrites) g_lockMgr->releaseWrite(key);

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    {
        lock_guard<mutex> lg(clientCountMtx);
        activeClients--;
    }
    cout << "[Server] Client disconnected." << endl;
    return 0;
}

int main() {
    cout << "=== Employee File Server ===" << endl;
    cout << "Enter binary filename: ";
    getline(cin, filename);

    int n;
    cout << "Enter number of employees: ";
    cin >> n;
    cin.ignore();
    for (int i = 0; i < n; i++) {
        employee e;
        cout << "Employee " << (i + 1) << ":" << endl;
        cout << "  ID: ";         cin >> e.num;
        cout << "  Name (<10): "; cin >> e.name;
        cout << "  Hours: ";      cin >> e.hours;
        cin.ignore();
        employees.push_back(e);
    }

    g_fileMgr = new FileManager(filename);
    g_fileMgr->createFile(employees);
    g_fileMgr->displayFile();

    int clientCount;
    cout << "Enter number of clients: ";
    cin >> clientCount;
    cin.ignore();
    activeClients = clientCount;

    g_lockMgr = new LockManager();

    vector<HANDLE> pipeHandles(clientCount);
    vector<HANDLE> threadHandles(clientCount);

    for (int i = 0; i < clientCount; i++) {
        HANDLE h = CreateNamedPipe(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            clientCount,
            BUFFER_SIZE, BUFFER_SIZE,
            0, NULL
        );
        pipeHandles[i] = h;
        if (h == INVALID_HANDLE_VALUE) {
            cerr << "Failed to create named pipe instance " << i << endl;
            return 1;
        }
        threadHandles[i] = CreateThread(NULL, 0, ClientHandler, h, 0, NULL);
        if (!threadHandles[i]) {
            cerr << "Failed to create thread " << i << endl;
            return 1;
        }
    }

    vector<HANDLE> procHandles;
    for (int i = 0; i < clientCount; i++) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (CreateProcess(
            TEXT("client.exe"),
            NULL, NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE,
            NULL, NULL,
            &si, &pi
        )) {
            procHandles.push_back(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            cerr << "Failed to start client " << i
                 << " (error " << GetLastError() << ")" << endl;
            if (pipeHandles[i] != NULL) {
                CloseHandle(pipeHandles[i]);
                pipeHandles[i] = NULL;
            }
            {
                lock_guard<mutex> lg(clientCountMtx);
                activeClients--;
            }
        }
    }
    cout << "[Server] Waiting for all clients to finish..." << endl;

    for (;;) {
        {
            lock_guard<mutex> lg(clientCountMtx);
            if (activeClients <= 0) break;
        }
        Sleep(200);
    }

    if (!threadHandles.empty())
        WaitForMultipleObjects((DWORD)threadHandles.size(),
                               threadHandles.data(), TRUE, INFINITE);

    for (HANDLE h : pipeHandles)   if (h) CloseHandle(h);
    for (HANDLE h : threadHandles) if (h) CloseHandle(h);
    for (HANDLE h : procHandles)   if (h) CloseHandle(h);

    cout << "\n[Server] All clients finished." << endl;
    g_fileMgr->displayFile();

    cin.sync();
    cout << "\nPress ENTER to exit server..." << endl;
    cin.get();

    delete g_lockMgr;
    delete g_fileMgr;
    return 0;
}
