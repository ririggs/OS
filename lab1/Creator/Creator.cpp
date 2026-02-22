#include <windows.h>
#include <iostream>
#include <fstream>
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

    if (argc != 3) {
        std::cerr << "Usage: Creator <filename> <record_count>" << std::endl;
        return 1;
    }

    const char* filename = argv[1];
    int count = std::atoi(argv[2]);

    if (count <= 0) {
        std::cerr << "Record count must be positive." << std::endl;
        return 1;
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot create file: " << filename << std::endl;
        return 1;
    }

    for (int i = 0; i < count; ++i) {
        employee emp = {};

        std::cout << "Employee " << (i + 1) << ":" << std::endl;

        std::cout << "  ID: ";
        std::cin >> emp.num;

        std::cout << "  Name (max 9 chars): ";
        std::cin >> emp.name;
        emp.name[9] = '\0';

        std::cout << "  Hours worked: ";
        std::cin >> emp.hours;

        out.write(reinterpret_cast<const char*>(&emp), sizeof(employee));
    }

    out.close();
    std::cout << "Binary file \"" << filename << "\" created (" << count << " records)." << std::endl;

    return 0;
}
