#include <gtest/gtest.h>
#include "file_manager.h"
#include <cstdio>
#include <fstream>

class FileManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        testFile = "test_employees.bin";
        fm = new FileManager(testFile);
    }
    void TearDown() override {
        delete fm;
        std::remove(testFile.c_str());
    }

    std::vector<employee> makeSampleData() {
        std::vector<employee> emps;
        employee e1{101, "Ivanov", 160.5};
        employee e2{202, "Petrov", 140.0};
        employee e3{303, "Sidorov", 175.0};
        emps.push_back(e1);
        emps.push_back(e2);
        emps.push_back(e3);
        return emps;
    }

    std::string testFile;
    FileManager* fm;
};

TEST_F(FileManagerTest, CreateFileThenLoad) {
    auto emps = makeSampleData();
    fm->createFile(emps);

    auto loaded = fm->loadEmployees();
    ASSERT_EQ(loaded.size(), 3u);
    EXPECT_EQ(loaded[0].num, 101);
    EXPECT_STREQ(loaded[0].name, "Ivanov");
    EXPECT_DOUBLE_EQ(loaded[0].hours, 160.5);

    EXPECT_EQ(loaded[1].num, 202);
    EXPECT_STREQ(loaded[1].name, "Petrov");
    EXPECT_DOUBLE_EQ(loaded[1].hours, 140.0);

    EXPECT_EQ(loaded[2].num, 303);
    EXPECT_STREQ(loaded[2].name, "Sidorov");
    EXPECT_DOUBLE_EQ(loaded[2].hours, 175.0);
}

TEST_F(FileManagerTest, FindExistingKey) {
    auto emps = makeSampleData();
    fm->createFile(emps);

    EXPECT_EQ(fm->findIndexByKey(101), 0);
    EXPECT_EQ(fm->findIndexByKey(202), 1);
    EXPECT_EQ(fm->findIndexByKey(303), 2);
}

TEST_F(FileManagerTest, FindNonExistingKey) {
    auto emps = makeSampleData();
    fm->createFile(emps);

    EXPECT_EQ(fm->findIndexByKey(999), -1);
}

TEST_F(FileManagerTest, UpdateRecord) {
    auto emps = makeSampleData();
    fm->createFile(emps);

    employee updated{202, "Smirnov", 200.0};
    fm->updateRecord(202, updated);

    auto loaded = fm->loadEmployees();
    ASSERT_EQ(loaded.size(), 3u);

    EXPECT_EQ(loaded[1].num, 202);
    EXPECT_STREQ(loaded[1].name, "Smirnov");
    EXPECT_DOUBLE_EQ(loaded[1].hours, 200.0);

    EXPECT_EQ(loaded[0].num, 101);
    EXPECT_STREQ(loaded[0].name, "Ivanov");
    EXPECT_EQ(loaded[2].num, 303);
    EXPECT_STREQ(loaded[2].name, "Sidorov");
}

TEST_F(FileManagerTest, UpdateNonExistingKey_NoEffect) {
    auto emps = makeSampleData();
    fm->createFile(emps);

    employee fake{999, "Nobody", 0.0};
    fm->updateRecord(999, fake);

    auto loaded = fm->loadEmployees();
    ASSERT_EQ(loaded.size(), 3u);
    EXPECT_EQ(loaded[0].num, 101);
    EXPECT_EQ(loaded[1].num, 202);
    EXPECT_EQ(loaded[2].num, 303);
}

TEST_F(FileManagerTest, LoadEmptyFile) {
    std::vector<employee> empty;
    fm->createFile(empty);

    auto loaded = fm->loadEmployees();
    EXPECT_TRUE(loaded.empty());
    EXPECT_EQ(fm->findIndexByKey(101), -1);
}

TEST_F(FileManagerTest, FileSizeCorrect) {
    auto emps = makeSampleData();
    fm->createFile(emps);

    std::ifstream f(testFile, std::ios::binary | std::ios::ate);
    ASSERT_TRUE(f.is_open());
    auto size = f.tellg();
    f.close();

    EXPECT_EQ(size, static_cast<std::streamoff>(emps.size() * sizeof(employee)));
}

TEST_F(FileManagerTest, SingleRecordRoundTrip) {
    std::vector<employee> emps;
    employee e{42, "Test", 99.9};
    emps.push_back(e);
    fm->createFile(emps);

    auto loaded = fm->loadEmployees();
    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].num, 42);
    EXPECT_STREQ(loaded[0].name, "Test");
    EXPECT_DOUBLE_EQ(loaded[0].hours, 99.9);
}
