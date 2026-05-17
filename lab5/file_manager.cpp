#include "file_manager.h"
#include <fstream>
#include <iostream>

FileManager::FileManager(const std::string& filename) : filename_(filename) {}

void FileManager::createFile(const std::vector<employee>& employees) {
    std::ofstream f(filename_, std::ios::binary | std::ios::trunc);
    if (!f) {
        std::cerr << "Cannot create file: " << filename_ << std::endl;
        return;
    }
    for (const auto& e : employees)
        f.write(reinterpret_cast<const char*>(&e), sizeof(e));
    f.close();
}

void FileManager::displayFile() const {
    std::ifstream f(filename_, std::ios::binary);
    if (!f) {
        std::cerr << "Cannot open file: " << filename_ << std::endl;
        return;
    }
    std::cout << "--- File contents (" << filename_ << ") ---" << std::endl;
    employee e;
    while (f.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        std::cout << "ID: " << e.num
                  << "  Name: " << e.name
                  << "  Hours: " << e.hours << std::endl;
    }
    f.close();
    std::cout << "--- End of file ---" << std::endl;
}

int FileManager::findIndexByKey(int key) const {
    std::vector<employee> emps = loadEmployees();
    for (size_t i = 0; i < emps.size(); i++)
        if (emps[i].num == key) return (int)i;
    return -1;
}

void FileManager::updateRecord(int key, const employee& emp) {
    int idx = findIndexByKey(key);
    if (idx < 0) return;

    std::fstream f(filename_, std::ios::binary | std::ios::in | std::ios::out);
    if (!f) return;
    f.seekp(idx * sizeof(employee));
    f.write(reinterpret_cast<const char*>(&emp), sizeof(emp));
    f.close();
}

std::vector<employee> FileManager::loadEmployees() const {
    std::vector<employee> result;
    std::ifstream f(filename_, std::ios::binary);
    if (!f) return result;
    employee e;
    while (f.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        result.push_back(e);
    }
    f.close();
    return result;
}
