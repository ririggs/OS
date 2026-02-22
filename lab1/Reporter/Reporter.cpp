#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <cstring>

struct employee {
    int num;
    char name[10];
    double hours;
};

int main(int argc, char* argv[]) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (argc != 4) {
        std::cerr << "Usage: Reporter <binary_file> <report_file> <hourly_rate>" << std::endl;
        return 1;
    }

    const char* binaryFile = argv[1];
    const char* reportFile = argv[2];
    double hourlyRate = std::atof(argv[3]);

    if (hourlyRate <= 0) {
        std::cerr << "Hourly rate must be positive." << std::endl;
        return 1;
    }

    std::ifstream in(binaryFile, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open binary file: " << binaryFile << std::endl;
        return 1;
    }

    std::vector<employee> employees;
    employee emp;
    while (in.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        employees.push_back(emp);
    }
    in.close();

    if (employees.empty()) {
        std::cerr << "No records found in " << binaryFile << std::endl;
        return 1;
    }

    std::sort(employees.begin(), employees.end(), [](const employee& a, const employee& b) {
        return std::strcmp(a.name, b.name) < 0;
        });

    std::ofstream out(reportFile);
    if (!out) {
        std::cerr << "Cannot create report file: " << reportFile << std::endl;
        return 1;
    }

    out << "Report for file \"" << binaryFile << "\"" << std::endl;
    out << std::left << std::setw(12) << "Employee #"
        << std::setw(12) << "Name"
        << std::setw(10) << "Hours"
        << std::setw(12) << "Salary" << std::endl;
    out << std::string(46, '-') << std::endl;

    for (const auto& e : employees) {
        double salary = e.hours * hourlyRate;
        out << std::left << std::setw(12) << e.num
            << std::setw(12) << e.name
            << std::setw(10) << std::fixed << std::setprecision(1) << e.hours
            << std::setw(12) << std::fixed << std::setprecision(2) << salary << std::endl;
    }

    out.close();
    std::cout << "Report file \"" << reportFile << "\" created." << std::endl;

    return 0;
}
