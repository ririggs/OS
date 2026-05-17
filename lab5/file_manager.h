#pragma once

#include <string>
#include <vector>
#include "types.h"

class FileManager {
public:
    explicit FileManager(const std::string& filename);

    void createFile(const std::vector<employee>& employees);
    void displayFile() const;
    int findIndexByKey(int key) const;
    void updateRecord(int key, const employee& emp);
    std::vector<employee> loadEmployees() const;

    const std::string& getFilename() const { return filename_; }

private:
    std::string filename_;
};
