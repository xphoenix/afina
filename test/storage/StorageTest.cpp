#include "gtest/gtest.h"
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

#include <afina/execute/Add.h>
#include <afina/execute/Append.h>
#include <afina/execute/Delete.h>
#include <afina/execute/Get.h>
#include <afina/execute/Set.h>

#include "storage/SimpleLRU.h"

using namespace Afina::Backend;
using namespace Afina::Execute;
using namespace std;

TEST(StorageTest, PutGet) {
    SimpleLRU storage;

    storage.Put("KEY1", "val1");
    storage.Put("KEY2", "val2");

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val1");

    EXPECT_TRUE(storage.Get("KEY2", value));
    EXPECT_TRUE(value == "val2");
}

TEST(StorageTest, PutOverwrite) {
    SimpleLRU storage;

    storage.Put("KEY1", "val1");
    storage.Put("KEY1", "val2");

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val2");
}

TEST(StorageTest, PutIfAbsent) {
    SimpleLRU storage;

    storage.Put("KEY1", "val1");
    storage.PutIfAbsent("KEY1", "val2");

    std::string value;
    EXPECT_TRUE(storage.Get("KEY1", value));
    EXPECT_TRUE(value == "val1");
}

std::string pad_space(const std::string &s, size_t length) {
    std::string result = s;
    result.resize(length, ' ');
    return result;
}

TEST(StorageTest, BigTest) {
    const size_t length = 20;
    SimpleLRU storage(2 * 100000 * length);

    for (long i = 0; i < 100000; ++i) {
        auto key = pad_space("Key " + std::to_string(i), length);
        auto val = pad_space("Val " + std::to_string(i), length);
        storage.Put(key, val);
    }

    for (long i = 99999; i >= 0; --i) {
        auto key = pad_space("Key " + std::to_string(i), length);
        auto val = pad_space("Val " + std::to_string(i), length);

        std::string res;
        EXPECT_TRUE(storage.Get(key, res));

        EXPECT_TRUE(val == res);
    }
}

TEST(StorageTest, MaxTest) {
    const size_t length = 20;
    SimpleLRU storage(2 * 1000 * length);

    std::stringstream ss;

    for (long i = 0; i < 1100; ++i) {
        auto key = pad_space("Key " + std::to_string(i), length);
        auto val = pad_space("Val " + std::to_string(i), length);
        storage.Put(key, val);
    }

    for (long i = 100; i < 1100; ++i) {
        auto key = pad_space("Key " + std::to_string(i), length);
        auto val = pad_space("Val " + std::to_string(i), length);

        std::string res;
        EXPECT_TRUE(storage.Get(key, res));

        EXPECT_TRUE(val == res);
    }

    for (long i = 0; i < 100; ++i) {
        auto key = pad_space("Key " + std::to_string(i), length);

        std::string res;
        EXPECT_FALSE(storage.Get(key, res));
    }
}
