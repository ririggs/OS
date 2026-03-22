#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

#include "marker_utils.h"


struct SharedState {
    std::vector<int> arr;       
    CRITICAL_SECTION arrCs;     
    CRITICAL_SECTION coutCs;    
};


struct MarkerContext {
    SharedState* state;
    int          id;          
    HANDLE       hStart;     
    HANDLE       hStuck;      
    HANDLE       hContinue;   
    HANDLE       hTerminate;  
};


static void markerLog(SharedState* s, const std::string& msg) {
    EnterCriticalSection(&s->coutCs);
    std::cout << msg;
    LeaveCriticalSection(&s->coutCs);
}


DWORD WINAPI MarkerThread(LPVOID param) {
    auto*        ctx = reinterpret_cast<MarkerContext*>(param);
    SharedState* s   = ctx->state;
    const int    id  = ctx->id;
    const int    n   = static_cast<int>(s->arr.size());

    WaitForSingleObject(ctx->hStart, INFINITE);

    srand(static_cast<unsigned>(id));

    while (true) {
        const int r = rand() % n;

        EnterCriticalSection(&s->arrCs);
        const bool isZero = (s->arr[r] == 0);
        LeaveCriticalSection(&s->arrCs);

        if (isZero) {
            Sleep(5);
            EnterCriticalSection(&s->arrCs);
            s->arr[r] = id;  
            LeaveCriticalSection(&s->arrCs);
            Sleep(5);
        } else {
            EnterCriticalSection(&s->arrCs);
            const int marked = countMarkedBy(s->arr, id);
            LeaveCriticalSection(&s->arrCs);

            markerLog(s,
                "[marker " + std::to_string(id) +
                "] stuck: marked=" + std::to_string(marked) +
                ", conflict at index=" + std::to_string(r) + "\n");

            SetEvent(ctx->hStuck);

            HANDLE h[2] = { ctx->hContinue, ctx->hTerminate };
            DWORD  res  = WaitForMultipleObjects(2, h, FALSE, INFINITE);

            if (res == WAIT_OBJECT_0 + 1) {
                EnterCriticalSection(&s->arrCs);
                clearMarkerEntries(s->arr, id);
                LeaveCriticalSection(&s->arrCs);
                return 0;
            }
        }
    }
}


int main() {
    int n = 0;
    std::cout << "Enter array size: ";
    std::cin >> n;
    if (n <= 0) { std::cerr << "Array size must be positive.\n"; return 1; }

    int numMarkers = 0;
    std::cout << "Enter number of marker threads: ";
    std::cin >> numMarkers;
    if (numMarkers <= 0) { std::cerr << "Thread count must be positive.\n"; return 1; }
    if (numMarkers > MAXIMUM_WAIT_OBJECTS) {
        std::cerr << "Too many threads (max " << MAXIMUM_WAIT_OBJECTS << ").\n";
        return 1;
    }

    SharedState state;
    state.arr.assign(n, 0);
    InitializeCriticalSection(&state.arrCs);
    InitializeCriticalSection(&state.coutCs);

    HANDLE hStart = CreateEvent(NULL, /*bManualReset=*/TRUE, /*bInitialState=*/FALSE, NULL);

    std::vector<MarkerContext> ctxs(numMarkers);
    std::vector<HANDLE>        hThreads(numMarkers, NULL);
    std::vector<HANDLE>        hStuck(numMarkers, NULL);
    std::vector<bool>          active(numMarkers, true);

    for (int i = 0; i < numMarkers; ++i) {
        ctxs[i].state      = &state;
        ctxs[i].id         = i + 1;
        ctxs[i].hStart     = hStart;
        ctxs[i].hStuck     = CreateEvent(NULL, FALSE, FALSE, NULL);
        ctxs[i].hContinue  = CreateEvent(NULL, FALSE, FALSE, NULL);
        ctxs[i].hTerminate = CreateEvent(NULL, FALSE, FALSE, NULL);
        hStuck[i]          = ctxs[i].hStuck;
    }

    for (int i = 0; i < numMarkers; ++i) {
        hThreads[i] = CreateThread(NULL, 0, MarkerThread, &ctxs[i], 0, NULL);
        if (!hThreads[i]) {
            std::cerr << "Failed to create thread " << (i + 1)
                      << " (error " << GetLastError() << ").\n";
            return 1;
        }
    }

    SetEvent(hStart);

    int activeCount = numMarkers;

    while (activeCount > 0) {
        std::vector<HANDLE> stuckNow;
        stuckNow.reserve(activeCount);
        for (int i = 0; i < numMarkers; ++i)
            if (active[i]) stuckNow.push_back(hStuck[i]);

        WaitForMultipleObjects(
            static_cast<DWORD>(stuckNow.size()),
            stuckNow.data(),
            /*bWaitAll=*/TRUE,
            INFINITE);

        std::cout << "\n[main] Array: ";
        printArray(state.arr, std::cout);

        int choice = 0;
        while (true) {
            std::cout << "[main] Enter marker number to terminate (1-" << numMarkers << "): ";
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(1024, '\n');
                continue;
            }
            if (choice >= 1 && choice <= numMarkers && active[choice - 1]) break;
            std::cout << "[main] Invalid choice or marker already terminated. Try again.\n";
        }

        const int idx = choice - 1;

        SetEvent(ctxs[idx].hTerminate);

        WaitForSingleObject(hThreads[idx], INFINITE);
        CloseHandle(hThreads[idx]);
        hThreads[idx] = NULL;
        active[idx]   = false;
        --activeCount;

        std::cout << "[main] Array after marker " << choice << " terminated: ";
        printArray(state.arr, std::cout);

        for (int i = 0; i < numMarkers; ++i)
            if (active[i]) SetEvent(ctxs[i].hContinue);
    }

    DeleteCriticalSection(&state.arrCs);
    DeleteCriticalSection(&state.coutCs);
    CloseHandle(hStart);
    for (int i = 0; i < numMarkers; ++i) {
        CloseHandle(hStuck[i]);
        CloseHandle(ctxs[i].hContinue);
        CloseHandle(ctxs[i].hTerminate);
    }

    std::cout << "\n[main] All marker threads terminated. Exiting.\n";
    return 0;
}
