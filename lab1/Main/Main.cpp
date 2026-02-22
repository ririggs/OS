#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

struct employee {
    int num;
    char name[10];
    double hours;
};

bool RunProcess(const std::string& cmdLine) {
    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    std::string cmd = cmdLine;

    if (!CreateProcessA(
        NULL,
        &cmd[0],
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi)) {
        std::cerr << "CreateProcess failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0) {
        std::cerr << "Process exited with code " << exitCode << std::endl;
        return false;
    }

    return true;
}

void PrintBinaryFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open binary file: " << filename << std::endl;
        return;
    }

    std::cout << std::endl;
    std::cout << "Contents of binary file \"" << filename << "\":" << std::endl;
    std::cout << std::left << std::setw(12) << "Employee #"
        << std::setw(12) << "Name"
        << std::setw(10) << "Hours" << std::endl;
    std::cout << std::string(34, '-') << std::endl;

    employee emp;
    while (in.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        std::cout << std::left << std::setw(12) << emp.num
            << std::setw(12) << emp.name
            << std::setw(10) << std::fixed << std::setprecision(1) << emp.hours
            << std::endl;
    }

    in.close();
    std::cout << std::endl;
}

void PrintTextFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Cannot open report file: " << filename << std::endl;
        return;
    }

    std::cout << std::endl;
    std::string line;
    while (std::getline(in, line)) {
        std::cout << line << std::endl;
    }
    in.close();
    std::cout << std::endl;
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    std::string binaryFile;
    int recordCount;

    std::cout << "Enter binary file name: ";
    std::cin >> binaryFile;

    std::cout << "Enter number of records: ";
    std::cin >> recordCount;

    // Step 1: Run Creator
    std::ostringstream creatorCmd;
    creatorCmd << "Creator.exe " << binaryFile << " " << recordCount;

    std::cout << std::endl << "Starting Creator..." << std::endl;
    if (!RunProcess(creatorCmd.str())) {
        std::cerr << "Creator failed." << std::endl;
        return 1;
    }

    // Step 2: Display binary file contents
    PrintBinaryFile(binaryFile);

    // Step 3: Get report parameters
    std::string reportFile;
    double hourlyRate;

    std::cout << "Enter report file name: ";
    std::cin >> reportFile;

    std::cout << "Enter hourly rate: ";
    std::cin >> hourlyRate;

    // Step 4: Run Reporter
    std::ostringstream reporterCmd;
    reporterCmd << "Reporter.exe " << binaryFile << " " << reportFile << " " << hourlyRate;

    std::cout << std::endl << "Starting Reporter..." << std::endl;
    if (!RunProcess(reporterCmd.str())) {
        std::cerr << "Reporter failed." << std::endl;
        return 1;
    }

    // Step 5: Display report
    PrintTextFile(reportFile);

    return 0;
}
